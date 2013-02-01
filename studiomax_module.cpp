/*
	\file		py3dsmax_module.cpp

	\remarks	This file defines the Py3dsMax module for python, and the mxs class
				that can be imported from it

	\author		Blur Studio (c) 2010
	\email		beta@blur.com

	\license	This software is released under the GNU General Public License.  For more info, visit: http://www.gnu.org/
*/

#include "imports.h"
#include "version.h"

// blurPython imports
#include "macros.h"
#include "wrapper.h"
#include "protector.h"

// define the mxs class
typedef struct {
	PyObject_HEAD
} mxs;

// ctor
static PyObject*
mxs_new( PyTypeObject* type, PyObject* args, PyObject* kwds ) {
	return (PyObject*)type->tp_alloc(type, 0);
}

// dtor
static void
mxs_dealloc( PyObject* self ) {
	self->ob_type->tp_free(self);
}

// __getattr__: get an item from the maxscript globals hash table
static PyObject*
mxs_getattro( PyObject* self, PyObject* key ) {
	// Step 1: convert the key to a name
	//mprintf( _T("mxs_getattro entered\n") );
	MXS_PROTECT(one_value_local(name));
	PyStringToMCHAR mkey(key);
	vl.name = Name::intern( mkey.mchar() );
	// Step 2: collect the PyObject* instance
	PyObject * output = ObjectWrapper::py_intern( globals->get( vl.name ) );
	MXS_CLEANUP();
	//mprintf( _T("mxs_getattro leaving\n") );

	return output;
}

// __setattr__: set an item in the maxscript globals hash table
static int
mxs_setattro( PyObject* self, PyObject* key, PyObject* value ) {
	
	//mprintf( _T("mxs_setattro entered\n") );
	MXS_PROTECT( two_value_locals( name, result ) );
	PyStringToMCHAR mkey(key);
	vl.name		= Name::intern( mkey.mchar() );

	// Step 3: get a preexisting global value
	vl.result	= globals->get( vl.name );
	if ( vl.result ) {
		// Step 4: try to set the global variable
		try {
			if ( is_thunk( vl.result ) )
				((Thunk*) vl.result)->assign( ObjectWrapper::intern(value) );
			else
				globals->set( vl.name, ObjectWrapper::intern(value) );
		}
		MXS_CATCHERRORS();
	}

	// Step 5: cleanup the maxscript memory
	MXS_CLEANUP();
	//mprintf( _T("mxs_setattro leaving\n") );
	
	return 0;
}

// define the mxs class
static PyTypeObject MxsType = {
    PyObject_HEAD_INIT(NULL)
    0,											// ob_size
	"Py3dsMax.mxs",								// tp_name
    sizeof(mxs),								// tp_basicsize
    0,											// tp_itemsize
    0,											// tp_dealloc
    0,											// tp_print
    0,											// tp_getattr
	0,											// tp_setattr
    0,											// tp_compare
    0,											// tp_repr
    0,											// tp_as_number
    0,											// tp_as_sequence
    0,											// tp_as_mapping
    0,											// tp_hash 
    0,											// tp_call
    0,											// tp_str
    (getattrofunc)mxs_getattro,					// tp_getattro
    (setattrofunc)mxs_setattro,					// tp_setattro
    0,											// tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	// tp_flags
    "MAXScript Globals Accessor",				// tp_doc 
    0,											// tp_traverse 
    0,											// tp_clear 
    0,											// tp_richcompare 
    0,											// tp_weaklistoffset 
    0,											// tp_iter 
    0,											// tp_iternext 
    0,											// tp_methods 
    0,											// tp_members 
    0,											// tp_getset 
    0,											// tp_base 
    0,											// tp_dict 
    0,											// tp_descr_get 
    0,											// tp_descr_set 
    0,											// tp_dictoffset 
    0,											// tp_init 
    0,											// tp_alloc 
    mxs_new,									// tp_new 
};


typedef struct {
    PyObject_HEAD;
} AtTime;

// ctor
static PyObject*
AtTime_new( PyTypeObject* type, PyObject* args, PyObject* kwds ) {
	return (PyObject*)type->tp_alloc(type, 0);
}

static PyObject * AtTime_call( AtTime* self, PyObject* args, PyObject* kwds );

static int
AtTime_init(AtTime *self, PyObject *args, PyObject *kwds)
{
	Py_ssize_t argCount = PyTuple_Check(args) ? PyTuple_Size(args) : 0;
	if( argCount > 1 )
		return -1;
	if( argCount == 1 ) {
		PyObject * result = AtTime_call( self, args, kwds );
		Py_XDECREF(result);
		return result ? 0 : -1;
	}
    return 0;
}

// dtor
static void
AtTime_dealloc( PyObject* self ) {
	AtTime * at = (AtTime*)self;
	
	PyObject * tsd = PyThreadState_GetDict();
	PyObject * atd = PyDict_GetItemString(tsd, "_AtTime" );
	if( atd ) {
		PyObject * cur_stack = PyDict_GetItemString( atd, "current_stack" );
		PyObject * time_stack = PyDict_GetItemString( atd, "time_stack" );
		
		Py_ssize_t time_top = PyList_Size(time_stack) - 1;
		PyObject * hashKey = PyLong_FromLong( PyObject_Hash((PyObject*)self) );
		Py_ssize_t pos = PySequence_Index(cur_stack, hashKey);
		Py_DECREF(hashKey);
		
		// Restore the correct time value if we are current
		if( PyList_Size(cur_stack) == pos + 1 ) {
			thread_local(current_time) = PyLong_AsLong( PyList_GetItem( time_stack, time_top ) );
			//PySys_WriteStdout( "Current AtTime object destroyed, restoring thread_local(current_time) to %i\n", thread_local(current_time) );
			PySequence_DelItem(time_stack,time_top);
		} else
			// If we aren't current, then we delete the time at the position one above where we are in the current_stack
			PySequence_DelItem(time_stack,pos+1);
		
		PySequence_DelItem(cur_stack,pos);
		
		// If we are the last AtTime object local to this thread, then remove the _AtTime thread-local dict
		if( PyList_Size(cur_stack) == 0 ) {
			thread_local(use_time_context) = (PyDict_GetItemString( atd, "restore_use_time_context" ) == Py_True) ? TRUE : FALSE;
			//PySys_WriteStdout( "Last AtTime object destroyed, setting thread_local(use_time_context) to FALSE\n" );
			PyDict_DelItemString(tsd, "_AtTime");
		}
	}
	self->ob_type->tp_free(self);
}

static PyObject * AtTime_call( AtTime* self, PyObject* args, PyObject* kwds )
{
	if( !PyTuple_Check(args) || (PyTuple_Size(args) != 1) || (!PyNumber_Check(PyTuple_GetItem(args,0))) ) {
		PyErr_SetString( PyExc_AttributeError, "Calling AtTime instances require a single time(int) argument" );
		return 0;
	}
	
	int timeValue = PyInt_AsLong(PyTuple_GetItem(args,0));
	if( PyErr_Occurred() )
		return 0;
	
	PyObject * tsd = PyThreadState_GetDict();
	PyObject * atd = PyDict_GetItemString(tsd, "_AtTime" );
	
	// New reference is either transfered to a list, or decremented below
	PyObject * hashKey = PyLong_FromLong( PyObject_Hash((PyObject*)self) );
	
	if( !atd ) {
		
		// New ref
		atd = PyDict_New();
		
		PyDict_SetItemString( tsd, "_AtTime", atd );
		// tsd now has a ref to atd
		Py_DECREF(atd);
		
		// New refs
		PyObject * time_stack = PyList_New(1), * cur_stack = PyList_New(1);
		
		// PyList_SetItem steals a reference
		PyList_SetItem( time_stack, 0, PyLong_FromLong(thread_local(current_time)) );
		// Give our hashKey reference to cur_stack
		PyList_SetItem( cur_stack, 0, hashKey );
		
		// Dicts take their own reference, so we release ours after this
		PyDict_SetItemString( atd, "time_stack", time_stack );
		PyDict_SetItemString( atd, "current_stack", cur_stack );
		Py_DECREF(time_stack);
		Py_DECREF(cur_stack);
		
		PyObject * utl = PyBool_FromLong( thread_local(use_time_context) );
		// takes it's own ref, so we release ours
		PyDict_SetItemString( atd, "restore_use_time_context", utl );
		Py_DECREF(utl);
		
		thread_local(use_time_context) = TRUE;
		//PySys_WriteStdout( "First AtTime struct activated, setting thread_local(use_time_context) to TRUE\n" );
	} else {
		// Borrowed refs
		PyObject * time_stack = PyDict_GetItemString( atd, "time_stack" );
		PyObject * cur_stack = PyDict_GetItemString( atd, "current_stack" );
		
		Py_ssize_t pos = PySequence_Index(cur_stack, hashKey);
		
		// If we are already current then there is nothing to do
		if( pos != PyList_Size(cur_stack) - 1 ) {
			// If we have restore entries, but aren't current, then delete our existing restore entries
			if( pos >= 0 ) {
				//PySys_WriteStdout( "Deleting current entries in time and cur stack\n" );
				PySequence_DelItem( time_stack, pos + 1 );
				PySequence_DelItem( cur_stack, pos );
			}
			// And add the new restore entries
			//PySys_WriteStdout( "Appending new entries in time and cur stack\n" );
			PyList_Append(time_stack, PyLong_FromLong(thread_local(current_time)) );
			PyList_Append(cur_stack, hashKey );
		} else {
			//PySys_WriteStdout( "Already current, doing nothing but setting current_time\n" );
			Py_DECREF(hashKey);
		}
	}

	//PySys_WriteStdout( "thread_local(current_time) set to %i\n", timeValue );
	thread_local(current_time) = timeValue * GetTicksPerFrame();
	
	Py_INCREF(Py_None);
	return Py_None;
}

static PyTypeObject AtTimeType = {
    PyObject_HEAD_INIT(NULL)
    0,											// ob_size
	"Py3dsMax.AtTime",							// tp_name
    sizeof(AtTime),								// tp_basicsize
    0,											// tp_itemsize
    (destructor)AtTime_dealloc,					// tp_dealloc
    0,											// tp_print
    0,											// tp_getattr
	0,											// tp_setattr
    0,											// tp_compare
    0,											// tp_repr
    0,											// tp_as_number
    0,											// tp_as_sequence
    0,											// tp_as_mapping
    0,											// tp_hash
    (ternaryfunc)AtTime_call,					// tp_call
    0,											// tp_str
    0,											// tp_getattro
    0,											// tp_setattro
    0,											// tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_CLASS,	// tp_flags
    "MAXScript at time emulator",				// tp_doc 
    0,											// tp_traverse 
    0,											// tp_clear 
    0,											// tp_richcompare 
    0,											// tp_weaklistoffset 
    0,											// tp_iter 
    0,											// tp_iternext 
    0,											// tp_methods 
    0,											// tp_members 
    0,											// tp_getset 
    0,											// tp_base 
    0,											// tp_dict 
    0,											// tp_descr_get 
    0,											// tp_descr_set 
    0,											// tp_dictoffset 
    (initproc)AtTime_init,						// tp_init 
    0,											// tp_alloc 
    AtTime_new,									// tp_new 
};

//-----------------------------------------------------------
// define the studio max module (Py3dsMax)

// Py3dsMax.GetWindowHandle() - required for Qt/Ui parenting
static PyObject*
studiomax_getwindowhandle( PyObject* self ) {
	return PyInt_FromLong( (long)GetCOREInterface()->GetMAXHWnd() );
}

// Py3dsMax.GetPluginInstance() - required for Qt/plugin parenting
static PyObject*
studiomax_getplugininstance( PyObject* self ) {
	return PyInt_FromLong( (long)ObjectWrapper::hInstance );
}

// Py3dsMax.DispatchMessage() - passes along a message to the window
static PyObject*
studiomax_dispatchmessage( PyObject* self, PyObject* args ) {
	if ( args && PyTuple_Size(args) == 3 ) {
		MSG msg;
		msg.message = PyInt_AsLong( PyTuple_GetItem( args, 0 ) );
		msg.wParam	= PyInt_AsLong( PyTuple_GetItem( args, 1 ) );
		msg.lParam	= PyInt_AsLong( PyTuple_GetItem( args, 2 ) );

		GetCOREInterface()->TranslateAndDispatchMAXMessage( msg );

		Py_INCREF( Py_None );
		return Py_None;
	}
	else {
		PyErr_SetString( PyExc_AttributeError, "Py3dsMax.DispatchMessage expects exactly 3 arguemnts" );
		return NULL;
	}
}

// Py3dsMax.redo() - redoes the last action
static PyObject*
studiomax_redo( PyObject* self ) {
	ExecuteMAXScriptScript( L"max redo" );

	Py_INCREF( Py_True );
	return Py_True;
}

// Py3dsMax.runScript() - runs a script file
static PyObject*
studiomax_runScript( PyObject* self, PyObject* args ) {
	char* filename = NULL;
	
	mprintf( _T("mxs_runScript entered") );

	// Step 1: convert the input parameters
	if ( !PyArg_ParseTuple( args, "s", &filename ) ) { 
		PyErr_SetString( PyExc_AttributeError, "Py3dsMax.runScript() is expecting a string input" );
		return NULL; 
	}

	// Step 2: load the file
	PyObject* py_file = PyFile_FromString( filename, "r" );
	if ( !py_file ) {
		PyErr_SetString( PyExc_AttributeError, "Py3dsMax.runScript() got an invalid file input" );
		return NULL;
	}

	// Step 3: run the file
	PyRun_SimpleFile( PyFile_AsFile(py_file), filename );

	// Step 4: clear the memory
	Py_XDECREF( py_file );

	if( PyErr_Occurred() )
		return 0;

	// return true
	Py_INCREF( Py_True );
	return Py_True;
}

// Py3dsMax.undo() - undoes the last action
static PyObject*
studiomax_undo( PyObject* self ) {
	ExecuteMAXScriptScript( L"max undo" );

	Py_INCREF( Py_True );
	return Py_True;
}

// Py3dsMax.undoOn() - set the undo system to be on
static PyObject*
studiomax_undoOn( PyObject* self ) {
	theHold.Begin();

	Py_INCREF( Py_True );
	return Py_True;
}

// Py3dsMax.undoOff() - set the undo system to be off
static PyObject*
studiomax_undoOff( PyObject* self, PyObject* args ) {
	MCHAR* name;
	
#ifdef UNICODE
	const char * format = "u";
#else
	const char * format = "s";
#endif
	
	if ( !PyArg_ParseTuple( args, format, &name ) )
		return NULL;

	theHold.Accept( name );

	Py_INCREF( Py_True );
	return Py_True;
}

/*

static bool file_in( CharStream* source, CharStream* log ) {
	init_thread_locals();
	push_alloc_frame();
	three_typed_value_locals(Parser* parser, Value* code, Value* result);
	save_current_frames();

	// these variables have been deprecated for maxscript in max 2012
#ifdef __MAXSCRIPT_2012__
	set_error_trace_back_disabled(FALSE);
	set_error_trace_back_active(TRUE);
	set_error_trace_back_level(10);
#else
	disable_trace_back = FALSE;
	trace_back_active = TRUE;
	trace_back_levels = 10;
#endif

	CharStream* out = thread_local(current_stdout);

	// loop through stream compiling & evaluating all expressions
	try {
		// make a new compiler instance
		vl.parser = new Parser(out);
		source->flush_whitespace();
		while (!source->at_eos() || vl.parser->back_tracked) {
			// In max 2012 this line will error out when the line return is only \n (a linux convention) see http://redmine.blur.com/issues/6446 for more details.
			vl.code		= vl.parser->compile(source);
			vl.result	= vl.code->eval();
			source->flush_whitespace();
		}
		if ( vl.parser->expr_level != 0 || vl.parser->back_tracked && vl.parser->token != t_end )
			throw;

		source->close();
	}
	catch (...) {
		// catch any errors and tell what file we are in if any
		out->puts("Unknown MAXScript Error occurred Occurred: ");
		source->sprin1(out);
		source->close();
		out->puts( "\n" );
		pop_alloc_frame();
		pop_value_locals();
		return false;
	}

	pop_alloc_frame();
	pop_value_locals();
	return true;
}

// Py3dsMax.runMaxscript
static PyObject*
studiomax_runMaxscript( PyObject* self, PyObject* args ) {
	char* fname;
	
	if ( !PyArg_ParseTuple( args, "s", &fname ) )
		return NULL;
#if	1
	one_typed_value_local(FileStream * file);

	// open a fileStream instance on the file
	vl.file = (new FileStream)->open( fname, "rt");
	if (vl.file == (FileStream*)&undefined) {
		PyErr_SetString( PyExc_RuntimeError, (TSTR("Py3dsMax.runMaxscript(filename): cannot open file - ") + fname) );
		pop_value_locals();
		return NULL;
	}
	
	bool result = 0;
	ExecuteScript(vl.file,&result);
	
	PyObject * out = result ? Py_True : Py_False;
	Py_INCREF(out);
#else
	// pick up arguments
	two_typed_value_locals(FileStream* file, StringStream* log );

	vl.log = new StringStream();
	
	// open a fileStream instance on the file
	vl.file = (new FileStream)->open( fname, "rt");
	if (vl.file == (FileStream*)&undefined) {
		PyErr_SetString( PyExc_RuntimeError, (TSTR("Py3dsMax.runMaxscript(filename): cannot open file - ") + fname) );
		pop_value_locals();
		return NULL;
	}

	// run using the stream-based filein utility
	PyObject* out = NULL;
	if ( !file_in( vl.file, vl.log ) ) {
		PyErr_SetString( PyExc_RuntimeError, vl.log->to_string() );
	}
	else {
		Py_INCREF(Py_True);
		out = Py_True;
	}
#endif

	pop_value_locals();
	return out;
}

*/

// Py3dsMax.getVisController() - get the visibility controller of a node
static PyObject*
studiomax_getVisController( PyObject* self, PyObject* args ) {
	PyObject * ret = 0;
	if ( PyTuple_Size(args) == 1 ) {
		// convert the input item to a maxscript value
		PyObject* item	= PyTuple_GetItem(args,0);
		MXS_PROTECT(one_value_local(obj));
		vl.obj		= ObjectWrapper::intern(item);

		if ( is_node(vl.obj) ) {
			ret = ObjectWrapper::py_intern( MAXControl::intern( ((MAXNode*)vl.obj)->node->GetVisController() ) );
		}
		MXS_CLEANUP();
	} else {
		PyErr_SetString( PyExc_AttributeError, "getVisController takes one argument, a max object." );
		return 0;
	}

	if( !ret ) {
		Py_INCREF( Py_None );
		ret = Py_None;
	}
	return ret;
}

// Py3dsMax.setVisController() - set the visibility controller of a node
static PyObject*
studiomax_setVisController( PyObject* self, PyObject* args ) {
	bool success = false;
	if ( PyTuple_Size(args) == 2 ) {
		// convert the input items to maxscript values
		PyObject* item	= PyTuple_GetItem(args,0);
		PyObject* pctl	= PyTuple_GetItem(args,1);

		Value* obj	= ObjectWrapper::intern(item);
		Value* ctrl = ObjectWrapper::intern(pctl);

		if ( is_node(obj) && is_controller(ctrl) ) {
			try {
				((MAXNode*) obj)->node->SetVisController(((MAXControl*) ctrl)->controller); 
				success = true;
			}
			catch ( ... ) {};
		}
	} else {
		PyErr_SetString( PyExc_AttributeError, "setVisController takes two arguments, a max object and a visibility controller." );
		return 0;
	}
	if ( success ) {
		Py_INCREF( Py_True );
		return Py_True;
	}
	else {
		Py_INCREF( Py_False );
		return Py_False;
	}
}

// Py3dsMax.getVersion() - gets the version number of Py3dsMax as a string
static PyObject*
studiomax_getVersion( PyObject* self ){
	PyObject* vstr = PyString_FromString(BLURPYTHON_VERSION_STR);
	return vstr;
}

// define the Py3dsMax module built-in methods
static PyMethodDef module_methods[] = {
	// windows methods
	{ "GetWindowHandle",	(PyCFunction)studiomax_getwindowhandle,		METH_NOARGS,	"Get the HWND value of the max window." },
	{ "GetPluginInstance",	(PyCFunction)studiomax_getplugininstance,	METH_NOARGS,	"Get the HINSTANCE value of the max window." },
	{ "DispatchMessage",	(PyCFunction)studiomax_dispatchmessage,		METH_VARARGS,	"Send the MAX Window a message." },

	// maxscript undo stack
	{ "redo",				(PyCFunction)studiomax_redo,				METH_NOARGS,	"Redo's the lastest stack." },
	{ "undo",				(PyCFunction)studiomax_undo,				METH_NOARGS,	"Undo's the latest stack." },
	{ "undoOn",				(PyCFunction)studiomax_undoOn,				METH_NOARGS,	"Starts a new undo stack." },
	{ "undoOff",			(PyCFunction)studiomax_undoOff,				METH_VARARGS,	"Finishes the current undo stack." },

	// maxscript workarounds
	{ "getVisController",	(PyCFunction)studiomax_getVisController,	METH_VARARGS,	"Get a nodes visibility controller" },
	{ "setVisController",	(PyCFunction)studiomax_setVisController,	METH_VARARGS,	"Set a nodes visibility controller" },

	// python methods
//	{ "runMaxscript",		(PyCFunction)studiomax_runMaxscript,		METH_VARARGS,	"Runs a maxscript file for proper error checking" },
	{ "runScript",			(PyCFunction)studiomax_runScript,           METH_VARARGS,   "Runs a python file in our global scope." },

	// Version Functions
	{ "getVersion",			(PyCFunction)studiomax_getVersion,			METH_NOARGS,	"Get the Py3dsMax version" },

	{ NULL, NULL, 0, NULL }
};

// initialize the plugin
PyMODINIT_FUNC
init_module(void) {
	// Step 1: initialize python
	Py_Initialize();

	// Step 2: make sure the mxs type is running
	if ( PyType_Ready(&MxsType) < 0 ) {
		return;
	}

	if ( PyType_Ready(&AtTimeType) < 0 ) {
		return;
	}
	
	// Step 3: make sure the object wrapper is running
	if ( !ObjectWrapper::init() ) {
		return;
	}

	// Step 4: initialize the Py3dsMax module
	PyObject* module = Py_InitModule3( "Py3dsMax", module_methods, "Python interface into 3d Studio Max" );

	if ( !module ) {
		return;
	}

	// Step 4: create the mxs class in the Py3dsMax module
	PyObject* instance = mxs_new( &MxsType, NULL, NULL );
	PyModule_AddObject( module, "mxs", instance );

	Py_INCREF(&AtTimeType);
	PyModule_AddObject( module, "AtTime", (PyObject*)&AtTimeType );
	
	mprintf( L"[blurPython] DLL has been successfully loaded.\n" );
}