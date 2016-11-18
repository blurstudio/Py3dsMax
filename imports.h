/*
	\file		imports.h

	\remarks	This file contains with it all the imports necessary
				for this plugin to compile, and in the precise order
				they need to be imported.

	\author		Blur Studio (c) 2010
	\email		beta@blur.com

	\license	This software is released under the GNU General Public License.  For more info, visit: http://www.gnu.org/
*/

#ifndef	__IMPORTS_H__
#define __IMPORTS_H__


#ifdef __MAXSCRIPT_2016__
#define __MAXSCRIPT_2015__
#endif

// As of 2013 max is compiled with UNICODE enabled - MCHAR == TCHAR == wchar == Py_UNICODE, utf16
#if __MAXSCRIPT_2013__ || __MAXSCRIPT_2015__
#define _UNICODE
#define UNICODE
#endif

// max 2012 switched the name of maxscript includes
#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__ || __MAXSCRIPT_2015__

#include "foundation/numbers.h"
#include "foundation/structs.h"
#include "foundation/strings.h"

#include "kernel/exceptions.h"
#include "kernel/value.h"

#include "maxwrapper/maxclasses.h"
#include "maxwrapper/objectsets.h"

#include "compiler/parser.h"

#include "maxscript.h"
#include "ScripterExport.h"

// these are the includes for previous versions of 3dsmax
#else

#include "MAXScrpt.h"
#include "Numbers.h"
#include "MAXclses.h"
#include "structs.h"
#include "strings.h"
#include "excepts.h"
#include "Parser.h"
#include "maxscrpt.h"
#include "value.h"

#endif

// include python headers
#include <Python.h>

#if PY_VERSION_HEX < 0x02050000 && !defined(PY_SSIZE_T_MIN)
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
#endif

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#endif __IMPORTS_H__