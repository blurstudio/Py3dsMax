/*
	\file		imports.h

	\remarks	This file contains with it all the imports necessary
				for this plugin to compile, and in the precise order
				they need to be imported.
*/

#ifndef	__IMPORTS_H__
#define __IMPORTS_H__

#include "MAXScrpt.h"
#include "Numbers.h"
#include "MAXclses.h"

// 3dsMax Includes
#include "structs.h"
#include "strings.h"
#include "objsets.h"
#include "Parser.h"
#include "maxobj.h"
#include "mathpro.h"

#include <Python.h>

// BlurPython Includes
#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

//#include "defextfn.h"
//#include "defimpfn.h"
//#include "definsfn.h"

#endif __IMPORTS_H__