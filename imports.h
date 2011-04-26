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

// max 2012 switched the name of maxscript includes
#ifdef __MAXSCRIPT_2012__

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

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#endif __IMPORTS_H__