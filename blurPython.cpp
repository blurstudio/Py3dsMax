/*
	\file		blurPython.cpp

	\remarks	Entry point for the blurPython dlx

	\author		Blur Studio (c) 2010
	\email		beta@blur.com

	\license	This software is released under the GNU General Public License.  For more info, visit: http://www.gnu.org/
*/

#include "imports.h"

#pragma comment( lib, "comctl32.lib" )

//------------------------------------------------------------------------------------------------------
HMODULE hInstance	= NULL;
HINSTANCE g_hInst;

BOOL APIENTRY		DLLMain( HMODULE hModule, DWORD ul_reason, LPVOID lpReserved ) {
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof( icex );
	icex.dwICC	= ICC_TREEVIEW_CLASSES | ICC_LISTVIEW_CLASSES;

	switch ( ul_reason ) {
		case DLL_PROCESS_ATTACH: {
			hInstance	= hModule;
			g_hInst		= hModule;

			DisableThreadLibraryCalls( hModule );
			break;
		}
		case DLL_PROCESS_DETACH: {
			// Kill the python system
#ifndef __MAXSCRIPT_2015__
			// This should only be run in versions of max that don't have python native integration
			Py_Finalize();
#endif
		}
	}

	return TRUE;
}

// the init_module function is found in the studiomax_module file
#ifdef __MAXSCRIPT_2015__
// calling init_module when LibInit is called will cause max to crash.
__declspec( dllexport ) void				LibInit()			{}
#else
PyMODINIT_FUNC init_module();

__declspec( dllexport ) void				LibInit()			{ init_module(); }
#endif

__declspec( dllexport ) const TCHAR*		LibDescription()	{ return _T( "Py3dsMax Python Extension" ); }
__declspec( dllexport ) ULONG				LibVersion()		{ return VERSION_3DSMAX; }

// Maxscript 2012 requires these additional exports
#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__ || __MAXSCRIPT_2015__

// we aren't defining any classes in this plugin, so this is pretty easy
// other plugins should refer to samples/scriptplugin for an example of the new
// registration system - that is the only plugin that does not error out of the samples
// in max
__declspec( dllexport ) int					LibNumberClasses()		{ return 0; }
__declspec( dllexport ) ClassDesc*			LibClassDesc( int i )	{ return 0; }

#endif