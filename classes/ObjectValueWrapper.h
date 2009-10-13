#ifndef		__OBJECTVALUEWRAPPER_H__
#define		__OBJECTVALUEWRAPPER_H__

#include "structs.h"
#include <string>

#define		MXS_CLEARERRORS()								clear_error_source_data(); \
															restore_current_frames(); \
															MAXScript_signals = 0;

#define		MXS_PROTECT( VALUE_LOCALS )						init_thread_locals(); \
															push_alloc_frame(); \
															VALUE_LOCALS; \
															save_current_frames();

#define		MXS_CLEANUP()									pop_value_locals(); \
															pop_alloc_frame();

#define		MXS_EVAL(VARIABLE)								VARIABLE = ((MXSValueWrapper*) self)->value; \
															while ( VARIABLE != NULL && is_thunk( VARIABLE ) ) \
																VARIABLE = VARIABLE->eval();

#define		THROW_PYERROR( ERROR, PYEXCEPT )				vl.errlog = new StringStream(); \
															ERROR.sprin1( (StringStream*) vl.errlog ); \
															PyErr_SetString( PYEXCEPT, vl.errlog->to_string() ); \
															MXS_CLEARERRORS();

#define		CATCH_ERRORS()									catch ( ... ) { \
																MXS_CLEARERRORS(); \
																PyErr_SetString( PyExc_Exception, "MAXScript Error Occured. See MAXScript Logger for more details." ); \
															}

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
		static bool			init();
		static bool			isWrapper(		PyObject* item );
		static PyObject*	pyintern(		Value* item );
		PyObject*			pyobject();
		void				sprin1(			CharStream* s );
		char*				to_string();

		def_generic(		eq,				"==" );
		def_generic(		get,			"get" );
		def_generic(		put,			"put" );
		def_generic(		get_props,		"getPropNames" );
};

#endif		__OBJECTVALUEWRAPPER_H__