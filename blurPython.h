#ifndef		__BLURPYTHON_H__
#define		__BLURPYTHON_H__

#include "IPathConfigMgr.h"

// the initialize_blurpython function is found in the studiomax_module file
PyMODINIT_FUNC initialize_blurpython();

void BlurPythonInit() {
	// Step 1: initialize the blur plugin
	init_plugin( "blurPython", 1100 );

	// Step 2: initialize python
	Py_Initialize();

	// Step 3: initialize 3dsMax
	initialize_blurpython();
}

#endif		__BLURPYTHON_H__