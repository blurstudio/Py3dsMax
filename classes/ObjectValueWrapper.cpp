// 3dsMax Includes
#include "max\include\imports.h"
#include "strings.h"
#include "objsets.h"
#include "Parser.h"
#include "maxobj.h"
#include "mathpro.h"

#include <Python.h>
#include <algorithm>

// BlurPython Includes
#include "max\include\exports.h"
#include "ObjectValueWrapper.h"

// Use if compiling for Python24 mappings
typedef inquiry			lenfunc;
typedef int				Py_ssize_t;
typedef intobjargproc	ssizeobjargproc;
typedef	intargfunc		ssizeargfunc;

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

			Py_ssize_t pos = 0;			// Use for Python25
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
		CATCH_ERRORS( NULL );

		return ObjectValueWrapper::pyintern( result );
	}
}

// __cmp__
static int
MXSValueWrapper_compare( PyObject* self, PyObject* other ) {
	int result = -1;

	Value* mCheck	= ((MXSValueWrapper*) self)->value;
	Value* oCheck	= ObjectValueWrapper::intern( other );

	// Check the inputed value against the current value
	if ( oCheck ) {
		// Check direct pointers
		if ( mCheck == oCheck || mCheck->eval() == oCheck->eval() )		{ result = 0; }
		else {
			// Check MAXScript __lt__, __eq__, __gt__
			try { 
				if		( mCheck->eq_vf( &oCheck, 1 ) == &true_value )  { result = 0; } //( mCheck == oCheck ) { result = 0; } 
				else if	( mCheck->lt_vf( &oCheck, 1 ) == &true_value )	{ result = -1; }
				else if ( mCheck->gt_vf( &oCheck, 1 ) == &true_value )	{ result = 1; }
			}
			catch ( ... ) {}
		}
	}

	return result;
}

static void
MXSValueWrapper_dealloc( MXSValueWrapper* self ) {
	self->ob_type->tp_free((PyObject *)self);
	self->value->make_collectable();
	self->value = NULL;
}

// __getattr__
static PyObject*
MXSValueWrapper_getattr( MXSValueWrapper* self, char* key ) {
	Value* result;
	Value* keyName = Name::intern( key );

	// Map generic items
	try	{ result = self->value->eval()->_get_property( keyName ); }
	CATCH_ERRORS( NULL );

	if ( !result ) {
		PyErr_SetString( PyExc_AttributeError, key );
		return NULL;
	}
	return ObjectValueWrapper::pyintern( result );
}

// __setattr__
static int
MXSValueWrapper_setattr( MXSValueWrapper* self, char* key, PyObject* value ) {
	Value* result;
	Value* check	= self->value->eval();
	Value* keyName	= Name::intern( key );
	Value* maxValue	= ObjectValueWrapper::intern( value );

	// Set Controllers
	if ( keyName == Name::intern( "controller" ) && is_subAnim( check ) ) {
		if ( is_controller( maxValue ) ) {
			try { ((MAXSubAnim*) check)->set_max_controller( (MAXControl*) maxValue ); }
			CATCH_ERRORS( NULL );
		}
	}

	try										{ result = check->_set_property( keyName, maxValue ); }
	CATCH_ERRORS( NULL );

	if ( !result ) {
		PyErr_SetString( PyExc_AttributeError, key );
		return NULL;
	}
	return 0;
}

// __str__
static PyObject*
MXSValueWrapper_str( MXSValueWrapper* self ) {
	try {
		StringStream* s = new StringStream(0);
		s->puts( "<mxs." );
		self->value->eval()->tag->sprin1(s);
		s->puts( " " );
		self->value->eval()->sprin1(s);
		s->puts( ">" );
		return PyString_FromString( s->to_string() );
	}
	CATCH_ERRORS(NULL);
}

//----------------------------------------------			SEQUENCE METHODS				------------------------------------------------

// __len__
static int
MXSValueWrapper_length( PyObject* self )		{ 
	Value* check = ((MXSValueWrapper*) self)->value->eval();

	// Calculate Array size
	if ( is_array( check ) )			{ return (int) ((Array*) check)->size; }

	// Calculate Collection size
	else {
		try { return check->_get_property( Name::intern( "count" ) )->to_int(); }
		CATCH_ERRORS(-1);
	}
	
	PyErr_SetString( PyExc_TypeError, TSTR( "MXSValueWrapper_item: Cannot convert to <list>: " ) + PyString_AsString( MXSValueWrapper_str( (MXSValueWrapper*) self )) );
	return -1;
}

// __getitem__
static PyObject*
MXSValueWrapper_objitem( PyObject* self, PyObject* key ) {
	Value* check = ((MXSValueWrapper*) self)->value->eval();

	// Grab an item by index
	if ( key->ob_type == &PyInt_Type ) {
		// Grab the collection's count
		int count;
		int index = (int) PyInt_AsLong( key );
		try { count = check->_get_property( Name::intern( "count" ) )->to_int(); }
		CATCH_ERRORS(NULL);

		if ( 0 <= index && index < count ) {
			Value* maxIndex		= Integer::intern( index + 1 );
			return ObjectValueWrapper::pyintern( check->get_vf( &maxIndex, 1 ) );
		}

		PyErr_SetString( PyExc_IndexError, TSTR( "__getitem__ error: index is out of range" ) );
		return NULL;
	}

	// Grab an item by some other mapping
	else {
		Value* keyValue = ObjectValueWrapper::intern( key );
		try { return ObjectValueWrapper::pyintern( check->get_vf( &keyValue, 1 ) ); }
		CATCH_ERRORS(NULL);
	}
}
static PyObject*
MXSValueWrapper_item( PyObject* self, int index ) { return MXSValueWrapper_objitem( self, PyInt_FromLong( index ) ); }

// __setitem__
static int
MXSValueWrapper_setobjitem( PyObject* self, PyObject* key, PyObject* value ) {
	Value* check = ((MXSValueWrapper*) self)->value->eval();

	Value* keyValue;

	// Convert to index
	if ( key->ob_type == &PyInt_Type ) {
		int index		= (int) PyInt_AsLong( key );
		int count		= MXSValueWrapper_length( self );
		if ( 0 <= index && index < count )
			keyValue	= Integer::intern( index );
		else {
			PyErr_SetString( PyExc_IndexError, "__setitem__ error: index is out of range" );
			return -1;
		}
	}
	// Keep as key
	else keyValue		= ObjectValueWrapper::intern( key );

	// Build the arguments
	Value** arg_list;

	init_thread_locals();
	push_alloc_frame();
	value_local_array( arg_list, 2 );

	// Set the arguments
	arg_list[0]		= keyValue;
	arg_list[1]		= ObjectValueWrapper::intern( value );

	try { check->put_vf( arg_list, 2 ); }
	catch ( AccessorError e )				{ 
		pop_value_local_array( arg_list );
		pop_alloc_frame();
		THROW_PYERROR( e, PyExc_Exception, -1 ); 
	}
	catch ( ArgCountError e )				{ 
		pop_value_local_array( arg_list );
		pop_alloc_frame();
		THROW_PYERROR( e, PyExc_Exception, -1 ); 
	}
	catch ( AssignToConstError e )			{ 
		pop_value_local_array( arg_list );
		pop_alloc_frame();
		THROW_PYERROR( e, PyExc_Exception, -1 ); 
	}
	catch ( CompileError e )				{ 
		pop_value_local_array( arg_list );
		pop_alloc_frame();
		THROW_PYERROR( e, PyExc_Exception, -1 ); 
	}
	catch ( ConversionError e )				{ 
		pop_value_local_array( arg_list );
		pop_alloc_frame();
		THROW_PYERROR( e, PyExc_Exception, -1 ); 
	}
	catch ( DebuggerRuntimeError e )		{ 
		pop_value_local_array( arg_list );
		pop_alloc_frame();
		THROW_PYERROR( e, PyExc_Exception, -1 ); 
	}
	catch ( IncompatibleTypes e )			{ 
		pop_value_local_array( arg_list );
		pop_alloc_frame();
		THROW_PYERROR( e, PyExc_Exception, -1 ); 
	}
	catch ( NoMethodError e	)				{ 
		pop_value_local_array( arg_list );
		pop_alloc_frame();
		THROW_PYERROR( e, PyExc_Exception, -1 ); 
	}
	catch ( RuntimeError e )				{ 
		pop_value_local_array( arg_list );
		pop_alloc_frame();
		THROW_PYERROR( e, PyExc_RuntimeError, -1 ); 
	}
	catch ( SignalException e )				{ 
		pop_value_local_array( arg_list );
		pop_alloc_frame();
		THROW_PYERROR( e, PyExc_Exception, -1 ); 
	}
	catch ( TypeError e )					{ 
		pop_value_local_array( arg_list );
		pop_alloc_frame();
		THROW_PYERROR( e, PyExc_Exception, -1 ); 
	}
	catch ( UnknownSystemException e )		{ 
		pop_value_local_array( arg_list );
		pop_alloc_frame();
		THROW_PYERROR( e, PyExc_SystemError, -1 ); 
	}
	catch ( UserThrownError e )				{ 
		pop_value_local_array( arg_list );
		pop_alloc_frame();
		THROW_PYERROR( e, PyExc_Exception, -1 ); 
	}
	catch ( MAXScriptException e )			{ 
		pop_value_local_array( arg_list );
		pop_alloc_frame();
		THROW_PYERROR( e, PyExc_Exception, -1 ); 
	}
	catch ( ... ) { 
		pop_value_local_array( arg_list );
		pop_alloc_frame();
		PyErr_SetString( PyExc_RuntimeError, "Unknown Error Occured." );
		return -1;
	}
	
	pop_value_local_array( arg_list );
	pop_alloc_frame();
	
	return 0;
}

static int
MXSValueWrapper_setitem( PyObject* self, int index, PyObject* value ) { return MXSValueWrapper_setobjitem( self, PyInt_FromLong( index ), value ); }

static PySequenceMethods proxy_as_sequence = {
	(lenfunc) MXSValueWrapper_length,			// sq_length
	0,											// sq_concat
	0,											// sq_repeat
	(ssizeargfunc) MXSValueWrapper_item,		// sq_item
	0,											// sq_slice
	(ssizeobjargproc) MXSValueWrapper_setitem,	// sq_ass_item
	0,											// sq_ass_slice
	0,											// sq_contains
	0,											// sq_inplace_concat
	0,											// sq_inplace_repeat
};

static PyMappingMethods proxy_as_mapping = {
	(lenfunc) MXSValueWrapper_length,				// mp_length
	(binaryfunc) MXSValueWrapper_objitem,			// mp_subscript
	(objobjargproc) MXSValueWrapper_setobjitem,		// mp_ass_subscript
};

//----------------------------------------------			NUMBER METHODS				------------------------------------------------


// __add__
static PyObject*
MXSValueWrapper_add( PyObject* self, PyObject* other ) {
	Value* vSelf	= ObjectValueWrapper::intern( self );
	Value* vOther	= ObjectValueWrapper::intern( other );

	try								{ return ObjectValueWrapper::pyintern( vSelf->plus_vf( &vOther, 1 ) ); }
	CATCH_ERRORS( NULL );
}

// __sub__
static PyObject*
MXSValueWrapper_subtract( PyObject* self, PyObject* other ) {
	Value* vSelf	= ObjectValueWrapper::intern( self );
	Value* vOther	= ObjectValueWrapper::intern( other );


	try								{ return ObjectValueWrapper::pyintern( vSelf->minus_vf( &vOther, 1 ) ); }
	CATCH_ERRORS( NULL );
}

// __div__
static PyObject*
MXSValueWrapper_divide( PyObject* self, PyObject* other ) {
	Value* vSelf	= ObjectValueWrapper::intern( self );
	Value* vOther	= ObjectValueWrapper::intern( other );

	try	{ return ObjectValueWrapper::pyintern( vSelf->div_vf( &vOther, 1 ) ); }
	CATCH_ERRORS( NULL );
}

// __mul__
static PyObject*
MXSValueWrapper_multiply( PyObject* self, PyObject* other ) {
	Value* vSelf	= ObjectValueWrapper::intern( self );
	Value* vOther	= ObjectValueWrapper::intern( other );

	try	{ return ObjectValueWrapper::pyintern( vSelf->times_vf( &vOther, 1 ) ); }
	CATCH_ERRORS( NULL );
}

// __pow__
static PyObject*
MXSValueWrapper_power( PyObject* self, PyObject* other, PyObject* args ) {
	Value* vSelf	= ObjectValueWrapper::intern( self );
	Value* vOther	= ObjectValueWrapper::intern( other );

	try	{ return ObjectValueWrapper::pyintern( vSelf->pwr_vf( &vOther, 1 ) ); }
	CATCH_ERRORS( NULL );
}

// __abs__
static PyObject*
MXSValueWrapper_absolute( PyObject* self ) {
	Value* vSelf	= ObjectValueWrapper::intern( self );
	try	{ return ObjectValueWrapper::pyintern( vSelf->abs_vf( NULL, 0 ) ); }
	CATCH_ERRORS( NULL );
}

// __int__
static PyObject*
MXSValueWrapper_int( PyObject* self ) {
	Value* vSelf	= ObjectValueWrapper::intern( self );
	try	{ return PyInt_FromLong( vSelf->to_int() ); }
	CATCH_ERRORS( NULL );
}

// __float__
static PyObject*
MXSValueWrapper_float( PyObject* self ) {
	Value* vSelf	= ObjectValueWrapper::intern( self );
	try	{ return PyFloat_FromDouble( vSelf->to_float() ); }
	CATCH_ERRORS( NULL );
}

// __neg__
static PyObject*
MXSValueWrapper_negative( PyObject* self ) { return MXSValueWrapper_multiply( self, PyInt_FromLong( -1 ) ); }

// __nonzero__
static bool
MXSValueWrapper_nonzero( PyObject* self ) { return true; }

static PyNumberMethods proxy_as_number = {
	(binaryfunc) MXSValueWrapper_add,			// nb_add
	(binaryfunc) MXSValueWrapper_subtract,		// nb_subtract
	(binaryfunc) MXSValueWrapper_multiply,		// nb_multiply
	(binaryfunc) MXSValueWrapper_divide,		// nb_divide
	0,											// nb_remainder
	0,											// nb_divmod
	(ternaryfunc) MXSValueWrapper_power,		// nb_power
	(unaryfunc)	MXSValueWrapper_negative,		// nb_negative	
	0,											// nb_positive
	(unaryfunc) MXSValueWrapper_absolute,		// nb_absolute
	(inquiry) MXSValueWrapper_nonzero,			// nb_nonzero
	0,											// nb_invert
	0,											// nb_lshift
	0,											// nb_rshift
	0,											// nb_and
	0,											// nb_xor
	0,											// nb_or
	0,											// nb_coerce
	(unaryfunc)	MXSValueWrapper_int,			// nb_int
	0,											// nb_long
	(unaryfunc) MXSValueWrapper_float,			// nb_float
	0,											// nb_oct
	0,											// nb_hex
	0,											// nb_inplace_add
	0,											// nb_inplace_subtract
    0,											// nb_inplace_multiply
	0,											// nb_inplace_divide
	0,											// nb_inplace_remainder
	0,											// nb_inplace_power
	0,											// nb_inplace_lshift
	0,											// nb_inplace_rshift
	0,											// nb_inplace_and
	0,											// nb_inplace_xor
	0,											// nb_inplace_or
};

static PyTypeObject MXSValueWrapperType = {
    PyObject_HEAD_INIT(NULL)
    0,																	// ob_size
    "mxs",																// tp_name
    sizeof(MXSValueWrapper),											// tp_basicsize
    0,																	// tp_itemsize
    (destructor)MXSValueWrapper_dealloc,								// tp_dealloc
    0,																	// tp_print
    (getattrfunc)MXSValueWrapper_getattr,								// tp_getattr
    (setattrfunc)MXSValueWrapper_setattr,								// tp_setattr
    (cmpfunc)MXSValueWrapper_compare,									// tp_compare
    0,																	// tp_repr
    &proxy_as_number,													// tp_as_number
    &proxy_as_sequence,													// tp_as_sequence
    &proxy_as_mapping,													// tp_as_mapping
    0,																	// tp_hash 
    (ternaryfunc)MXSValueWrapper_call,									// tp_call
    (reprfunc)MXSValueWrapper_str,										// tp_str
    0,																	// tp_getattro
    0,																	// tp_setattro
    0,																	// tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_CHECKTYPES,	// tp_flags
    "Maxscript Value Wrapper",											// tp_doc 
    0,																	// tp_traverse 
    0,																	// tp_clear 
    0,																	// tp_richcompare 
    0,																	// tp_weaklistoffset 
    0,																	// tp_iter 
    0,																	// tp_iternext 
    0,																	// tp_methods 
    0,																	// tp_members 
    0,																	// tp_getset 
    0,																	// tp_base 
    0,																	// tp_dict 
    0,																	// tp_descr_get 
    0,																	// tp_descr_set 
    0,																	// tp_dictoffset 
    0,																	// tp_init 
    0,																	// tp_alloc 
    MXSValueWrapper_new,												// tp_new 
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
bool		ObjectValueWrapper::isWrapper(			PyObject* item ) { return item->ob_type == &MXSValueWrapperType; }
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
			wrapper->value				= item->make_heap_permanent();
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