/*
	\file		wrapper.cpp

	\remarks	Implementation of the ValueWrapper and ObjectWrapper classes

	\author		Blur Studio (c) 2010
	\email		beta@blur.com

	\license	This software is released under the GNU General Public License.  For more info, visit: http://www.gnu.org/
*/

#include "imports.h"

#include <string>

#include "macros.h"
#include "wrapper.h"
#include "protector.h"

#include "Parser.h"  // for print functions

// define these properties if the MAKE_PY26_COMPAT preprocessor definition exists
#ifdef MAKE_PY26_COMPAT

typedef inquiry			lenfunc;
typedef int				Py_ssize_t;
typedef intobjargproc	ssizeobjargproc;
typedef	intargfunc		ssizeargfunc;

#endif

//---------------------------------------------------------------
// ValueWrapper Implementation
//---------------------------------------------------------------

// ctor
static PyObject*
ValueWrapper_new( PyTypeObject* type, PyObject* args, PyObject* kwds ) {
	ValueWrapper* self;
	self = (ValueWrapper*)type->tp_alloc(type, 0);

	self->mValue		= NULL;
	self->mNext = self->mPrev = 0;
	return (PyObject*) self;
}

// dtor
static void
ValueWrapper_dealloc( ValueWrapper* self ) {

	Protector::unprotect( self );

	self->ob_type->tp_free( (PyObject*) self );
}

// __call__ function: called when calling a function on a ValueWrapper instance
static PyObject*
ValueWrapper_call( ValueWrapper* self, PyObject* args, PyObject* kwds ) {
	// Step 1: calculate how many Value**'s we are going to need to pass the Value* method
	int arg_count		= (args) ? PyTuple_Size( args ) : 0;
	int key_count		= (kwds) ? PyDict_Size( kwds )	: -1;
	int mxs_count		= (key_count == -1) ? arg_count : arg_count + 1 + (key_count*2);

	// Step 2: protect the maxcript memory we are going to use
	MXS_PROTECT( three_typed_value_locals( Value* method, Value* result, StringStream* log ) );

	// Step 3: pull out the proper method from maxscript
	MXS_EVAL( self->mValue, vl.method );

	// override the current log for error catching purposes
	vl.log	= new StringStream();
	CharStream* old_log = thread_local(current_stdout);
	((MAXScript_TLS*)TlsGetValue(thread_locals_index))->current_stdout = vl.log;

	vl.result = NULL;

	// Step 4: call the maxscript method
	if ( vl.method ) {
		// Step 5: check to see if we need maxscript arguments
		if ( mxs_count ) {
			// Step 6: if we do, we need to create a volatile array and protect it
			Value** mxs_args;
			value_local_array(mxs_args,mxs_count);

			// Step 7: add converted arguments
			for ( int i = 0; i < arg_count; i++ ) {
				mxs_args[i] = ObjectWrapper::intern( PyTuple_GetItem( args, i ) );
			}

			// Step 8: add converted keywords
			if ( key_count != -1 ) {
				PyObject *pkey, *pvalue;
				Py_ssize_t pos	= 0;
				int key_pos		= 0;

				// Maxscript deliminates keywords from arguments by supplying the &keyarg_marker pointer
				// for when the keywords start, and from there does a key, value pairing in the array
				mxs_args[arg_count]	= &keyarg_marker;

				while ( PyDict_Next(kwds, &pos, &pkey, &pvalue ) ) {
					mxs_args[ arg_count + 1 + (key_pos*2) ] = Name::intern( PyString_AsString( pkey ) );
					mxs_args[ arg_count + 2 + (key_pos*2) ] = ObjectWrapper::intern( pvalue );
					key_pos++;
				}

				// Release the key and value pointers
				pkey	= NULL;
				pvalue	= NULL;
			}

			// Step 9: call the method and use try/catch to protect the memory
			try { vl.result = vl.method->apply( mxs_args, mxs_count ); }
			catch ( ... ) {
				MXS_CLEARERRORS();
				PyErr_SetString( PyExc_RuntimeError, vl.log->to_string() );
				vl.result = NULL;
			}

			// clear the value local memory
			pop_value_local_array(mxs_args);
		}
		else {
			// Step 6: if we don't, simply call the method with a NULL array
			try { vl.result = vl.method->apply( NULL, 0 ); }
			catch ( ... ) {
				MXS_CLEARERRORS();
				PyErr_SetString( PyExc_RuntimeError, vl.log->to_string() );
				vl.result = NULL;
			}
		}
	}

	// restore the old log
	((MAXScript_TLS*)TlsGetValue(thread_locals_index))->current_stdout = old_log;

	// Step 10: convert the result
	PyObject* output = NULL;
	if ( vl.result )
		output = ObjectWrapper::py_intern( vl.result );
	else if( !PyErr_Occurred() ) {
		output = Py_None;
		Py_INCREF(Py_None);
	}
	
	// Step 11: cleanup the maxscript errors
	MXS_CLEANUP();

	return output;
}

// __cmp__ function: called when comparing 2 ValueWrapper instances (a == b, a < b, a <= b, etc.)
static int
ValueWrapper_compare( PyObject* self, PyObject* other ) {
	int result = -1;

	// Step 1: protect the maxscript memory
	MXS_PROTECT( two_value_locals( mxs_self, mxs_other ) );

	// Step 2: convert the two items to values
	MXS_EVAL( ObjectWrapper::intern( self ), vl.mxs_self );
	MXS_EVAL( ObjectWrapper::intern( other ), vl.mxs_other );

	// Step 3: make sure both items exist
	if ( vl.mxs_self && vl.mxs_other ) {
		// Step 4: compare direct pointers
		if ( vl.mxs_self == vl.mxs_other || vl.mxs_self->eval() == vl.mxs_other->eval() )
			result = 0;
		else {
			// Step 5: use maxscript's built in <, ==, > checks
			try {
				if		( vl.mxs_self->eq_vf( &vl.mxs_other, 1 ) == &true_value ) { result = 0; }
				else if ( vl.mxs_self->lt_vf( &vl.mxs_other, 1 ) == &true_value ) { result = -1; }
				else if ( vl.mxs_self->gt_vf( &vl.mxs_other, 1 ) == &true_value ) { result = 1; }
			}
			catch ( ... ) {
				MXS_CLEARERRORS();
			}
		}
	}

	// Step 6: cleanup maxscript memory
	MXS_CLEANUP();

	return result;
}

// __str__ function: convert a Value* wrapper to a string
static PyObject*
ValueWrapper_str( ValueWrapper* self ) {
	// Step 1: protect values
	MXS_PROTECT( two_typed_value_locals( Value* mxs_check, StringStream* mxs_stream ) );

	// Step 2: evaluate the value
	MXS_EVAL( self->mValue, vl.mxs_check );

	// Step 3: check for simple conversions
	PyObject* output = NULL;
	if ( is_name( vl.mxs_check ) || is_string( vl.mxs_check ) ) 
		output = PyString_FromString( vl.mxs_check->to_string() );
	
	// Step 4: block recursive struct printouts
	else if ( is_struct( vl.mxs_check ) )
		output = PyString_FromString( "<mxs.StructDef instance>" );

	// Step 5: try to use the builtin maxscript print out
	else {
		vl.mxs_stream = new StringStream();
		try {
			vl.mxs_check->sprin1( vl.mxs_stream );
			output = PyString_FromString( vl.mxs_stream->to_string() );
		}
		MXS_CATCHERRORS();
	}

	// Step 6: cleanup maxscript memory
	MXS_CLEANUP();
	if( !output ) {
		output = Py_None;
		Py_INCREF(output);
	}
	
	return output;
}

// __getattr__ function: get a property by name from a Value* instance
static PyObject*
ValueWrapper_getattr( ValueWrapper* self, char* key ) {
	// check for pre-defined python properties
	PyObject* tmp;
	PyObject* check = PyString_FromString(key);
	if ( !(tmp = PyObject_GenericGetAttr( (PyObject*) self, check )) ) {
		Py_XDECREF(check);

		// check for python internals
		if ( key[0] == '_' || !PyErr_ExceptionMatches(PyExc_AttributeError) ) {
			return NULL;
		}
		PyErr_Clear();
	}
	else {
		Py_XDECREF(check);
		return tmp;
	}

	// return the default maxscript value
	one_value_local(result);
	try { vl.result = ((ValueWrapper*) self)->mValue->eval()->_get_property( Name::intern(key) ); }
	catch ( ... ) {
		PyObject * self_str = ValueWrapper_str(self);
		PyErr_Format( PyExc_AttributeError, "%s is not a property of %s", key, PyString_AsString(self_str) );
		Py_DECREF(self_str);
		return 0;
	}

	return ObjectWrapper::py_intern( vl.result );
}

// __setattr__ function: sets a property by name for a Value*
static int
ValueWrapper_setattr( ValueWrapper* self, char* key, PyObject* value ) {
	bool success = true;
	try { self->mValue->eval()->_set_property( Name::intern(key), ObjectWrapper::intern(value) ); }
	catch ( ... ) { success = false; }
	
	if ( success ) {
		return 0;
	}
	else {
		PyObject * self_str = ValueWrapper_str(self);
		PyErr_Format( PyExc_AttributeError, "%s is not a member of %s", key, PyString_AsString(self_str) );
		Py_DECREF(self_str);
		return 1;
	}
}

//------- Sequence Methods

// __len__ function: calculate the length of a maxscript value
static int
ValueWrapper_length( PyObject* self ) {
	// Step 1: protect the maxscript memory
	MXS_PROTECT( one_value_local( mxs_check ) );

	// Step 2: evaluate the value
	MXS_EVAL( ((ValueWrapper*) (self))->mValue, vl.mxs_check );

	int count = NULL;

	// Step 3: use the count property to check if an object has length in maxscript
	try { count = vl.mxs_check->_get_property( n_count )->to_int(); }
	MXS_CATCHERRORS();

	// Step 4: cleanup maxscript memory
	MXS_CLEANUP();

	return count;
}

// __getitem__ function: get an item from an index in a maxscript value
static PyObject*
ValueWrapper_item( PyObject* self, int index ) {
	MXS_PROTECT( three_value_locals( mxs_check, mindex, result ) );
	MXS_EVAL( ((ValueWrapper*) self)->mValue, vl.mxs_check );

	int count = 0;

	// pull the lenght of the collection
	try { count = vl.mxs_check->_get_property( n_count )->to_int(); }
	catch ( ... ) { 
		MXS_CLEARERRORS();
		PyErr_SetString( PyExc_IndexError, "__getitem__ cannot access item for index" ); 
		MXS_CLEANUP();
		return NULL;
	}

	// support pythonic negative lookup
	if ( index < 0 ) {
		index = count + index;
	}

	// make sure the index is valid
	if ( !( 0 <= index && index < count) ) {
		PyErr_SetString( PyExc_IndexError, "__getitem__ index is out of range" ); 
		MXS_CLEANUP();
		return NULL;
	}

	// pull the value
	vl.mindex = Integer::intern( index + 1 );

	try { vl.result = vl.mxs_check->get_vf( &vl.mindex, 1 ); }
	MXS_CATCHERRORS();

	PyObject* output = NULL;
	if ( !vl.result ) { PyErr_SetString( PyExc_IndexError, "__getitem__ index is out of range" ); }
	else { output = ObjectWrapper::py_intern(vl.result); }

	MXS_CLEANUP();

	return output;
}

// __getitem__ function: get an item based on an abstract python object
static PyObject*
ValueWrapper_objitem( PyObject* self, PyObject* key ) {
	// Step 1: check to see if the key is a number, then use the above method
	if ( key->ob_type == &PyInt_Type ) {
		return ValueWrapper_item( self, PyInt_AsLong( key ) );
	}

	// Step 2: protect the maxscript memory
	MXS_PROTECT( three_value_locals( mxs_index, mxs_check, mxs_result ) );

	// Step 3: evaluate the value
	MXS_EVAL( ((ValueWrapper*) (self))->mValue, vl.mxs_check );

	// Step 4: convert the python object to a maxscript key
	vl.mxs_index = ObjectWrapper::intern( key );

	// Step 5: try to pull the value from the sequence using virtual functions
	try { vl.mxs_result = vl.mxs_check->get_vf( &vl.mxs_index, 1 ); }
	MXS_CATCHERRORS();

	// Step 6: convert the maxscript result to a python value
	PyObject* output = NULL;
	if ( vl.mxs_result ) { output = ObjectWrapper::py_intern( vl.mxs_result ); }
	else { PyErr_SetString( PyExc_IndexError, "__getitem__ could not get a maxscript value" ); }

	// Step 7: cleanup maxscript memory
	MXS_CLEANUP();

	return output;
}

// __setitem__ function: set a maxscript value by an index
static int
ValueWrapper_setitem( PyObject* self, int index, PyObject* value ) {
	// Step 1: protect maxscript memory
	MXS_PROTECT( two_value_locals( mxs_check, mxs_result ) );

	// Step 2: evaluate the value
	MXS_EVAL( ((ValueWrapper*) (self))->mValue, vl.mxs_check );

	// Step 3: calculate the length of the sequence
	int count = 0;
	try { count = vl.mxs_check->_get_property( n_count )->to_int(); }
	MXS_CATCHERRORS();

	if ( count ) {
		// Step 3: resolve index differences in maxscript & python
		if ( index < 0 )
			index += count;

		if ( 0 <= index && index < count ) {
			// Step 4: create a volatile array of values to pass to the put virtual function
			Value** arg_list;
			value_local_array( arg_list, 2 );
			arg_list[0] = Integer::intern( index + 1 );
			arg_list[1] = ObjectWrapper::intern( value );

			try { vl.mxs_result = vl.mxs_check->put_vf( arg_list, 2 ); }
			MXS_CATCHERRORS();

			// Step 5: pop the local array
			pop_value_local_array(arg_list);
		}
	}

	// Step 6: convert the result
	int result = -1;
	if ( vl.mxs_result ) { result = 0; }
	else { PyErr_SetString( PyExc_IndexError, "__setitem__ index is out of range" ); }

	// Step 6: cleanup maxscript memory
	MXS_CLEANUP();

	return result;
}

// __setitem__ function: set a maxscript value by python object
static int
ValueWrapper_setobjitem( PyObject* self, PyObject* key, PyObject* value ) {
	// Step 1: check to see if the python object is a number type, if so, use the above method
	if ( key->ob_type == &PyInt_Type ) {
		return ValueWrapper_setitem( self, PyInt_AsLong( key ), value );
	}

	// Step 2: protect the maxscript memory
	MXS_PROTECT( one_value_local( mxs_check ) );

	// Step 3: evaluate the value
	MXS_EVAL( ((ValueWrapper*) (self))->mValue, vl.mxs_check );

	// Step 4: create a volatile maxscript value array to pass to the put virtual function
	Value** arg_list;
	value_local_array( arg_list, 2 );
	arg_list[0] = ObjectWrapper::intern( key );
	arg_list[1] = ObjectWrapper::intern( value );

	// Step 5: call the put virtual method
	int result = -1;
	try { 
		vl.mxs_check->put_vf( arg_list, 2 );
		result = 0;
	}
	MXS_CATCHERRORS();

	// Step 6: convert the results
	if ( result == -1 ) { PyErr_SetString( PyExc_IndexError, "__setitem__ could not set the maxscript value" ); }

	// Step 7: cleanup maxscript memory
	pop_value_local_array( arg_list );
	MXS_CLEANUP();

	return result;
}

// Number Methods

// __add__ function: called when trying to add two wrapper values together
static PyObject*
ValueWrapper_add( PyObject* self, PyObject* other ) {
	// Step 1: protect maxscript memory
	MXS_PROTECT( three_value_locals( mxs_self, mxs_other, mxs_value ) );
	
	// Step 2: convert the two items to values
	MXS_EVAL( ObjectWrapper::intern( self ), vl.mxs_self );
	MXS_EVAL( ObjectWrapper::intern( other ), vl.mxs_other );

	// Step 3: calculate maxscript value
	try { vl.mxs_value = vl.mxs_self->plus_vf( &vl.mxs_other, 1 ); }
	MXS_CATCHERRORS();

	// Step 4: convert to python
	PyObject* output = NULL;
	if ( vl.mxs_value ) { output = ObjectWrapper::py_intern( vl.mxs_value ); }
	else { PyErr_SetString( PyExc_ArithmeticError, "__add__ error: could not add maxscript values together" ); }

	// Step 5: cleanup maxscipt memory
	MXS_CLEANUP();

	return output;
}

// __sub__ function: called when trying to subtract two wrapper values together
static PyObject*
ValueWrapper_subtract( PyObject* self, PyObject* other ) {
	// Step 1: protect maxscript memory
	MXS_PROTECT( three_value_locals( mxs_self, mxs_other, mxs_value ) );
	
	// Step 2: convert the two items to values
	MXS_EVAL( ObjectWrapper::intern( self ), vl.mxs_self );
	MXS_EVAL( ObjectWrapper::intern( other ), vl.mxs_other );

	// Step 3: calculate maxscript value
	try { vl.mxs_value = vl.mxs_self->minus_vf( &vl.mxs_other, 1 ); }
	MXS_CATCHERRORS();

	// Step 4: convert to python
	PyObject* output = NULL;
	if ( vl.mxs_value ) { output = ObjectWrapper::py_intern( vl.mxs_value ); }
	else { PyErr_SetString( PyExc_ArithmeticError, "__sub__ error: could not add maxscript values together" ); }

	// Step 5: cleanup maxscipt memory
	MXS_CLEANUP();

	return output;
}

// __div__ function: called when trying to divide two wrapper values together
static PyObject*
ValueWrapper_divide( PyObject* self, PyObject* other ) {
	// Step 1: protect maxscript memory
	MXS_PROTECT( three_value_locals( mxs_self, mxs_other, mxs_value ) );
	
	// Step 2: convert the two items to values
	MXS_EVAL( ObjectWrapper::intern( self ), vl.mxs_self );
	MXS_EVAL( ObjectWrapper::intern( other ), vl.mxs_other );

	// Step 3: calculate maxscript value
	try { vl.mxs_value = vl.mxs_self->div_vf( &vl.mxs_other, 1 ); }
	MXS_CATCHERRORS();

	// Step 4: convert to python
	PyObject* output = NULL;
	if ( vl.mxs_value ) { output = ObjectWrapper::py_intern( vl.mxs_value ); }
	else { PyErr_SetString( PyExc_ArithmeticError, "__div__ error: could not add maxscript values together" ); }

	// Step 5: cleanup maxscipt memory
	MXS_CLEANUP();

	return output;
}

// __mul__ function: called when trying to multiply two wrapper values together
static PyObject*
ValueWrapper_multiply( PyObject* self, PyObject* other ) {
	// Step 1: protect maxscript memory
	MXS_PROTECT( three_value_locals( mxs_self, mxs_other, mxs_value ) );
	
	// Step 2: convert the two items to values
	MXS_EVAL( ObjectWrapper::intern( self ), vl.mxs_self );
	MXS_EVAL( ObjectWrapper::intern( other ), vl.mxs_other );

	// Step 3: calculate maxscript value
	try { vl.mxs_value = vl.mxs_self->times_vf( &vl.mxs_other, 1 ); }
	MXS_CATCHERRORS();

	// Step 4: convert to python
	PyObject* output = NULL;
	if ( vl.mxs_value ) { output = ObjectWrapper::py_intern( vl.mxs_value ); }
	else { PyErr_SetString( PyExc_ArithmeticError, "__mul__ error: could not add maxscript values together" ); }

	// Step 5: cleanup maxscipt memory
	MXS_CLEANUP();

	return output;
}

// __pow__ function: called when trying to raise one number by the other
static PyObject*
ValueWrapper_power( PyObject* self, PyObject* other, PyObject *args ) {
	// Step 1: protect maxscript memory
	MXS_PROTECT( three_value_locals( mxs_self, mxs_other, mxs_value ) );
	
	// Step 2: convert the two items to values
	MXS_EVAL( ObjectWrapper::intern( self ), vl.mxs_self );
	MXS_EVAL( ObjectWrapper::intern( other ), vl.mxs_other );

	// Step 3: calculate maxscript value
	try { vl.mxs_value = vl.mxs_self->pwr_vf( &vl.mxs_other, 1 ); }
	MXS_CATCHERRORS();

	// Step 4: convert to python
	PyObject* output = NULL;
	if ( vl.mxs_value ) { output = ObjectWrapper::py_intern( vl.mxs_value ); }
	else { PyErr_SetString( PyExc_ArithmeticError, "__pow__ error: could not add maxscript values together" ); }

	// Step 5: cleanup maxscipt memory
	MXS_CLEANUP();

	return output;
}

// __abs__ function: called when trying to divide two wrapper values together
static PyObject*
ValueWrapper_absolute( PyObject* self ) {
	// Step 1: protect maxscript memory
	MXS_PROTECT( two_value_locals( mxs_self, mxs_value ) );
	
	// Step 2: convert the two items to values
	MXS_EVAL( ObjectWrapper::intern( self ), vl.mxs_self );

	// Step 3: calculate maxscript value
	try { vl.mxs_value = vl.mxs_self->abs_vf( NULL, 0 ); }
	MXS_CATCHERRORS();

	// Step 4: convert to python
	PyObject* output = NULL;
	if ( vl.mxs_value ) { output = ObjectWrapper::py_intern( vl.mxs_value ); }
	else { PyErr_SetString( PyExc_ArithmeticError, "__abs__ error: could not add maxscript values together" ); }

	// Step 5: cleanup maxscipt memory
	MXS_CLEANUP();

	return output;
}

// __int__ function: convert an object to an integer
static PyObject*
ValueWrapper_int( PyObject* self ) {
	// Step 1: protect maxscript memory
	MXS_PROTECT( one_value_local( mxs_self ) );
	
	// Step 2: convert the two items to values
	MXS_EVAL( ObjectWrapper::intern( self ), vl.mxs_self );

	// Step 3: calculate maxscript value
	int result = 0;
	try { result = vl.mxs_self->to_int(); }
	MXS_CATCHERRORS();

	// Step 4: convert to python
	PyObject* output = PyInt_FromLong( result );
	
	// Step 5: cleanup maxscipt memory
	MXS_CLEANUP();

	return output;
}

// __float__ function: convert an object to an integer
static PyObject*
ValueWrapper_float( PyObject* self ) {
	// Step 1: protect maxscript memory
	MXS_PROTECT( one_value_local( mxs_self ) );
	
	// Step 2: convert the two items to values
	MXS_EVAL( ObjectWrapper::intern( self ), vl.mxs_self );

	// Step 3: calculate maxscript value
	float result = 0;
	try { result = vl.mxs_self->to_float(); }
	MXS_CATCHERRORS();

	// Step 4: convert to python
	PyObject* output = PyFloat_FromDouble( result );
	
	// Step 5: cleanup maxscipt memory
	MXS_CLEANUP();

	return output;
}

// __neg__ function: return the negative of a wrapper object
static PyObject*
ValueWrapper_negative( PyObject* self ) {
	// Step 1: create the multiplier
	PyObject* mult		= PyInt_FromLong(-1);
	PyObject* result = ValueWrapper_multiply( self, mult );
	
	// Step 2: free the multiplier
	Py_DECREF( mult );

	return result;
}

// __nonzero__ function: return true always
static bool
ValueWrapper_nonzero( PyObject* self ){
	return true;
}

// create custom methods
static PyObject*
ValueWrapper_property( PyObject* self, PyObject* args ) {
	PyObject * ret = 0;
	char* propName;
	
	if ( !PyArg_ParseTuple( args, "s", &propName ) )
		return NULL;

	one_value_local(result);
	try { vl.result = ((ValueWrapper*) self)->mValue->eval()->_get_property( Name::intern(propName) ); }
	catch ( ... ) {
		PyObject * self_str = ValueWrapper_str((ValueWrapper*)self);
		PyErr_Format( PyExc_AttributeError, "%s is not a property of %s", propName, PyString_AsString(self_str) );
		Py_DECREF(self_str);
	}

	if ( vl.result ) {
		ret = ObjectWrapper::py_intern( vl.result );
	}
	
	pop_value_locals();
	return ret;
}

static PyObject*
ValueWrapper_setProperty( PyObject* self, PyObject* args ) {
	char* propName;
	PyObject* propValue;

	if ( !PyArg_ParseTuple( args, "sO", &propName, &propValue ) )
		return NULL;

	bool success = true;
	try { ((ValueWrapper*) self)->mValue->eval()->_set_property( Name::intern(propName), ObjectWrapper::intern(propValue) ); }
	catch ( ... ) { success = false; }

	if ( success ) {
		Py_INCREF(Py_True);
		return Py_True;
	}
	else {
		Py_INCREF(Py_False);
		return Py_False;
	}
}

//--------------------------------------------------------------
// create the python mappings

// define the custom methods for a wrapper
static PyMethodDef ValueWrapper_methods[] = {
	// windows methods
	{ "property",			(PyCFunction)ValueWrapper_property,			METH_VARARGS,	"Get a property from a MXS wrapper" },
	{ "setProperty",		(PyCFunction)ValueWrapper_setProperty,		METH_VARARGS,	"Set a property from a MXS wrapper" },
	{ NULL, NULL, 0, NULL }
};


// sequence methods
static PySequenceMethods proxy_as_sequence = {
	(lenfunc) ValueWrapper_length,			// sq_length
	0,										// sq_concat
	0,										// sq_repeat
	(ssizeargfunc) ValueWrapper_item,		// sq_item
	0,										// sq_slice
	(ssizeobjargproc) ValueWrapper_setitem,	// sq_ass_item
	0,										// sq_ass_slice
	0,										// sq_contains
	0,										// sq_inplace_concat
	0,										// sq_inplace_repeat
};

// mapping methods
static PyMappingMethods proxy_as_mapping = {
	(lenfunc) ValueWrapper_length,				// mp_length
	(binaryfunc) ValueWrapper_objitem,			// mp_subscript
	(objobjargproc) ValueWrapper_setobjitem,	// mp_ass_subscript
};

// number methods
static PyNumberMethods proxy_as_number = {
	(binaryfunc) ValueWrapper_add,				// nb_add
	(binaryfunc) ValueWrapper_subtract,			// nb_subtract
	(binaryfunc) ValueWrapper_multiply,			// nb_multiply
	(binaryfunc) ValueWrapper_divide,			// nb_divide
	0,											// nb_remainder
	0,											// nb_divmod
	(ternaryfunc) ValueWrapper_power,			// nb_power
	(unaryfunc) ValueWrapper_negative,			// nb_negative
	0,											// nb_positive
	(unaryfunc) ValueWrapper_absolute,			// nb_absolute
	(inquiry) ValueWrapper_nonzero,				// nb_nonzero
	0,											// nb_invert
	0,											// nb_lshift
	0,											// nb_rshift
	0,											// nb_and
	0,											// nb_xor
	0,											// nb_or
	0,											// nb_coerce
	(unaryfunc) ValueWrapper_int,				// nb_int
	0,											// nb_long
	(unaryfunc) ValueWrapper_float,				// nb_float,
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

// python type methods
static PyTypeObject ValueWrapperType = {
    PyObject_HEAD_INIT(NULL)
    0,																	// ob_size
    "value_wrapper",													// tp_name
    sizeof(ValueWrapper),												// tp_basicsize
    0,																	// tp_itemsize
    (destructor)ValueWrapper_dealloc,									// tp_dealloc
    0,																	// tp_print
    (getattrfunc)ValueWrapper_getattr,									// tp_getattr
    (setattrfunc)ValueWrapper_setattr,									// tp_setattr
    (cmpfunc)ValueWrapper_compare,										// tp_compare
    0,																	// tp_repr
    &proxy_as_number,													// tp_as_number
    &proxy_as_sequence,													// tp_as_sequence
    &proxy_as_mapping,													// tp_as_mapping
    0,																	// tp_hash 
    (ternaryfunc)ValueWrapper_call,										// tp_call
    (reprfunc)ValueWrapper_str,											// tp_str
    0,																	// tp_getattro
    0,																	// tp_setattro
    0,																	// tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_CHECKTYPES | Py_TPFLAGS_HAVE_CLASS,	// tp_flags
    "Maxscript Value Wrapper",											// tp_doc 
    0,																	// tp_traverse 
    0,																	// tp_clear 
    0,																	// tp_richcompare 
    0,																	// tp_weaklistoffset 
    0,																	// tp_iter 
    0,																	// tp_iternext 
    ValueWrapper_methods,												// tp_methods 
    0,																	// tp_members 
    0,																	// tp_getset 
    0,																	// tp_base 
    0,																	// tp_dict 
    0,																	// tp_descr_get 
    0,																	// tp_descr_set 
    0,																	// tp_dictoffset 
    0,																	// tp_init 
    0,																	// tp_alloc 
    ValueWrapper_new,													// tp_new 
};

//--------------------------------------------------------------------------------
// ObjectWrapper implementation
//--------------------------------------------------------------------------------

visible_class_instance( ObjectWrapper, "PyObjectWrapper" );

// ctor
ObjectWrapper::ObjectWrapper( PyObject* obj ) {
	this->tag			= class_tag(ObjectWrapper);
	this->mObject		= obj;
	this->mObjectDict	= NULL;

	Py_XINCREF(obj);
}

// dtor
ObjectWrapper::~ObjectWrapper() {
	Py_XDECREF( this->mObject );
	Py_XDECREF( this->mObjectDict );
	this->mObject		= NULL;
	this->mObjectDict	= NULL;
}

// __call__ function: call a python object with maxscript values
Value*
ObjectWrapper::apply( Value** arg_list, int count, CallContext* cc ) {
	// Step 1: Make sure this object is callable
	if ( this->mObject && PyCallable_Check( this->mObject ) ) {
		// Step 2; protect the maxscript memory
		MXS_PROTECT( one_value_local( output ) );

		vl.output = &undefined;

		// Step 3: determine the keyarg_marker to deliminate the break between args and keywords
		int py_count = 0;
		for ( int i = 0; i < count; i++ ) {
			// if the current item is the marker, then we have keywords
			if ( arg_list[i] == &keyarg_marker )
				break;
			py_count++;
		}

		// Step 4: generate the python arguments and keywords
		PyObject* args		= NULL;
		PyObject* kwds		= NULL;
		PyObject* py_result = NULL;

		// Step 5: generate the function arguments (args cannot be NULL)
		args = PyTuple_New( py_count );
		for ( int i = 0; i < py_count; i++ )
			PyTuple_SetItem( args, i, ObjectWrapper::py_intern( arg_list[i]->eval() ) );

		// Step 6: generate the function keywords
		if ( py_count != count ) {
			kwds = PyDict_New();
			for ( int i = py_count + 1; i < count; i += 2 )
				PyDict_SetItem( kwds, PyString_FromString( arg_list[i]->eval()->to_string() ), ObjectWrapper::py_intern( arg_list[i+1]->eval() ) );
		}

		// Step 7: execture the python call
		py_result = PyObject_Call( this->mObject, args, kwds );
		PY_CLEARERRORS();

		// Step 8: convert the result to a value
		
		if ( py_result ) { vl.output = ObjectWrapper::intern( py_result ); }
		else { mprintf( "Python Error: could not properly execute python function.\n" ); }

		// Step 9: release the python memory
		Py_XDECREF( args );
		Py_XDECREF( kwds );
		Py_XDECREF( py_result );

		// Step 10: return the value, protecting maxscript memory
		MXS_RETURN( vl.output );
	}
	else { return Value::apply( arg_list, count, cc ); }
}

// collect function: this function is run when the garbage collection determines this maxscript variable is ready to be destroyed
void
ObjectWrapper::collect() {
	// Python object cleanup is done in the dtor
	delete this;
}

// __eq__ function: checks to see if one objectwrapper is equal to another
Value*
ObjectWrapper::eq_vf( Value** arg_list, int count ) {
	if ( is_objectwrapper( arg_list[0] ) && is_objectwrapper( arg_list[0] ) ) {
		return ( ((ObjectWrapper*) arg_list[0])->object() == ((ObjectWrapper*) arg_list[1])->object() ) ? &true_value : &false_value;
	}
	return &false_value;
}

// __getattr__ function: get a property of a python value from maxscript
Value*
ObjectWrapper::get_property( Value** arg_list, int count ) {
	// Step 1: setup the output value
	MXS_PROTECT( one_value_local( output ) );

	vl.output = &undefined;

	// Step 2: get default properties
	if ( arg_list[0] == n_count ) {
		if ( PyMapping_Check( this->mObject ) )
			vl.output = Integer::intern( PyMapping_Length( this->mObject ) );
		else if ( PySequence_Check( this->mObject ) )
			vl.output = Integer::intern( PySequence_Length( this->mObject ) );
		else
			vl.output = Integer::intern( 0 );
	}

	// Step 3: lookup by keyword
	else {
		// try to access the attribute directly
		char* kstring = arg_list[0]->eval()->to_string();
		
		if ( PyObject_HasAttrString( this->mObject, kstring ) )
			vl.output = ObjectWrapper::intern( PyObject_GetAttrString( this->mObject, kstring ) );

		// because maxscript does not preserve the actual case sensitivity for the property, we have to check against
		// all possible options comparing lower case sensitvity to the keys
		else {
			// create a lowercase version of the maxscript key
			TSTR mxskstring = TSTR(kstring);
			mxskstring.toLower();

			if ( this->mObjectDict == NULL ) {
				// loop through the internal dictionary comparing the lowercase attributes to see if we have one
				PyObject **dictptr = _PyObject_GetDictPtr(this->mObject);
				if ( dictptr != NULL ) {
					this->mObjectDict = *dictptr;
					Py_XINCREF(this->mObjectDict);
				}
			}

			if ( this->mObjectDict != NULL ) {
				// These will be borrowed references during the loop
				PyObject *key, *value;
				Py_ssize_t pos = 0;

				while ( PyDict_Next( this->mObjectDict, &pos, &key, &value ) ) {
					// create a lowercase version of the python key
					TSTR pykstring = TSTR(PyString_AsString(key));
					pykstring.toLower();

					// return the first instance in the dictionary that has matching lowercase keys
					if ( pykstring == mxskstring ) {
						vl.output = ObjectWrapper::intern( PyObject_GetAttr( this->mObject, key ) );
						break;
					}
				}
			}
		}
	}

	// Step 4: return the value, protecting maxscript memory
	MXS_RETURN( vl.output );
}

// __setattr__ function: set a python value from maxscript
Value*
ObjectWrapper::set_property( Value** arg_list, int count ) {
	// Step 1: lookup by keyword
	char* kstring = arg_list[1]->eval()->to_string();

	if ( PyObject_HasAttrString( this->mObject, kstring ) )
		PyObject_SetAttrString( this->mObject, kstring, ObjectWrapper::py_intern( arg_list[0]->eval() ) );

	return &ok;
}

// __getitem__ function: get a value from a python sequence/mapping from maxscript
Value*
ObjectWrapper::get_vf( Value** arg_list, int count ) {
	MXS_PROTECT( two_value_locals( output, key ) );

	PyObject* py_key	= NULL;
	PyObject* py_result = NULL;

	// Step 1: convert the input keys
	vl.key = arg_list[0]->eval();

	try {
		if ( is_number( vl.key ) ) { py_key = PyInt_FromLong( vl.key->to_int() ); }
		else					{ py_key = PyString_FromString( vl.key->to_string() ); }
	}
	MXS_CATCHERRORS();

	// Step 2: call the getItem method for the python object
	if ( py_key && this->mObject ) {
		py_result = PyObject_GetItem( this->mObject, py_key );
	}

	// Step 4: convert the return to a maxscript value
	vl.output = ObjectWrapper::intern( py_result );

	// Step 5: release the python memory
	Py_XDECREF( py_key );
	Py_XDECREF( py_result );

	// Step 6: return the maxscript value, protecting its memory
	MXS_RETURN( vl.output );
}

// object function: return the python object for this instance
PyObject*
ObjectWrapper::object() {
	return this->mObject;
}

// __setitem__ function: sets an item for a python sequence/mapping type
Value*
ObjectWrapper::put_vf( Value** arg_list, int count ) {
	// \todo: implement this method
	return &ok;
}

// print function: prints this value to the screen
void
ObjectWrapper::sprin1( CharStream* s ) {
	s->puts( this->to_string() );
}

// __str__ function: converts this item to a string
char*
ObjectWrapper::to_string() {
	// Step 1: check to make sure we have an object
	if ( this->mObject ) {
		// Step 2: pull the python object string for this object
		PyObject* py_string = PyObject_Str( this->mObject );
		char* out = ( py_string ) ? PyString_AsString( py_string ) : "<<python: error converting value to string>>";
		PY_CLEARERRORS();
		
		// Step 3: release the python memory
		Py_XDECREF( py_string );

		return out;
	}
	return "<<python: error accessing wrapper object>>";
}

// Static methods

// intern method: create a Value* internal from a PyObject* instance
Value*
ObjectWrapper::intern( PyObject* obj ) {
	// Step 1: convert NULL or Py_None values
	if ( !obj || obj == Py_None )
		return &undefined;

	// Step 2: convert ValueWrapper instances
	else if ( obj->ob_type == &ValueWrapperType ) {
		one_value_local( output );
		vl.output = ((ValueWrapper*) obj)->mValue->eval();
		return_value( vl.output );
	}

	// Step 3: convert strings/unicodes
	else if ( obj->ob_type == &PyString_Type || obj->ob_type == &PyUnicode_Type ) {
		one_value_local(output);
		vl.output = new String(PyString_AsString( obj ));
		return_value(vl.output);
	}

	// Step 4: convert integers/longs
	else if ( obj->ob_type == &PyInt_Type || obj->ob_type == &PyLong_Type )
		return Integer::intern( PyInt_AsLong( obj ) );

	// Step 5: convert float
	else if ( obj->ob_type == &PyFloat_Type )
		return Double::intern( PyFloat_AsDouble( obj ) );

	// Step 6: convert boolean
	else if ( obj->ob_type == &PyBool_Type )
		return ( obj == Py_True ) ? &true_value : &false_value;

	// Step 7: convert lists/tuples
	else if ( obj->ob_type == &PyList_Type || obj->ob_type == &PyTuple_Type ) {
		int count	= PyObject_Length(obj);

		// Step 8: create a maxscript array of items
		one_typed_value_local(Array* output);
		vl.output = new Array(count);

		PyObject* iterator = PyObject_GetIter(obj);
		PyObject* item;

		if ( iterator != NULL ) {			
			while ( item = PyIter_Next(iterator) ) {
				vl.output->append( ObjectWrapper::intern( item ) );
				Py_DECREF(item);
			}

			Py_DECREF(iterator);
			if ( PyErr_Occurred() ) {
				// propogate error
			}
		}
		else {
				// propogate error
		}

		return_value(vl.output);
	}

	else {
		// Step 12: create a ObjectWrapper instance
		return new ObjectWrapper( obj );
	}
}

// initialize function: Initializes the PyObject* class
bool
ObjectWrapper::init() {
	return ( PyType_Ready( &ValueWrapperType ) < 0 ) ? false : true;
}

void
ObjectWrapper::log( PyObject* obj ) {
	PyObject * self_str = ValueWrapper_str((ValueWrapper*)obj);
	mprintf( "%s\n", PyString_AsString(self_str) );
	Py_DECREF(self_str);
}

// is_wrapper function: checks the type of the object to make sure its a ValueWrapper
bool
ObjectWrapper::is_wrapper( PyObject* obj ) {
	return ( obj->ob_type == &ValueWrapperType ) ? true : false;
}

void
ObjectWrapper::handleMaxscriptError() {
	MAXScriptException* e = thread_local(current_exception);
	// process the current exception
	if ( e ) {
		one_typed_value_local( StringStream* buffer );
		vl.buffer = new StringStream( "MAXScript Error has occurred: \n" );
		e->sprin1(vl.buffer);
		PyErr_SetString( PyExc_RuntimeError, vl.buffer->to_string() );
		pop_value_locals();
	}
	// set the exception to unknown
	else {
		PyErr_SetString( PyExc_RuntimeError, "An unknown MAXScript error has occurred" );
	}
}

// py_intern function: converts a maxscript internal to a python internal
PyObject*
ObjectWrapper::py_intern( Value* val ) {
	// Step 1: evaluate the value
	Value* mxs_check = NULL;
	MXS_EVAL( val, mxs_check );

	// Step 2: check for NULL, &undefined, or &unsupplied values
	if ( !mxs_check || mxs_check == &undefined || mxs_check == &unsupplied ) {
		Py_INCREF( Py_None );
		return Py_None;
	}

	// Step 3: check for ObjectWrappers
	if ( is_objectwrapper(mxs_check) ) {
		PyObject* output = ((ObjectWrapper*) mxs_check)->mObject;
		Py_INCREF( output );
		return output;
	}

	// Step 4: check for strings
	else if ( is_string( mxs_check ) )
		return PyString_FromString( mxs_check->to_string() );

	// Step 5: check for integers
	else if ( is_integer( mxs_check ) )
		return PyInt_FromLong( mxs_check->to_int() );

	// Step 6: check for all other numbers
	else if ( is_number( mxs_check ) )
		return PyFloat_FromDouble( mxs_check->to_float() );

	// Step 7: check for ok/true values
	else if ( mxs_check == &ok || mxs_check == &true_value ) {
		Py_INCREF( Py_True );
		return Py_True;
	}

	// Step 8: check for false values
	else if ( mxs_check == &false_value ) {
		Py_INCREF( Py_False );
		return Py_False;
	}

	// map object sets or arrays to a python array using a node_map
	else if ( is_objectset( mxs_check ) || is_array( mxs_check ) ) {
		PyObject* output = PyList_New(0);
		Value* args[2] = { NULL, (Value*)output };
		node_map m = { NULL, ObjectWrapper::collectionMapper, args, 2 };
		mxs_check->map( m );
		return output;
	}

	// Step 9: check for all collections (except bitarrays, modifiers, and objectsets)
	else if ( is_collection( mxs_check ) && !(is_bitarray( mxs_check ) || is_maxmodifierarray( mxs_check )) ) {
		// Step 10: grab the collection's count
		int count = mxs_check->_get_property( n_count )->to_int();
		
		// Step 11: create output array and maxscript index
		PyObject* output = PyList_New(count);
		Value* index;

		for ( int i = 0; i < count; i++ ) {
			// Step 12: set the maxscript index to the count + 1 (maxscript is 1 based)
			index = Integer::intern(i+1);
			PyList_SetItem( output, i, ObjectWrapper::py_intern( mxs_check->get_vf( &index, 1 ) ) );
		}

		return output;
	}
	else {
		// Step 13: create a new ValueWrapper instance
		ValueWrapper* output = (ValueWrapper*) ValueWrapper_new( &ValueWrapperType, NULL, NULL );
		
		one_value_local(heap_ptr);
		vl.heap_ptr = val->get_heap_ptr();
		output->mValue = vl.heap_ptr;
		Protector::protect( output );
		pop_value_locals();
		
		return (PyObject*) output;
	}
}

Value*
ObjectWrapper::collectionMapper( Value** arg_list, int count ) {
	// method used to map items from an array to a python array - faster than using the get_vf method for non-array's
	PyObject* collection	= (PyObject*)arg_list[1];
	PyObject* object		= ObjectWrapper::py_intern( arg_list[0] );
	PyList_Append( collection, object );
	Py_DECREF(object);
	return &ok;
}

HMODULE ObjectWrapper::hInstance = 0;