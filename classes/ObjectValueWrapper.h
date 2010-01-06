#ifndef		__OBJECTVALUEWRAPPER_H__
#define		__OBJECTVALUEWRAPPER_H__

#include "structs.h"
#include "excepts.h"

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

#define		MXS_GC_PROTECT( RETURN )						if ( ((MXSValueWrapper*) self)->value ) { \
																((MXSValueWrapper*) self)->value->gc_trace(); \
															} \
															else { \
																PyErr_SetString( PyExc_Exception, "Attempting to access Maxscript wrapper value that has been collected by MAXScript garbage collection." ); \
																return RETURN; \
															}

#define		MXS_EVAL(VARIABLE)								VARIABLE = ((MXSValueWrapper*) self)->value; \
															while ( VARIABLE != NULL && is_thunk( VARIABLE ) ) \
																VARIABLE = VARIABLE->eval();

#define		THROW_PYERROR( ERROR, PYEXCEPT )				MXS_CLEARERRORS(); \
															StringStream* log = new StringStream(); \
															ERROR.sprin1( log ); \
															PyErr_SetString( PYEXCEPT, log->to_string() );

#define		CATCH_ERRORS()									catch ( MAXScriptException e ) { \
																MXS_CLEARERRORS(); \
																PyErr_SetString( PyExc_Exception, "MAXScript error occurred" ); \
															} \
															catch ( ... ) { \
																MXS_CLEARERRORS(); \
																PyErr_SetString( PyExc_Exception, "MAXScript error occurred" ); \
															}

class ObjectValueWrapper;

applyable_class( Protector );
class Protector : public Value {
public:
			Protector();
			~Protector();

	classof_methods( Protector, Value );
	void	collect();
};

visible_class( ObjectValueWrapper );
class ObjectValueWrapper : public Value {
	private:
		PyObject*			_pyobj;
		static Value*		_protector;

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
		static PyObject*	cache;
		void				collect()		{ delete this; }
		static void			gc_protect();
		Value*				get_property(	Value** arg_list, int count );
		Value*				lookup(			std::string key );
		static Value*		intern(			PyObject* item );
		static bool			init();
		static bool			isWrapper(		PyObject* item );
		static void			logError(		MAXScriptException err );
		static PyObject*	pyintern(		Value* item, bool make_static = true );
		PyObject*			pyobject();
		void				sprin1(			CharStream* s );
		char*				to_string();

		def_generic(		eq,				"==" );
		def_generic(		get,			"get" );
		def_generic(		put,			"put" );
		def_generic(		get_props,		"getPropNames" );
};

#endif		__OBJECTVALUEWRAPPER_H__