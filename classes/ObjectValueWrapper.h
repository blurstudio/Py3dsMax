#ifndef		__OBJECTVALUEWRAPPER_H__
#define		__OBJECTVALUEWRAPPER_H__

#include "structs.h"
#include <string>

#define		THROW_PYERROR( ERROR, PYEXCEPT, RETURN )		StringStream* s = new StringStream(); \
															ERROR.sprin1( s ); \
															PyErr_SetString( PYEXCEPT, s->to_string() ); \
															return RETURN

#define		CATCH_ERRORS( RETURN )							catch ( AccessorError e )				{ THROW_PYERROR( e, PyExc_Exception, RETURN ); } \
															catch ( ArgCountError e )				{ THROW_PYERROR( e, PyExc_Exception, RETURN ); } \
															catch ( AssignToConstError e )			{ THROW_PYERROR( e, PyExc_Exception, RETURN ); } \
															catch ( CompileError e )				{ THROW_PYERROR( e, PyExc_Exception, RETURN ); } \
															catch ( ConversionError e )				{ THROW_PYERROR( e, PyExc_TypeError, RETURN ); } \
															catch ( DebuggerRuntimeError e )		{ THROW_PYERROR( e, PyExc_Exception, RETURN ); } \
															catch ( IncompatibleTypes e )			{ THROW_PYERROR( e, PyExc_TypeError, RETURN ); } \
															catch ( NoMethodError e	)				{ THROW_PYERROR( e, PyExc_AssertionError, RETURN ); } \
															catch ( RuntimeError e )				{ THROW_PYERROR( e, PyExc_RuntimeError, RETURN ); } \
															catch ( SignalException e )				{ THROW_PYERROR( e, PyExc_Exception, RETURN ); } \
															catch ( TypeError e )					{ THROW_PYERROR( e, PyExc_TypeError, RETURN ); } \
															catch ( UnknownSystemException e )		{ THROW_PYERROR( e, PyExc_SystemError, RETURN ); } \
															catch ( UserThrownError e )				{ THROW_PYERROR( e, PyExc_Exception, RETURN ); } \
															catch ( MAXScriptException e )			{ THROW_PYERROR( e, PyExc_Exception, RETURN ); } \
															catch ( ... ) { \
																PyErr_SetString( PyExc_Exception, "Unknown Error Occured." ); \
																return RETURN; \
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