/*
	\file		python_struct.cpp

	\remarks	This file defines the 3dsMax struct called python

	\author		Blur Studio (c) 2010
	\email		beta@blur.com

	\license	This software is released under the GNU General Public License.  For more info, visit: http://www.gnu.org/
*/

#include "imports.h"

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>

#include <import.h>
#include <graminit.h>
#include <pythonrun.h>

// updated to work with max 2012's new maxscript organization
#ifdef __MAXSCRIPT_2012__

#include "macros/define_external_functions.h"
#include "macros/define_instantiation_functions.h"

// include settings for previous versions of 3dsmax
#else

#include "defextfn.h"
#include "definsfn.h"

#endif

#include "macros.h"
#include "wrapper.h"

// Define the python struct in 3dsMax
def_struct_primitive( import,			python,			"import" );
def_struct_primitive( reload,			python,			"reload" );
def_struct_primitive( run,				python,			"run" );
def_struct_primitive( exec,				python,			"exec" );

// python.import function: import a python module to maxscript
Value*
import_cf( Value** arg_list, int count ) {
	// Step 1: make sure the arguments supplied are correct
	check_arg_count( python.import, 1, count );

	// Step 2: protect the maxscript memory
	MXS_PROTECT( two_value_locals( mxs_check, mxs_return ) );

	// Step 3: convert the maxscript value
	MXS_EVAL( arg_list[0], vl.mxs_check );

	// Step 4: calculate the python module name to import
	char* module_name = NULL;
	try { module_name = vl.mxs_check->to_string(); }
	MXS_CATCHERRORS();

	// Step 5: import the module
	if ( module_name ) {
		PyObject* module = PyImport_ImportModule( module_name );
		PY_ERROR_PROPAGATE_MXS_CLEANUP();

		vl.mxs_return = ( module ) ? ObjectWrapper::intern( module ) : &undefined;
	}
	else {
		mprintf( "python.import() error: importing modules must be done with a string value\n" );
		vl.mxs_return = &undefined;
	}
	MXS_RETURN( vl.mxs_return );
}

// python.reload function: reload an existing module in maxscript
Value*
reload_cf( Value** arg_list, int count ) {
	// Step 1: make sure the arguments supplied are correct in count
	check_arg_count( python.reload, 1, count );

	// Step 2: evaluate the input item
	MXS_PROTECT(one_value_local(mxs_check));
	MXS_EVAL( arg_list[0], vl.mxs_check );
	
	// Step 3: make sure the item is a proper type
	if ( is_objectwrapper(vl.mxs_check) ) {
		PyImport_ReloadModule( ((ObjectWrapper*) vl.mxs_check)->object() );
		PY_ERROR_PROPAGATE_MXS_CLEANUP();
	}
	else { mprintf( "python.reload() error: you need to supply a valid python module to reload\n" ); }

	MXS_CLEANUP();
	return &ok;	
}

// python.run function: run a python file from maxscript
Value*
run_cf( Value** arg_list, int count ) {
	// Step 1: make sure the arguments supplied are correct in count
	check_arg_count( python.run, 1, count );

	// Step 2: protect the maxscript memory
	MXS_PROTECT(one_value_local(mxs_filename));
	MXS_EVAL( arg_list[0], vl.mxs_filename );

	// Step 2: create a python file based on the filename
	char* filename		= vl.mxs_filename->to_string();
	PyObject* py_file	= PyFile_FromString( filename, "r" );

	// Step 3: check to make sure the file exists
	if ( !py_file ) {
		MXS_CLEANUP();
		return &false_value;
	}

	// Step 4: run the file
	PyRun_SimpleFile( PyFile_AsFile(py_file), filename );
	PY_ERROR_PROPAGATE_MXS_CLEANUP();

	// Step 5: cleanup the memory
	Py_XDECREF( py_file );
	MXS_CLEANUP();
	
	return &true_value;
}

// python.exec function: execute a python command through a maxscript string
Value*
exec_cf( Value** arg_list, int count ) {
	// Step 1: make sure the arguments supplied are correct in count
	check_arg_count( python.exec, 1, count );

	// Step 2: protect the maxscript memory
	MXS_PROTECT(one_value_local(mxs_command));
	MXS_EVAL( arg_list[0], vl.mxs_command );

	// Step 2: create a python file based on the filename
	char* command	= NULL;
	try { command	= vl.mxs_command->to_string(); }
	MXS_CATCHERRORS();

	// Step 3: check to make sure the command is valid
	if ( !command ) {
		MXS_CLEANUP();
		return &false_value;
	}

	// Step 4: run the command
	PyRun_SimpleString( command );
	PY_ERROR_PROPAGATE_MXS_CLEANUP();

	// Step 5: cleanup the memory
	MXS_CLEANUP();
	
	return &ok;
}

PyExcRuntimeError::PyExcRuntimeError( char * _error )
: RuntimeError( _error )
, error( _error )
{}

PyExcRuntimeError::~PyExcRuntimeError()
{
	delete error;
	// Hopefully this is safe and keeps RuntimeError from double deleting our copy of the string
	desc1 = 0;
}

// Returns a new string
char * pythonExceptionTraceback( bool clearException )
{
	/*
		import traceback
		return '\n'.join(traceback.format_exc()).encode('ascii','replace')
	*/
	char * ret = 0;
	bool success = false;

	PyObject * type, * value, * traceback;
	/* Save the current exception */
	PyErr_Fetch(&type, &value, &traceback);
	if( type ) {
	
		PyObject * traceback_module = PyImport_ImportModule("traceback");
		if (traceback_module) {

			/* Call the traceback module's format_exception function, which returns a list */
			PyObject * traceback_list = PyObject_CallMethod(traceback_module, "format_exception", "OOO", type, value ? value : Py_None, traceback ? traceback : Py_None);
			if( traceback_list ) {

				PyObject * separator = PyString_FromString("");
				if( separator ) {

					PyObject * retUni = PyUnicode_Join(separator, traceback_list);
					if( retUni ) {
						PyObject * retAscii = PyUnicode_AsEncodedString(retUni, "ascii", "replace");
						if( retAscii ) {
#ifdef Py_ssize_t
							Py_ssize_t len = 0;
#else
							int len = 0;
#endif
							char * tmp;
							if( PyString_AsStringAndSize( retAscii, &tmp, &len ) != -1 ) {
								ret = strdup( tmp ); //, len );
								Py_DECREF( retAscii );
								success = true;
							} else {
								ret = strdup( "Uhoh, failed to get pointer to ascii representation of the exception" );
								success = false;
							}
							Py_DECREF( retAscii );
						} else {
							ret = strdup( "Uhoh, encoding exception to ascii failed" );
							success = false;
						}
						Py_DECREF(retUni);

					} else
						ret = strdup("PyUnicode_Join failed");

					Py_DECREF(separator);
				} else
					ret = strdup("PyUnicode_FromString failed");

				Py_DECREF(traceback_list);
			} else
				ret = strdup("Failure calling traceback.format_exception");

			Py_DECREF(traceback_module);
		} else
			ret = strdup("Unable to load the traceback module, can't get exception text");
	} else
		ret = strdup("pythonExceptionTraceback called, but no exception set");

	if( clearException ) {
		Py_DECREF(type);
		Py_XDECREF(value);
		Py_XDECREF(traceback);
	} else
		PyErr_Restore(type,value,traceback);

	return ret;
}