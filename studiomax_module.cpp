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
#include "protector.h"
#include "wrapper.h"

// define the mxs class
typedef struct {
	PyObject_HEAD
} mxs;

// ctor
static PyObject*
mxs_new( PyTypeObject* type, PyObject* args, PyObject* kwds ) {
	mxs* self;
	self = (mxs*)type->tp_alloc(type, 0);
	return (PyObject*) self;
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
	char* keystr	= PyString_AsString( key );
	Value* name		= Name::intern( keystr );

	// Step 2: collect the PyObject* instance
	PyObject* output = ObjectWrapper::py_intern( globals->get( name ) );

	return output;
}

// __setattr__: set an item in the maxscript globals hash table
static int
mxs_setattro( PyObject* self, PyObject* key, PyObject* value ) {
	// Step 1: convert the key to a name
	char* keystr	= PyString_AsString( key );

	// Step 2: protect the maxscript memory
	MXS_PROTECT( two_value_locals( name, result ) );
	vl.name		= Name::intern( keystr );

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
	return PyInt_FromLong( (long)hInstance );
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
	ExecuteMAXScriptScript( "max redo" );

	Py_INCREF( Py_True );
	return Py_True;
}

// Py3dsMax.runScript() - runs a script file
static PyObject*
studiomax_runScript( PyObject* self, PyObject* args ) {
	char* filename = NULL;

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
	PY_CLEARERRORS();

	// Step 4: clear the memory
	Py_XDECREF( py_file );

	// return true
	Py_INCREF( Py_True );
	return Py_True;
}

// Py3dsMax.undo() - undoes the last action
static PyObject*
studiomax_undo( PyObject* self ) {
	ExecuteMAXScriptScript( "max undo" );

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
	char* name;
	
	if ( !PyArg_ParseTuple( args, "s", &name ) )
		return NULL;

	theHold.Accept( name );

	Py_INCREF( Py_True );
	return Py_True;
}

// Py3dsMax.getVisController() - get the visibility controller of a node
static PyObject*
studiomax_getVisController( PyObject* self, PyObject* args ) {
	if ( PyTuple_Size(args) == 1 ) {
		// convert the input item to a maxscript value
		PyObject* item	= PyTuple_GetItem(args,0);
		Value* obj		= ObjectWrapper::intern(item);

		if ( is_node(obj) ) {
			return ObjectWrapper::py_intern( MAXControl::intern( ((MAXNode*)obj)->node->GetVisController() ) );
		}
	}

	Py_INCREF( Py_None );
	return Py_None;
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
	{ "runScript",			(PyCFunction)studiomax_runScript,           METH_VARARGS,   "Runs a python file in our global scope." },

	// Version Functions
	{ "getVersion",			(PyCFunction)studiomax_getVersion,			METH_NOARGS,	"Get the Py3dsMax version" },

	{ NULL, NULL, 0, NULL }
};

// initialize the plugin
PyMODINIT_FUNC
init_module(void) {
	// Step 0: initialize the protector
	Protector::init();

	// Step 1: initialize python
	Py_Initialize();

	// Step 2: make sure the mxs type is running
	if ( PyType_Ready(&MxsType) < 0 ) {
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

	mprintf( "[blurPython] DLL has been successfully loaded.\n" );
}