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

#include "Python.h"
#include "imports.h"
#include "wrapper.h"

// Returns pointer to new data owned by caller
MCHAR * pythonExceptionTraceback( bool clearException = true );

class PyExcRuntimeError : public RuntimeError
{
public:
	PyExcRuntimeError( MCHAR * error );
	~PyExcRuntimeError();
private:
	MCHAR * error;
};

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

#define PY_ERROR_PRINT_CLEAR \
	mprintf( pythonExceptionTraceback(true) );
	
// Call this macro to clean up any python errors that may have occurred
// TODO: Test for quiet mode
#define PY_ERROR_PRINT_THROW() \
	MCHAR * exc_str = pythonExceptionTraceback( /*clearException=*/ false ); \
	PyErr_Print(); \
	PyGILState_Release(gstate);\
	throw PyExcRuntimeError( exc_str );

#define		PY_ERROR_PROPAGATE()			if ( PyErr_Occurred() ) { \
												PY_ERROR_PRINT_THROW(); \
											}

#define PY_ERROR_PROPAGATE_MXS_CLEANUP() \
	MXS_CLEANUP(); \
	if ( PyErr_Occurred() ) { \
		PY_ERROR_PRINT_THROW(); \
	}

#define		PY_PROCESSERROR( PYEXC, MEXC )  MXS_CLEARERRORS(); \
											StringStream* buffer = new StringStream("MAXScript Error Has Occurred: \n"); \
											buffer->gc_trace(); \
											MEXC.sprin1(buffer); \
											PyErr_SetString( PYEXC, buffer->to_string() ); \
											delete buffer; \


#endif		__MACROS_H__