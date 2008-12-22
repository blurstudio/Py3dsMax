#include "max/include/imports.h"
#include "strings.h"
#include <Python.h>

#include "../classes/ObjectValueWrapper.h"
#include "maxscrpt.h"
#include "hold.h"
#include "Name.h"

typedef struct {
	PyObject_HEAD
} MXSGlobals;

static void
MXSGlobals_dealloc( PyObject* self ) {
	self->ob_type->tp_free(self);
}

static PyObject *
MXSGlobals_new( PyTypeObject *type, PyObject *args, PyObject *kwds ) {
	MXSGlobals* self;
	self = (MXSGlobals *)type->tp_alloc(type, 0);
	return (PyObject *)self;
}

static int
MXSGlobals_init( PyObject* self, PyObject *args, PyObject *kwds ) {
	return 0;
}

static PyObject*
MXSGlobals_getattro( PyObject* self, PyObject* keyObj ) {
	char* key = PyString_AsString( keyObj );

	// Look up globals
	Value* result = globals->get( Name::intern( key ) );
	if ( result ) {
		return ObjectValueWrapper::pyintern( result );
	}
	
	// Return None value for non-found items
	Py_INCREF(Py_None);
	return Py_None;
}

static int
MXSGlobals_setattro( PyObject* self, PyObject* keyObj, PyObject* value ) {
	PyErr_Clear();

	char* key		= PyString_AsString( keyObj );

	Value* keyName	= Name::intern( key );
	Value* result	= (Value*) globals->get( keyName );

	// Set Existing Global Variable
	if ( result ) {
		// Set Thunk
		if ( is_thunk( result ) )				{ 
			try								{ ((Thunk*) result)->assign( ObjectValueWrapper::intern( value ) ); }
			CATCH_ERRORS( -1 );
		}
		else { globals->set( keyName, ObjectValueWrapper::intern(value) ); }
	}

	return 0;
}

static PyTypeObject MXSGlobalsType = {
    PyObject_HEAD_INIT(NULL)
    0,											// ob_size
	"Py3dsMax.mxs",								// tp_name
    sizeof(MXSGlobals),							// tp_basicsize
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
    (getattrofunc)MXSGlobals_getattro,			// tp_getattro
    (setattrofunc)MXSGlobals_setattro,			// tp_setattro
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
    (initproc)MXSGlobals_init,					// tp_init 
    0,											// tp_alloc 
    MXSGlobals_new,								// tp_new 
};

//------------------------------------------------------------------------------------------------------------------

typedef struct {
	PyObject_HEAD
} StdLog;

static void
StdLog_dealloc( StdLog* self ) {
	self->ob_type->tp_free((PyObject *)self);
}

static PyObject *
StdLog_new( PyTypeObject *type, PyObject *args, PyObject *kwds ) {
	StdLog* self;
	self = (StdLog *)type->tp_alloc(type, 0);
	return (PyObject *)self;
}

static int
StdLog_init( StdLog* self, PyObject *args, PyObject *kwds ) {
	return 0;
}

static PyObject *
StdLog_flush( StdLog* self ) {
	mflush();

	Py_INCREF( Py_None );
	return Py_None;
}

static PyObject *
StdLog_write( StdLog* self, PyObject *args ) {
	const char *command;

	if ( !PyArg_ParseTuple( args, "s",&command ) )
		return NULL;

	mprintf( TSTR(command) );

	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef StdLog_methods[] = {
	{ "flush", (PyCFunction)StdLog_flush, METH_NOARGS,	"Flushes the maxscript listener." },
	{ "write", (PyCFunction)StdLog_write, METH_VARARGS, "Writes a message to the maxscript listener." },
	{NULL}
};

static PyTypeObject StdLogType = {
    PyObject_HEAD_INIT(NULL)
    0,											// ob_size
    "Py3dsMax.StdLog",							// tp_name
    sizeof(StdLog),								// tp_basicsize
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
    0,											// tp_getattro
    0,											// tp_setattro
    0,											// tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	// tp_flags
    "MAXScript Error Logging Class",			// tp_doc 
    0,											// tp_traverse 
    0,											// tp_clear 
    0,											// tp_richcompare 
    0,											// tp_weaklistoffset 
    0,											// tp_iter 
    0,											// tp_iternext 
    StdLog_methods,								// tp_methods 
    0,											// tp_members 
    0,											// tp_getset 
    0,											// tp_base 
    0,											// tp_dict 
    0,											// tp_descr_get 
    0,											// tp_descr_set
    0,											// tp_dictoffset
    (initproc)StdLog_init,						// tp_init
    0,											// tp_alloc
    StdLog_new,									// tp_new
};

//------------------------------------------------------------------------------------------------------------------

static PyObject*
max_getwindowhandle( PyObject* self ) {
	return PyInt_FromLong( (long)GetCOREInterface()->GetMAXHWnd() );
}

static PyObject*
max_dispatchmessage( PyObject* self, PyObject* args ) {
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
		PyErr_SetString( PyExc_AttributeError, "DispatchMessage expects exactly 3 arguments" );
		return NULL;
	}
}

static PyObject *
max_redo( PyObject* self ) {
	ExecuteMAXScriptScript( "max redo" );

	Py_INCREF(Py_True);
	return Py_True;
}

static PyObject *
max_undo( PyObject* self ) {
	ExecuteMAXScriptScript( "max undo" );

	Py_INCREF(Py_True);
	return Py_True;
}

static PyObject *
max_undoOn( PyObject* self ) {
	theHold.Begin();

	Py_INCREF(Py_True);
	return Py_True;
}

static PyObject *
max_undoOff( PyObject* self, PyObject *args ) {
	char *name;

	if ( !PyArg_ParseTuple( args, "s",&name ) )
		return NULL;

	theHold.Accept( name );

	Py_INCREF( Py_True );
	return Py_True;
}

static PyMethodDef module_methods[] = {
	{ "GetWindowHandle",	(PyCFunction)max_getwindowhandle,	METH_NOARGS,	"Get the HWND value of the max window." },
	{ "DispatchMessage",	(PyCFunction)max_dispatchmessage,	METH_VARARGS,	"Send the MAX Window a message." },
	{ "redo",				(PyCFunction)max_redo,				METH_NOARGS,	"Redo's the lastest stack." },
	{ "undo",				(PyCFunction)max_undo,				METH_NOARGS,	"Undo's the latest stack." },
	{ "undoOn",				(PyCFunction)max_undoOn,			METH_NOARGS,	"Starts a new undo stack." },
	{ "undoOff",			(PyCFunction)max_undoOff,			METH_VARARGS,	"Finishes the current undo stack." },
	{ NULL, NULL, 0, NULL }
};

//------------------------------------------------------------------------------------------------------------------

static PyObject *MXSError;
PyMODINIT_FUNC
init_max(void) {
	PyObject* m;

    if (PyType_Ready(&StdLogType) < 0)
        return;

	if (PyType_Ready(&MXSGlobalsType) <0)
		return;
	
	if ( !ObjectValueWrapper::init() )
		return;

    m = Py_InitModule3("Py3dsMax", module_methods, "Creates a Python interface into 3dsMax.");

    if (m == NULL)
      return;

    Py_INCREF(&StdLogType);
    PyModule_AddObject(m, "StdLog", (PyObject *)&StdLogType);

	PyObject* mxs = MXSGlobals_new(&MXSGlobalsType, NULL, NULL);
	Py_INCREF(mxs);
	PyModule_AddObject(m, "mxs", mxs );
}