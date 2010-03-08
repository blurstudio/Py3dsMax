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

#include "MAXScrpt.h"
#include "Numbers.h"
#include "MAXclses.h"

// 3dsMax Includes
#include "structs.h"
#include "strings.h"
#include "excepts.h"
#include "Parser.h"
//#include "objsets.h"
//#include "maxobj.h"
//#include "mathpro.h"

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