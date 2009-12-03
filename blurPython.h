#ifndef		__BLURPYTHON_H__
#define		__BLURPYTHON_H__

#include "IPathConfigMgr.h"

PyMODINIT_FUNC init_max();

void BlurPythonInit() {
	init_plugin( "blurPython", 1000 );

	Py_Initialize();
	init_max();

	// Hi-jack the stdout && stderr and redirect to maxscript listener
//	PyRun_SimpleString( "import Py3dsMax" );
//	PyRun_SimpleString( "import sys" );
//	PyRun_SimpleString( "sys.stdout = Py3dsMax.StdLog()" );
//	PyRun_SimpleString( "sys.stderr = Py3dsMax.StdLog()" );
}

#endif		__BLURPYTHON_H__