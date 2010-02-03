/*
	\file		wrapper.h

	\remarks	This is the main class definition for the wrapper classes.
				* ValueWrapper is a PyObject* class that wraps a Value* class 
				* ObjectWrapper is a Value* class that wraps a PyObject* class
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

	classof_methods( ObjectWrapper, Value );
#define is_objectwrapper(o) ((o)->tag == class_tag(ObjectWrapper))

	// Instance Methods
	Value*			apply( Value** arg_list, int count, CallContext* cc = 0 );
	virtual void	collect();
	Value*			get_property( Value** arg_list, int count );
	Value*			set_property( Value** arg_list, int count );
	PyObject*		object();
	void			sprin1( CharStream* s );
	char*			to_string();

	BOOL			_is_collection()	{ return 1; }
	BOOL			_is_function()		{ return 1; }

	// Static Methods
	static Value*		intern( PyObject* obj );
	static bool			init();
	static void			gc_protect( PyObject* obj );
	static bool			is_wrapper( PyObject* obj );
	static PyObject*	py_intern( Value* val );

	// Define MAXScript comparisons
//	def_generic( eq, "==" );
//	Value*				eq_vf(	Value** arglist, int arg_count);
//	Value*				get_vf(	Value** arglist, int arg_count);
//	Value*				put_vf(	Value** arglist, int arg_count);
};

#endif		__WRAPPER_H__