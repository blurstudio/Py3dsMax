/*
	\file		wrapper.h

	\remarks	This is the main class definition for the wrapper classes.
				* ValueWrapper is a PyObject* class that wraps a Value* class 
				* ObjectWrapper is a Value* class that wraps a PyObject* class
	
	\author		Blur Studio (c) 2010
	\email		beta@blur.com

	\license	This software is released under the GNU General Public License.  For more info, visit: http://www.gnu.org/
*/

#ifndef		__WRAPPER_H__
#define		__WRAPPER_H__

#include "imports.h"

visible_class( ObjectWrapper );
class ObjectWrapper : public Value {
private:
	PyObject*						mObject;
	PyObject*						mObjectDict;
	PyObject*						mPyString;
public:
	ObjectWrapper( PyObject* obj = NULL );
	~ObjectWrapper();

	// maxscript value methods
	__declspec( dllexport ) virtual Value* eq_vf(  Value** arg_list, int arg_count );
	__declspec( dllexport ) virtual Value* get_vf( Value** arg_list, int arg_count );
	__declspec( dllexport ) virtual Value* put_vf( Value** arg_list, int arg_count );

	// maxscript value comparisons
	classof_methods( ObjectWrapper, Value );
#define is_objectwrapper(o) ((o)->tag == class_tag(ObjectWrapper))
#define is_maxmodifierarray(o) ((o)->tag == class_tag(MAXModifierArray))

	// objectwrapper instance methods
	Value*			apply( Value** arg_list, int count, CallContext* cc = 0 );
	virtual void	collect();
	Value*			get_property( Value** arg_list, int count );
	Value*			set_property( Value** arg_list, int count );
	PyObject*		object();
	void			sprin1( CharStream* s );
	const MCHAR*			to_string();

	BOOL			_is_collection()	{ return 1; }
	BOOL			_is_function()		{ return 1; }

	// objectwrapper static methods
	static Value*		intern( PyObject* obj, bool unwrap = true );
	static Value*		collectionMapper( Value** args, int count );
	static bool			init();
	static void			handleMaxscriptError();
	static bool			is_wrapper( PyObject* obj );
	static void			log(		PyObject* obj );
	static PyObject*	py_intern( Value* val );

	static HMODULE hInstance;
};

struct ValueWrapper {
	PyObject_HEAD
	Value*		mValue;			// Pointer to the MAXScript Value
	// Double linked list of ValueWrappers, used to provide
	// O(n) iteration by the Protector class, and O(1) insertion
	// and deletion.  The Protector stores the head of the list.
	ValueWrapper * mPrev, * mNext;
	PyObject* dict;
};

// Define the type object struct for use externally when building the
// Py3dsMax module, so that we have access to the type from the module.
extern PyTypeObject ValueWrapperType;
extern PyTypeObject TypedFloatType;
void TypedFloatType_init();


// Converts a python string object to MCHAR *
class PyStringToMCHAR {
public:
	PyStringToMCHAR( PyObject *, bool stealRef=false );
	PyStringToMCHAR( const char * );
	~PyStringToMCHAR();
	const MCHAR * mchar();
protected:
	PyObject * mObject;
#ifndef UNICODE
	const char * mData;
#endif
};

// Converts am MCHAR * to a python string
class MCharToPyString {
public:
	MCharToPyString( const MCHAR * );
	~MCharToPyString();
	// Returns borrowed ref
	PyObject * pyString();
	// Returns a new ref
	PyObject * pyStringRef();
	// Returns the internal data of the python string, same as PyString_AsString(pyString());
	const char * data();
protected:
	const MCHAR * mMChars;
	PyObject * mObject;
#ifdef UNICODE
	PyObject * mUtf8Object;
#endif
};

#endif		__WRAPPER_H__
