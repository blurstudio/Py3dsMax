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

// Use if compiling for Python24 mappings, comment out for Python25+
//typedef inquiry			lenfunc;
//typedef int				Py_ssize_t;
//typedef intobjargproc	ssizeobjargproc;
//typedef	intargfunc		ssizeargfunc;

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
	//----------------------------------------------
	// Calculate the number of arguments and keywords to determine how large to allocate the test
	
	int arg_count		= (args)					? PyTuple_Size( args )	: 0;
	int keyword_count	= (kwds)					? PyDict_Size( kwds )	: -1;
	int mxs_count		= ( keyword_count == -1 )	? arg_count				: arg_count + 1 + (keyword_count*2);

	PyObject* output	= NULL;

	// Protect the memory
	MXS_PROTECT( three_value_locals( result, method, errlog ) );

	try { 
		// Evaluate out of thunks the method
		vl.method = self->value;
		while ( vl.method != NULL && is_thunk( vl.method ) )
			vl.method = vl.method->eval();
		
		// Apply the method
		if ( vl.method != NULL ) {
			// Check to see if there are arguments/keys to supply
			if ( mxs_count ) {
				// Create the maxscript arguments
				Value** arg_list;
				value_local_array(arg_list,mxs_count);

				// Add arguments
				for ( int i = 0; i < arg_count; i++ )
					arg_list[i] = ObjectValueWrapper::intern( PyTuple_GetItem( args, i ) );

				// Add keywords
				if ( keyword_count != -1 ) {
					PyObject *key, *py_value;
					Py_ssize_t pos			= 0;
					int key_pos				= 0;

					arg_list[arg_count]	= &keyarg_marker;

					while ( PyDict_Next(kwds,&pos,&key,&py_value) ) {
						arg_list[ arg_count + 1 + (key_pos*2) ] = Name::intern( PyString_AsString( key ) );
						arg_list[ arg_count + 2 + (key_pos*2) ] = ObjectValueWrapper::intern( py_value );
						key_pos++;
					}
				}

				vl.result = vl.method->apply( arg_list, mxs_count );

				pop_value_local_array(arg_list);
			}
			else {
				vl.result = vl.method->apply( NULL, 0 );
			}
		}

		// Convert the result
		if ( vl.result != NULL )
			output = ObjectValueWrapper::pyintern( vl.result );
		
		// Convert the output to None if it is currently NULL without failure
		if ( output == NULL ) {
			Py_INCREF(Py_None);
			output = Py_None;
		}
	}
	CATCH_ERRORS();

	MXS_CLEANUP();

	return output;
}

// __cmp__
static int
MXSValueWrapper_compare( PyObject* self, PyObject* other ) {
	int result = -1;

	MXS_PROTECT( three_value_locals( mCheck, oCheck, errlog ) );

	vl.mCheck	= ((MXSValueWrapper*) self)->value;
	vl.oCheck	= ObjectValueWrapper::intern( other );

	// Check the inputed value against the current value
	if ( vl.oCheck ) {
		// Check direct pointers
		if ( vl.mCheck == vl.oCheck || vl.mCheck->eval() == vl.oCheck->eval() )		{ result = 0; }
		else {
			// Check MAXScript __lt__, __eq__, __gt__
			try { 
				if		( vl.mCheck->eq_vf( &vl.oCheck, 1 ) == &true_value )	{ result = 0; }
				else if	( vl.mCheck->lt_vf( &vl.oCheck, 1 ) == &true_value )	{ result = -1; }
				else if ( vl.mCheck->gt_vf( &vl.oCheck, 1 ) == &true_value )	{ result = 1; }
			}
			catch ( ... ) {
				MXS_CLEARERRORS();
			}
		}
	}

	MXS_CLEANUP();

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
	MXS_PROTECT( four_value_locals( result, keyName, check, errlog ) );
	vl.keyName	= Name::intern(key);
	vl.check	= self->value;
	
	while ( vl.check != NULL && is_thunk(vl.check) )
		vl.check = vl.check->eval();
	
	// Map generic items
	if ( vl.check ) {
		try	{ vl.result = vl.check->_get_property( vl.keyName ); }
		CATCH_ERRORS();
	}

	PyObject* output = NULL;

	if ( !vl.result )	{ PyErr_SetString( PyExc_AttributeError, key ); }
	else				{ output = ObjectValueWrapper::pyintern( vl.result ); }

	MXS_CLEANUP();

	return output;
}

// __setattr__
static int
MXSValueWrapper_setattr( MXSValueWrapper* self, char* key, PyObject* value ) {
	MXS_PROTECT( five_value_locals( result, check, keyName, maxValue, errlog ) );

	// Convert the value to an evaluated version of the thunk
	MXS_EVAL( vl.check );
	vl.keyName	= Name::intern(key);
	vl.maxValue	= ObjectValueWrapper::intern( value );
	
	try {
		// Set Controllers
		if ( vl.keyName == Name::intern( "controller" ) && is_subAnim( vl.check ) ) {
			((MAXSubAnim*) vl.check)->set_max_controller( (MAXControl*) vl.maxValue );
		}
		else {
			vl.result = vl.check->_set_property( vl.keyName, vl.maxValue );
		}
	}
	CATCH_ERRORS();

	int output = 0;

	if ( vl.result == NULL ) {
		PyErr_SetString( PyExc_AttributeError, key );
		output = NULL;
	}

	MXS_CLEANUP();

	return output;
}

// __str__
static PyObject*
MXSValueWrapper_str( MXSValueWrapper* self ) {
	// Convert name values to strings
	if ( is_name( self->value ) )
		return PyString_FromString( self->value->to_string() );

	else {
		MXS_PROTECT( three_value_locals( check, stream, errlog ) );

		MXS_EVAL(vl.check);
		vl.stream = new StringStream(0);

		PyObject* output = NULL;

		try { 
			vl.check->sprin1((StringStream *) vl.stream); 
			output = PyString_FromString( vl.stream->to_string() );
		}
		CATCH_ERRORS();

		MXS_CLEANUP();

		return output;
	}
}

//----------------------------------------------			SEQUENCE METHODS				------------------------------------------------

// __len__
static int
MXSValueWrapper_length( PyObject* self )		{
	MXS_PROTECT( two_value_locals( check, errlog ) );
	MXS_EVAL( vl.check );

	int output = -1;

	try {
		// Calculate Array size
		if ( is_array( vl.check ) )	{ output = (int) ((Array*) vl.check)->size; }

		// Calculate Collection size
		else { output = vl.check->_get_property( Name::intern( "count" ) )->to_int(); }
	}
	CATCH_ERRORS();
	
	if ( output == -1 ) {
		PyErr_SetString( PyExc_TypeError, TSTR( "MXSValueWrapper_item: Cannot convert to <list>: " ) + PyString_AsString( MXSValueWrapper_str( (MXSValueWrapper*) self )) );
	}

	MXS_CLEANUP();

	return output;
}

// __getitem__
static PyObject*
MXSValueWrapper_objitem( PyObject* self, PyObject* key ) {
	MXS_PROTECT( three_value_locals( check, maxIndex, errlog ) );
	MXS_EVAL( vl.check );

	PyObject* output;

	try {
		// Grab an item by index
		if ( key->ob_type == &PyInt_Type ) {
			// Grab the collection's count
			int count;
			int index = (int) PyInt_AsLong( key );
			
			count = vl.check->_get_property( Name::intern( "count" ) )->to_int();

			if ( 0 <= index && index < count ) {
				vl.maxIndex		= Integer::intern( index + 1 );
				output			= ObjectValueWrapper::pyintern( vl.check->get_vf( &vl.maxIndex, 1 ) );
			}
			else {
				PyErr_SetString( PyExc_IndexError, TSTR( "__getitem__ error: index is out of range" ) );
			}
		}

		// Grab an item by some other mapping
		else {
			vl.maxIndex = ObjectValueWrapper::intern( key );
			output		= ObjectValueWrapper::pyintern( vl.check->get_vf( &vl.maxIndex, 1 ) );
		}
	}
	CATCH_ERRORS();

	MXS_CLEANUP();

	return output;
}
static PyObject*
MXSValueWrapper_item( PyObject* self, int index ) { return MXSValueWrapper_objitem( self, PyInt_FromLong( index ) ); }

// __setitem__
static int
MXSValueWrapper_setobjitem( PyObject* self, PyObject* key, PyObject* value ) {
	MXS_PROTECT( three_value_locals(check,keyValue,errlog) );

	vl.check = ((MXSValueWrapper*) self)->value;
	while ( vl.check != NULL && is_thunk(vl.check) )
		vl.check = vl.check->eval();

	vl.keyValue;

	// Convert to index
	if ( key->ob_type == &PyInt_Type ) {
		int index		= (int) PyInt_AsLong( key );
		int count		= MXSValueWrapper_length( self );
		if ( 0 <= index && index < count )
			vl.keyValue	= Integer::intern( index );
		else {
			PyErr_SetString( PyExc_IndexError, "__setitem__ error: index is out of range" );
			return -1;
		}
	}
	// Keep as key
	else vl.keyValue	= ObjectValueWrapper::intern( key );

	// Build the arguments
	Value** arg_list;
	value_local_array( arg_list, 2 );

	// Set the arguments
	arg_list[0]		= vl.keyValue;
	arg_list[1]		= ObjectValueWrapper::intern( value );

	try { vl.check->put_vf( arg_list, 2 ); }
	CATCH_ERRORS();
	
	pop_value_local_array( arg_list );
	MXS_CLEANUP();
	
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
	MXS_PROTECT( three_value_locals( vSelf, vOther, errlog ) );
	vl.vSelf		= ObjectValueWrapper::intern( self );
	vl.vOther		= ObjectValueWrapper::intern( other );

	PyObject* output = NULL;
	try { output = ObjectValueWrapper::pyintern( vl.vSelf->plus_vf( &vl.vOther, 1 ) ); }
	CATCH_ERRORS();

	MXS_CLEANUP();

	return output;
}

// __sub__
static PyObject*
MXSValueWrapper_subtract( PyObject* self, PyObject* other ) {
	MXS_PROTECT( three_value_locals( vSelf, vOther, errlog ) );
	vl.vSelf		= ObjectValueWrapper::intern( self );
	vl.vOther		= ObjectValueWrapper::intern( other );

	PyObject* output = NULL;
	try { output = ObjectValueWrapper::pyintern( vl.vSelf->minus_vf( &vl.vOther, 1 ) ); }
	CATCH_ERRORS();

	MXS_CLEANUP();

	return output;
}

// __div__
static PyObject*
MXSValueWrapper_divide( PyObject* self, PyObject* other ) {
	MXS_PROTECT( three_value_locals( vSelf, vOther, errlog ) );
	vl.vSelf		= ObjectValueWrapper::intern( self );
	vl.vOther		= ObjectValueWrapper::intern( other );

	PyObject* output = NULL;
	try { output = ObjectValueWrapper::pyintern( vl.vSelf->div_vf( &vl.vOther, 1 ) ); }
	CATCH_ERRORS();

	MXS_CLEANUP();

	return output;
}

// __mul__
static PyObject*
MXSValueWrapper_multiply( PyObject* self, PyObject* other ) {
	MXS_PROTECT( three_value_locals( vSelf, vOther, errlog ) );
	vl.vSelf		= ObjectValueWrapper::intern( self );
	vl.vOther		= ObjectValueWrapper::intern( other );

	PyObject* output = NULL;
	try { output = ObjectValueWrapper::pyintern( vl.vSelf->times_vf( &vl.vOther, 1 ) ); }
	CATCH_ERRORS();

	MXS_CLEANUP();

	return output;
}

// __pow__
static PyObject*
MXSValueWrapper_power( PyObject* self, PyObject* other, PyObject* args ) {
	MXS_PROTECT( three_value_locals( vSelf, vOther, errlog ) );
	vl.vSelf		= ObjectValueWrapper::intern( self );
	vl.vOther		= ObjectValueWrapper::intern( other );

	PyObject* output = NULL;
	try { output = ObjectValueWrapper::pyintern( vl.vSelf->pwr_vf( &vl.vOther, 1 ) ); }
	CATCH_ERRORS();

	MXS_CLEANUP();

	return output;
}

// __abs__
static PyObject*
MXSValueWrapper_absolute( PyObject* self ) {
	MXS_PROTECT( two_value_locals( vSelf, errlog ) );
	vl.vSelf		= ObjectValueWrapper::intern( self );
	PyObject* output = NULL;
	try { output = ObjectValueWrapper::pyintern( vl.vSelf->abs_vf( NULL, 0 ) ); }
	CATCH_ERRORS();
	MXS_CLEANUP();
	return output;
}

// __int__
static PyObject*
MXSValueWrapper_int( PyObject* self ) {
	MXS_PROTECT( two_value_locals( vSelf, errlog ) );
	vl.vSelf = ObjectValueWrapper::intern( self );
	PyObject* output = NULL;
	try	{ output = PyInt_FromLong( vl.vSelf->to_int() ); }
	CATCH_ERRORS();
	MXS_CLEANUP();
	return output;
}

// __float__
static PyObject*
MXSValueWrapper_float( PyObject* self ) {
	MXS_PROTECT( two_value_locals( vSelf, errlog ) );
	vl.vSelf = ObjectValueWrapper::intern( self );
	PyObject* output = NULL;
	try	{ output = PyFloat_FromDouble( vl.vSelf->to_float() ); }
	CATCH_ERRORS();
	MXS_CLEANUP();
	return output;
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
		else if	( is_string( eval_item ) )									{ return PyString_FromString( eval_item->to_string() ); }
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
			wrapper->value				= item->make_heap_static();
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