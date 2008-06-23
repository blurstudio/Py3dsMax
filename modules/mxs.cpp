#include "max/include/imports.h"
#include "strings.h"
#include <Python.h>

#include "../classes/ObjectValueWrapper.h"
#include "maxscrpt.h"
#include "Name.h"

typedef struct {
	PyObject_HEAD
} MXSGlobals;

static void
MXSGlobals_dealloc( MXSGlobals* self ) {
	self->ob_type->tp_free((PyObject *)self);
}

static PyObject*
MXSGlobals_isglobal( PyObject* self, PyObject* args ) {
	char *command;

	if ( !PyArg_ParseTuple( args, "s",&command ) )
		return NULL;
	
	Value* result = globals->get( Name::intern( command ) );
	if ( result ) {
		Py_INCREF( Py_True );
		return Py_True;
	}
	Py_INCREF( Py_False );
	return Py_False;
}

static PyMethodDef MXSGlobals_methods[] = {
	{ "isGlobal",	(PyCFunction)MXSGlobals_isglobal, METH_VARARGS,	"Checks to see if a given string is a maxscript global." },
	{NULL}
};

static PyObject *
MXSGlobals_new( PyTypeObject *type, PyObject *args, PyObject *kwds ) {
	MXSGlobals* self;
	self = (MXSGlobals *)type->tp_alloc(type, 0);
	return (PyObject *)self;
}

static int
MXSGlobals_init( MXSGlobals* self, PyObject *args, PyObject *kwds ) {
	return 0;
}

static PyObject*
MXSGlobals_getattr( MXSGlobals* self, char* key ) {
	// Look up C++ methods first
	PyObject* out	= Py_FindMethod( MXSGlobals_methods, (PyObject*) self, key );
	if ( out ) { return out; }

	// Look up MAXScript members
	Value* result	= (Value*) globals->get( Name::intern( key ) );
	if (!result) {
		PyErr_SetString( PyExc_AttributeError, key );
		return NULL;
	}
	return ObjectValueWrapper::pyintern( result );
}

static int
MXSGlobals_setattr( MXSGlobals* self, char* key, PyObject* value ) {
	Value* keyName	= Name::intern( key );
	Value* result	= (Value*) globals->get( keyName );

	if ( result ) {
		if ( is_thunk( result ) )				{ 
			try { ((Thunk*) result)->assign( ObjectValueWrapper::intern( value ) ); }
			catch ( AssignToConstError e ) { THROW_PYERROR( e, PyExc_AttributeError, -1 ); }
		}
		else									{ globals->set( keyName, ObjectValueWrapper::intern(value) ); }
	}
	else { 
		one_typed_value_local( GlobalThunk* thunk );
		vl.thunk = new GlobalThunk( keyName, ObjectValueWrapper::intern(value) );
		globals->set( keyName, vl.thunk->make_heap_permanent() ); 
		pop_value_locals();
	}
	return 0;
}

static PyTypeObject MXSGlobalsType = {
    PyObject_HEAD_INIT(NULL)
    0,											/*ob_size*/
	"Py3dsMax.mxs",								/*tp_name*/
    sizeof(MXSGlobals),							/*tp_basicsize*/
    0,											/*tp_itemsize*/
    0,											/*tp_dealloc*/
    0,											/*tp_print*/
    (getattrfunc)MXSGlobals_getattr,			/*tp_getattr*/
	(setattrfunc)MXSGlobals_setattr,			/*tp_setattr*/
    0,											/*tp_compare*/
    0,											/*tp_repr*/
    0,											/*tp_as_number*/
    0,											/*tp_as_sequence*/
    0,											/*tp_as_mapping*/
    0,											/*tp_hash */
    0,											/*tp_call*/
    0,											/*tp_str*/
    0,											/*tp_getattro*/
    0,											/*tp_setattro*/
    0,											/*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/*tp_flags*/
    "MAXScript Globals Accessor",				/* tp_doc */
    0,											/* tp_traverse */
    0,											/* tp_clear */
    0,											/* tp_richcompare */
    0,											/* tp_weaklistoffset */
    0,											/* tp_iter */
    0,											/* tp_iternext */
    MXSGlobals_methods,							/* tp_methods */
    0,											/* tp_members */
    0,											/* tp_getset */
    0,											/* tp_base */
    0,											/* tp_dict */
    0,											/* tp_descr_get */
    0,											/* tp_descr_set */
    0,											/* tp_dictoffset */
    (initproc)MXSGlobals_init,					/* tp_init */
    0,											/* tp_alloc */
    MXSGlobals_new,									/* tp_new */
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
    0,											/*ob_size*/
    "Py3dsMax.StdLog",							/*tp_name*/
    sizeof(StdLog),								/*tp_basicsize*/
    0,											/*tp_itemsize*/
    0,											/*tp_dealloc*/
    0,											/*tp_print*/
    0,											/*tp_getattr*/
    0,											/*tp_setattr*/
    0,											/*tp_compare*/
    0,											/*tp_repr*/
    0,											/*tp_as_number*/
    0,											/*tp_as_sequence*/
    0,											/*tp_as_mapping*/
    0,											/*tp_hash */
    0,											/*tp_call*/
    0,											/*tp_str*/
    0,											/*tp_getattro*/
    0,											/*tp_setattro*/
    0,											/*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/*tp_flags*/
    "MAXScript Error Logging Class",			/* tp_doc */
    0,											/* tp_traverse */
    0,											/* tp_clear */
    0,											/* tp_richcompare */
    0,											/* tp_weaklistoffset */
    0,											/* tp_iter */
    0,											/* tp_iternext */
    StdLog_methods,								/* tp_methods */
    0,											/* tp_members */
    0,											/* tp_getset */
    0,											/* tp_base */
    0,											/* tp_dict */
    0,											/* tp_descr_get */
    0,											/* tp_descr_set */
    0,											/* tp_dictoffset */
    (initproc)StdLog_init,						/* tp_init */
    0,											/* tp_alloc */
    StdLog_new,									/* tp_new */
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

static PyMethodDef module_methods[] = {
	{ "GetWindowHandle",	(PyCFunction)max_getwindowhandle,	METH_NOARGS,	"Get the HWND value of the max window." },
	{ "DispatchMessage",	(PyCFunction)max_dispatchmessage,	METH_VARARGS,	"Send the MAX Window a message." },
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