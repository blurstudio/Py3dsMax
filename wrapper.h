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

visible_class( ObjectWrapper );
class ObjectWrapper : public Value {
private:
	PyObject*		mObject;

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
	char*			to_string();

	BOOL			_is_collection()	{ return 1; }
	BOOL			_is_function()		{ return 1; }

	// objectwrapper static methods
	static Value*		intern( PyObject* obj );
	static bool			init();
	static void			gc_protect( PyObject* obj );
	static bool			is_wrapper( PyObject* obj );
	static PyObject*	py_intern( Value* val );
};

#endif		__WRAPPER_H__