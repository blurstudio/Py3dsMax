// 3dsMax Includes
#include "max\include\imports.h"

#include <Python.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include "..\classes\ObjectValueWrapper.h"

#include <import.h>
#include <graminit.h>
#include <pythonrun.h>

// BlurPython Includes
#include "max\include\exports.h"

def_struct_primitive( import,			python,			"import" );
def_struct_primitive( reload,			python,			"reload" );
def_struct_primitive( run,				python,			"run" );
def_struct_primitive( shell,			python,			"shell" );

#define CHECK_PYERROR()					if ( PyErr_Occurred() ) { \
											PyErr_Print(); \
											throw RuntimeError( "Python Exception Occured" ); \
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
	return &ok;
}
Value*		run_cf(				Value** arg_list, int count ) {
	check_arg_count( run, 1, count );
	char* path = arg_list[0]->eval()->to_string();
	struct stat st;
	if( stat(path,&st) != 0 ){ return &false_value; }
	int result = PyRun_SimpleFile( PyFile_AsFile( PyFile_FromString( path, "r" ) ), path );
	return (result) ? &true_value : &false_value;
}
Value*		shell_cf(			Value** arg_list, int count ) {
	check_arg_count( shell, 0, count );
	char* args = "";
	Py_Main(0, &args);
	return &true_value;
}