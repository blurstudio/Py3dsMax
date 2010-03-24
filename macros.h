/*
	\file		macros.h

	 \remarks	Protecting maxscript memory is a hard thing to do, and its often easy to
				forget the proper steps, and sometimes, easy to do it incorrectly.  These macros
				make it a bit easier to work with.

	\author		Blur Studio (c) 2010
	\email		beta@blur.com

	\license	This software is released under the GNU General Public License.  For more info, visit: http://www.gnu.org/

*/

#ifndef		__MACROS_H__
#define		__MACROS_H__

#define		DEBUG_MODE						true

#define		DEBUG_MSG( MSG )				if ( DEBUG_MODE ) { PyRun_SimpleString( "print %%MSG%%" ); }

// Call this function to define and protect a series of values
#define		MXS_PROTECT( VALUE_LOCALS )		init_thread_locals(); \
											push_alloc_frame(); \
											VALUE_LOCALS; \
											save_current_frames();

// Split a Value* from its thunk containers
#define		MXS_EVAL(INPUT,OUTPUT)			OUTPUT = INPUT; \
											while ( OUTPUT != NULL && is_thunk(OUTPUT) ) \
												OUTPUT = OUTPUT->eval();

// Clear up MAXScript memory properly after an error
#define		MXS_CLEARERRORS()				clear_error_source_data(); \
											restore_current_frames(); \
											MAXScript_signals = 0;

// Quick macro to catch all errors, clean them, and set a MAXScript error for Python
#define		MXS_CATCHERRORS()				catch ( ... ) { \
												MXS_CLEARERRORS(); \
											}

// Call this macro to cleanup MAXScript memory before exiting a function
#define		MXS_CLEANUP()					pop_value_locals(); \
											pop_alloc_frame();


// Call this macro to cleanup MAXScript memory before exiting a function
#define		MXS_RETURN(VAL)					pop_alloc_frame(); \
											return_value( VAL );


// Call this macro to clean up any python errors that may have occurred
#define		PY_CLEARERRORS()			if ( PyErr_Occurred() ) { \
											PyErr_Print(); \
											throw RuntimeError( "Python Exception: Traceback printed in listener." ); \
										}

#define		PY_PROCESSERROR( PYEXC, MEXC )  MXS_CLEARERRORS(); \
											StringStream* buffer = new StringStream("MAXScript Error Has Occurred: \n"); \
											buffer->gc_trace(); \
											MEXC.sprin1(buffer); \
											PyErr_SetString( PYEXC, buffer->to_string() ); \
											delete buffer; \

#define		PY_CATCHMXSERROR()			catch ( MAXScriptException& e ) { \
											error_message_box( e, "Error" ); \
											PyErr_SetString( PyExc_RuntimeError, "MAXScript Error Occurred - see listener for details." ); \
										} \
										catch ( ... ) { \
											MXS_CLEARERRORS(); \
											PyErr_SetString( PyExc_RuntimeError, "Unknown MAXScript Error Occurred" ); \
										}



#endif		__MACROS_H__