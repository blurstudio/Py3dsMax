#ifndef		__OBJECTVALUEWRAPPER_H__
#define		__OBJECTVALUEWRAPPER_H__

#include "structs.h"
#include <string>

#define		THROW_PYERROR( ERROR, PYEXCEPT, VALUE )			StringStream* s = new StringStream(); \
															ERROR.sprin1( s ); \
															PyErr_SetString( PYEXCEPT, s->to_string() ); \
															return VALUE

visible_class( ObjectValueWrapper );
class ObjectValueWrapper : public Value {
	private:
		PyObject*			_pyobj;

	public:
							ObjectValueWrapper( PyObject* pyobj );
							~ObjectValueWrapper();

							classof_methods( ObjectValueWrapper, Value );
#define						is_pyobject(p)	((p)->tag == class_tag(ObjectValueWrapper))
		// Built-Ins
		TSTR				__str__();
		
		// Methods
		Value*				apply(			Value** arg_list, int count, CallContext* cc = NULL );
		static PyObject*	args(			Value** arg_list, int count );
		void				collect()		{ delete this; }
		Value*				get_property(	Value** arg_list, int count );
		Value*				lookup(			std::string key );
		static Value*		intern(			PyObject* item );
		static PyObject*	pyintern(		Value* item );
		static bool			init();
		PyObject*			pyobject();
		void				sprin1(			CharStream* s );
		char*				to_string();

		def_generic(		eq,				"==" );
		def_generic(		get,			"get" );
		def_generic(		put,			"put" );
		def_generic(		get_props,		"getPropNames" );
};

#endif		__OBJECTVALUEWRAPPER_H__