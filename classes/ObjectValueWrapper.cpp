// 3dsMax Includes
#include "max\include\imports.h"
#include "strings.h"
#include "objsets.h"
#include "Parser.h"
#include <Python.h>
#include <algorithm>

// BlurPython Includes
#include "max\include\exports.h"
#include "ObjectValueWrapper.h"

//-------------------------------------------------------------------------------------------------------------------------------

typedef struct {
	PyObject_HEAD
	Value* value;
} MXSValueWrapper;

static PyObject *
MXSValueWrapper_new( PyTypeObject *type, PyObject *args, PyObject *kwds ) {
	MXSValueWrapper* self;
	self = (MXSValueWrapper *)type->tp_alloc(type, 0);
	self->value = NULL;
	return (PyObject *)self;
}

static PyObject*
MXSValueWrapper_call( MXSValueWrapper* self, PyObject *args, PyObject *kwds ) {
	Value* result = NULL;

	//----------------------------------------------

	int count			= (args )					? PyTuple_Size( args )	: 0;
	int keyword_count	= (kwds)					? PyDict_Size( kwds )	: -1;
	int total			= ( keyword_count == -1 )	? count					: count + 1 + (keyword_count*2);

	if ( total ) {
		// Create arguments and keywords local Value** array
		Value** arg_list;
		init_thread_locals();
		push_alloc_frame();
		value_local_array( arg_list, total );

		// Build Arguments
		for ( int i = 0; i < count; i++ )
			arg_list[i] = ObjectValueWrapper::intern( PyTuple_GetItem( args, i ) );

		// Build Keywords
		if ( keyword_count != -1 ) {
			PyObject *key, *value;
			int pos		= 0;
			int key_pos = 0;
			arg_list[ count ] = &keyarg_marker;
			while ( PyDict_Next( kwds, &pos, &key, &value ) ) {
				arg_list[ count + 1 + (key_pos*2) ]	= Name::intern( PyString_AsString( key ) );
				arg_list[ count + 2 + (key_pos*2) ]	= ObjectValueWrapper::intern( value );
				key_pos++;
			}
		}

		//----------------------------------------------

		try										{ result = self->value->eval()->apply( arg_list, total ); }
		catch ( AccessorError e )				{ 
			pop_value_local_array( arg_list );
			pop_alloc_frame();
			THROW_PYERROR( e, PyExc_Exception, NULL ); 
		}
		catch ( ArgCountError e )				{ 
			pop_value_local_array( arg_list );
			pop_alloc_frame();
			THROW_PYERROR( e, PyExc_Exception, NULL ); 
		}
		catch ( AssignToConstError e )			{ 
			pop_value_local_array( arg_list );
			pop_alloc_frame();
			THROW_PYERROR( e, PyExc_Exception, NULL ); 
		}
		catch ( CompileError e )				{ 
			pop_value_local_array( arg_list );
			pop_alloc_frame();
			THROW_PYERROR( e, PyExc_Exception, NULL ); 
		}
		catch ( ConversionError e )				{ 
			pop_value_local_array( arg_list );
			pop_alloc_frame();
			THROW_PYERROR( e, PyExc_Exception, NULL ); 
		}
		catch ( DebuggerRuntimeError e )		{ 
			pop_value_local_array( arg_list );
			pop_alloc_frame();
			THROW_PYERROR( e, PyExc_Exception, NULL ); 
		}
		catch ( IncompatibleTypes e )			{ 
			pop_value_local_array( arg_list );
			pop_alloc_frame();
			THROW_PYERROR( e, PyExc_Exception, NULL ); 
		}
		catch ( NoMethodError e	)				{ 
			pop_value_local_array( arg_list );
			pop_alloc_frame();
			THROW_PYERROR( e, PyExc_Exception, NULL ); 
		}
		catch ( RuntimeError e )				{ 
			pop_value_local_array( arg_list );
			pop_alloc_frame();
			THROW_PYERROR( e, PyExc_RuntimeError, NULL ); 
		}
		catch ( SignalException e )				{ 
			pop_value_local_array( arg_list );
			pop_alloc_frame();
			THROW_PYERROR( e, PyExc_Exception, NULL ); 
		}
		catch ( TypeError e )					{ 
			pop_value_local_array( arg_list );
			pop_alloc_frame();
			THROW_PYERROR( e, PyExc_Exception, NULL ); 
		}
		catch ( UnknownSystemException e )		{ 
			pop_value_local_array( arg_list );
			pop_alloc_frame();
			THROW_PYERROR( e, PyExc_SystemError, NULL ); 
		}
		catch ( UserThrownError e )				{ 
			pop_value_local_array( arg_list );
			pop_alloc_frame();
			THROW_PYERROR( e, PyExc_Exception, NULL ); 
		}
		catch ( MAXScriptException e )			{ 
			pop_value_local_array( arg_list );
			pop_alloc_frame();
			THROW_PYERROR( e, PyExc_Exception, NULL ); 
		}
		catch ( ... ) { 
			pop_value_local_array( arg_list );
			pop_alloc_frame();
			PyErr_SetString( PyExc_RuntimeError, "Unknown Error Occured." );
			return NULL;
		}
		
		pop_value_local_array( arg_list );
		pop_alloc_frame();
		return ObjectValueWrapper::pyintern( result );
	}
	else {
		try										{ result = self->value->eval()->apply( NULL, 0 ); }
		catch ( AccessorError e )				{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
		catch ( ArgCountError e )				{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
		catch ( AssignToConstError e )			{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
		catch ( CompileError e )				{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
		catch ( ConversionError e )				{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
		catch ( DebuggerRuntimeError e )		{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
		catch ( IncompatibleTypes e )			{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
		catch ( NoMethodError e	)				{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
		catch ( RuntimeError e )				{ THROW_PYERROR( e, PyExc_RuntimeError, NULL ); }
		catch ( SignalException e )				{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
		catch ( TypeError e )					{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
		catch ( UnknownSystemException e )		{ THROW_PYERROR( e, PyExc_SystemError, NULL ); }
		catch ( UserThrownError e )				{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
		catch ( MAXScriptException e )			{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
		catch ( ... ) { 
			PyErr_SetString( PyExc_RuntimeError, "Unknown Error Occured." );
			return NULL;
		}
		return ObjectValueWrapper::pyintern( result );
	}
}

static int
MXSValueWrapper_compare( MXSValueWrapper* self, MXSValueWrapper* other ) {
	int result = 1;
	if ( self->value->eval() == other->value->eval() ) { result = 0; }
	return result;
}

static void
MXSValueWrapper_dealloc( MXSValueWrapper* self ) {
	self->ob_type->tp_free((PyObject *)self);
	self->value->gc_trace();
}

static PyObject*
MXSValueWrapper_getattr( MXSValueWrapper* self, char* key ) {
	Value* result;

	try										{ result = self->value->eval()->_get_property( Name::intern( key ) ); }
	catch ( AccessorError e )				{ THROW_PYERROR( e, PyExc_AttributeError, NULL ); }
	catch ( ArgCountError e )				{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
	catch ( AssignToConstError e )			{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
	catch ( CompileError e )				{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
	catch ( ConversionError e )				{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
	catch ( DebuggerRuntimeError e )		{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
	catch ( IncompatibleTypes e )			{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
	catch ( NoMethodError e	)				{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
	catch ( RuntimeError e )				{ THROW_PYERROR( e, PyExc_RuntimeError, NULL ); }
	catch ( SignalException e )				{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
	catch ( TypeError e )					{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
	catch ( UnknownSystemException e )		{ THROW_PYERROR( e, PyExc_SystemError, NULL ); }
	catch ( UserThrownError e )				{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
	catch ( MAXScriptException e )			{ THROW_PYERROR( e, PyExc_Exception, NULL ); }
	catch ( ... ) { 
		PyErr_SetString( PyExc_RuntimeError, "Unknown Error Occured." );
		return NULL;
	}
	if ( !result ) {
		PyErr_SetString( PyExc_AttributeError, TSTR( "Cannot get property: " ) + key );
		return NULL;
	}
	return ObjectValueWrapper::pyintern( result );
}

static int
MXSValueWrapper_setattr( MXSValueWrapper* self, char* key, PyObject* value ) {
	Value* result;
	try										{ result = self->value->_set_property( Name::intern( key ), ObjectValueWrapper::intern( value ) ); }
	catch ( AccessorError e )				{ THROW_PYERROR( e, PyExc_AttributeError, -1 ); }
	catch ( ArgCountError e )				{ THROW_PYERROR( e, PyExc_Exception, -1 ); }
	catch ( AssignToConstError e )			{ THROW_PYERROR( e, PyExc_Exception, -1 ); }
	catch ( CompileError e )				{ THROW_PYERROR( e, PyExc_Exception, -1 ); }
	catch ( ConversionError e )				{ THROW_PYERROR( e, PyExc_Exception, -1 ); }
	catch ( DebuggerRuntimeError e )		{ THROW_PYERROR( e, PyExc_Exception, -1 ); }
	catch ( IncompatibleTypes e )			{ THROW_PYERROR( e, PyExc_Exception, -1 ); }
	catch ( NoMethodError e	)				{ THROW_PYERROR( e, PyExc_Exception, -1 ); }
	catch ( RuntimeError e )				{ THROW_PYERROR( e, PyExc_RuntimeError, -1 ); }
	catch ( SignalException e )				{ THROW_PYERROR( e, PyExc_Exception, -1 ); }
	catch ( TypeError e )					{ THROW_PYERROR( e, PyExc_Exception, -1 ); }
	catch ( UnknownSystemException e )		{ THROW_PYERROR( e, PyExc_SystemError, -1 ); }
	catch ( UserThrownError e )				{ THROW_PYERROR( e, PyExc_Exception, -1 ); }
	catch ( MAXScriptException e )			{ THROW_PYERROR( e, PyExc_Exception, -1 ); }
	catch ( ... ) { 
		PyErr_SetString( PyExc_RuntimeError, "Unknown Error Occured." );
		return -1;
	}
	if ( !result ) {
		PyErr_SetString( PyExc_AttributeError, TSTR( "Cannot get property: " ) + key );
		return NULL;
	}
	return 0;
}

static PyObject*
MXSValueWrapper_str( MXSValueWrapper* self ) {
	StringStream* s = new StringStream(0);
	s->puts( "<mxs (" );
	self->value->eval()->sprin1(s);
	s->puts( ")>" );
	return PyString_FromString( s->to_string() );
}

static PyTypeObject MXSValueWrapperType = {
    PyObject_HEAD_INIT(NULL)
    0,											/*ob_size*/
    "mxs",										/*tp_name*/
    sizeof(MXSValueWrapper),					/*tp_basicsize*/
    0,											/*tp_itemsize*/
    (destructor)MXSValueWrapper_dealloc,		/*tp_dealloc*/
    0,											/*tp_print*/
    (getattrfunc)MXSValueWrapper_getattr,		/*tp_getattr*/
    (setattrfunc)MXSValueWrapper_setattr,		/*tp_setattr*/
    (cmpfunc)MXSValueWrapper_compare,			/*tp_compare*/
    0,											/*tp_repr*/
    0,											/*tp_as_number*/
    0,											/*tp_as_sequence*/
    0,											/*tp_as_mapping*/
    0,											/*tp_hash */
    (ternaryfunc)MXSValueWrapper_call,			/*tp_call*/
    (reprfunc)MXSValueWrapper_str,				/*tp_str*/
    0,											/*tp_getattro*/
    0,											/*tp_setattro*/
    0,											/*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/*tp_flags*/
    "Maxscript Value Wrapper",					/* tp_doc */
    0,											/* tp_traverse */
    0,											/* tp_clear */
    0,											/* tp_richcompare */
    0,											/* tp_weaklistoffset */
    0,											/* tp_iter */
    0,											/* tp_iternext */
    0,											/* tp_methods */
    0,											/* tp_members */
    0,											/* tp_getset */
    0,											/* tp_base */
    0,											/* tp_dict */
    0,											/* tp_descr_get */
    0,											/* tp_descr_set */
    0,											/* tp_dictoffset */
    0,											/* tp_init */
    0,											/* tp_alloc */
    MXSValueWrapper_new,						/* tp_new */
};

//------------------------------------------------------------------------------------------------------------------

visible_class_instance( ObjectValueWrapper, "PyObject" );
ObjectValueWrapper::ObjectValueWrapper( PyObject* pyobj ) { 
	this->tag		= class_tag(ObjectValueWrapper);
	this->_pyobj	= pyobj;
}
ObjectValueWrapper::~ObjectValueWrapper() { Py_XDECREF( this->_pyobj ); }
Value*		ObjectValueWrapper::apply(				Value** arg_list, int count, CallContext* cc ) {
	if ( PyCallable_Check( this->_pyobj ) ) {
		PyObject *pargs, *result;
		pargs	= ObjectValueWrapper::args( arg_list, count );
		result	= PyEval_CallObject( this->_pyobj, pargs );
		Py_DECREF( pargs );
		return ObjectValueWrapper::intern( result );
	}
	return Value::apply( arg_list, count, cc );
}
PyObject*	ObjectValueWrapper::args(				Value** arg_list, int count ) {
	PyObject *out;
	out = PyTuple_New( count );
	for ( int i = 0; i < count; i++ ) PyTuple_SetItem( out, i, ObjectValueWrapper::pyintern( arg_list[i]->eval() ) );
	return out;
}
Value*		ObjectValueWrapper::get_property(		Value** arg_list, int count ) {
	Value* key = arg_list[0]->eval();

	// Get Default Properties
	if ( key == n_count ) {
		if ( PyMapping_Check( this->_pyobj ) )			return Integer::intern( PyMapping_Length( this->_pyobj ) );
		else if ( PySequence_Check( this->_pyobj ) )	return Integer::intern( PySequence_Length( this->_pyobj ) );
		else throw RuntimeError( TSTR( "__len__ method is not defined for " ) + this->to_string() );
	}
	else {
		// Simple check, assumes case is correct
		if ( PyObject_HasAttrString( this->_pyobj, key->to_string() ) )
			return ObjectValueWrapper::intern( PyObject_GetAttrString( this->_pyobj, key->to_string() ) );
		// Advanced lookup, since MAXScript is caseInsensitive
		else
			return this->lookup( key->to_string() );
	}
}
Value*		ObjectValueWrapper::get_vf(				Value** arg_list, int count ) {
	PyObject *pkey, *presult;

	Value* key = arg_list[0]->eval();
	if ( is_number( key ) ) pkey = PyInt_FromLong(		key->to_int() - 1 );
	else					pkey = PyString_FromString( key->to_string() );

	presult	= PyObject_GetItem( this->_pyobj, pkey );
	Py_DECREF( pkey );

	return ObjectValueWrapper::intern( presult );
}
Value*		ObjectValueWrapper::get_props_vf(		Value** arg_list, int count ) {
	return ObjectValueWrapper::intern( PyObject_Dir( this->_pyobj ) );
}
Value*		ObjectValueWrapper::eq_vf(				Value** arg_list, int count ) {
	check_arg_count( eq, 1, count );
	if ( is_pyobject(arg_list[0]->eval()) )
		return (((ObjectValueWrapper*) arg_list[0]->eval())->_pyobj == this->_pyobj) ? &true_value : &false_value;
	return (arg_list[0] == this) ? &true_value : &false_value;
}
Value*		ObjectValueWrapper::lookup(				std::string key ) {
	// convert key to lowercase
	std::string check;
	char *checkChars;
	PyObject* k;
	std::transform( key.begin(), key.end(), key.begin(), tolower );

	// Build key mapping since MAXScript is case insensitive
	PyObject* keys	= PyObject_Dir( this->_pyobj );

	int count		= PySequence_Length( keys );
	for ( int i = 0; i < count; i++ ) {
		k = PySequence_GetItem( keys, i );
		PyArg_Parse( k, "s", &checkChars );

		check = std::string( checkChars );
		// create lowercase check
		std::transform( check.begin(), check.end(), check.begin(), tolower );
		if ( check == key ) {
			Py_DECREF( keys );
			Py_DECREF( k );
			return ObjectValueWrapper::intern( PyObject_GetAttrString( this->_pyobj, checkChars ) );
		}
	}
	Py_DECREF( keys );
	Py_DECREF( k );
	throw RuntimeError( (TSTR) key.c_str() + " is not a member of " + this->to_string() );
}
Value*		ObjectValueWrapper::put_vf(				Value** arg_list, int count ) {
	return &ok;
}
bool		ObjectValueWrapper::init() {
	if ( PyType_Ready(&MXSValueWrapperType) < 0 )
		return false;
	return true;
}
Value*		ObjectValueWrapper::intern(				PyObject* item ) {
	if ( item && item != Py_None ) {
		// Convert to basic MAX types
		if ( item->ob_type == &MXSValueWrapperType )		{ return ((MXSValueWrapper*) item)->value; }
		else if ( item->ob_type == &PyString_Type || item->ob_type == &PyUnicode_Type ) {
			one_value_local( out );
			vl.out = new String( PyString_AsString( item ) );
			return_value( vl.out );
		}
		else if ( item->ob_type == &PyInt_Type )			{ return Integer::intern( PyInt_AsLong( item ) ); }
		else if ( item->ob_type == &PyFloat_Type )			{ return Float::intern( PyFloat_AsDouble( item ) ); }
		else if ( item->ob_type == &PyBool_Type )			{ return ( item == Py_True ) ? &true_value : &false_value; }
		else if ( item->ob_type == &PyList_Type || item->ob_type == &PyTuple_Type )	{
			one_typed_value_local( Array* out );
			int count = PyObject_Length( item );
			vl.out = new Array(count);
			for ( int i = 0; i < count; i++ )
				vl.out->append( ObjectValueWrapper::intern( PySequence_GetItem( item, i ) ) );
			return_value( vl.out );
		}
		else {
			one_value_local( out );
			vl.out = new ObjectValueWrapper( item );
			return_value( vl.out );
		}
	}
	return &undefined;
}
PyObject*	ObjectValueWrapper::pyintern( Value* item )		{
	//-------------------------------------------------------------

	if			( item ) {
		Value* eval_item = item->eval();
		if		( is_pyobject( eval_item ) )								{ ((ObjectValueWrapper*) eval_item)->pyobject(); }
		else if	( is_string( eval_item ) || is_name( eval_item ) )			{ return PyString_FromString( eval_item->to_string() ); }
		else if ( is_integer( eval_item ) )									{ return PyInt_FromLong( eval_item->to_int() ); }
		else if ( is_float( eval_item ) )									{ return PyFloat_FromDouble( eval_item->to_float() ); }
		else if ( is_array( eval_item ) )								{
			int count		= ((Array*) eval_item)->size;
			PyObject* out	= PyList_New( count );
			for ( int i = 0; i < count; i++ ) PyList_SetItem( out, i, ObjectValueWrapper::pyintern( ((Array*) eval_item)->data[i] ) );
			return out;
		}
		else if ( is_collection( eval_item ) )									{
			NodeTab nodeTab;
			nodeTab.SetCount(0);
			Value* args[2]	= { NULL, (Value*)&nodeTab };
			node_map m		= { NULL, collect_nodes, args, 2 };
			eval_item->map(m);

			PyObject* out	= PyList_New(0);
			for( int i = 0; i < nodeTab.Count(); i++ ) { if ( nodeTab[i] ) PyList_Append( out, ObjectValueWrapper::pyintern( MAXNode::intern(nodeTab[i]) ) ); };
			
			return out;
		}
		else if ( eval_item == &ok || eval_item == &true_value )		{
			Py_INCREF( Py_True );
			return Py_True;
		}
		else if ( eval_item == &false_value ) {
			Py_INCREF( Py_False );
			return Py_False;
		}
		else if ( eval_item == &undefined )										{
			Py_INCREF( Py_None );
			return Py_None;
		}
		else {
			MXSValueWrapper* wrapper	= (MXSValueWrapper*) MXSValueWrapper_new( &MXSValueWrapperType, NULL, NULL );
			wrapper->value				= item;
			wrapper->value->mark_in_use();
			return (PyObject*) wrapper;
		}
	}

	Py_INCREF(Py_None);
	return Py_None;
}
PyObject*	ObjectValueWrapper::pyobject()					{ return this->_pyobj; }
void		ObjectValueWrapper::sprin1( CharStream* s )		{ s->puts( this->to_string() ); }
char*		ObjectValueWrapper::to_string()					{ 
	PyObject* pstr;
	pstr = PyObject_Str( this->_pyobj );
	if ( pstr ) {
		char *out;
		PyArg_Parse( pstr, "s", &out );
		Py_DECREF( pstr );
		return out;
	}
	return "<<python: error printing value>>";
}