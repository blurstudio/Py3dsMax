// 3dsMax Includes
#include "max\include\imports.h"

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <Python.h>
#include "..\classes\ObjectValueWrapper.h"

#include <import.h>
#include <graminit.h>
#include <pythonrun.h>

// BlurPython Includes
#include "max\include\exports.h"

def_struct_primitive( import,			python,			"import" );
def_struct_primitive( reload,			python,			"reload" );
def_struct_primitive( run,				python,			"run" );
def_struct_primitive( exec,				python,			"exec" );

#define CHECK_PYERROR()					if ( PyErr_Occurred() ) { \
											PyErr_Print(); \
											throw RuntimeError( "Python Exception: Traceback printed in listener." ); \
										}

Value*		import_cf(			Value** arg_list, int count ) {
	check_arg_count( import, 1, count );

	PyObject* module	= PyImport_ImportModule( arg_list[0]->eval()->to_string() );

	CHECK_PYERROR();
	
	if ( module ) { return ObjectValueWrapper::intern( module ); }
	return &undefined;
}
Value*		reload_cf(			Value** arg_list, int count ) {
	check_arg_count( reload, 1, count );
	PyImport_ReloadModule( ObjectValueWrapper::pyintern( arg_list[0]->eval() ) );

	CHECK_PYERROR();

	return &ok;
}
Value*		run_cf(				Value** arg_list, int count ) {
	check_arg_count( run, 1, count );

	// Check to see the path exists
	char* path = arg_list[0]->eval()->to_string();
	struct stat st;
	if( stat(path,&st) != 0 ){ return &false_value; }

	// Run the file in the main module
	PyObject* main_module		= PyImport_AddModule( "__main__" );
	PyObject* main_dict			= PyModule_GetDict(main_module);

	// Open the file
	PyObject* fileObject		= PyFile_FromString( path, "r" );
	FILE* file					= PyFile_AsFile( fileObject );
	PyRun_File( file, path, Py_file_input, main_dict, main_dict );

	Py_DECREF( fileObject );
	
	CHECK_PYERROR();

	return &true_value;
}
Value*		exec_cf(			Value** arg_list, int count ) {
	check_arg_count( exec, 1, count );

	char* cmd	= arg_list[0]->to_string();
	PyRun_SimpleString( cmd );

	CHECK_PYERROR();

	return &ok;
}
