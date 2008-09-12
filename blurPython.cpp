// MAXScript Includes
#include "max\include\imports.h"
#include <Python.h>

#pragma comment( lib, "comctl32.lib" )

// Load Initialization Code
#include "max\include\init.h"
#include "max\include\exports.h"
#include "blurPython.h"

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
			Py_Finalize();
		}
	}

	return TRUE;
}

__declspec( dllexport ) void				LibInit()			{ BlurPythonInit(); }
__declspec( dllexport ) const TCHAR*		LibDescription()	{ return _T( "Python MAXScript Extension" ); }
__declspec( dllexport ) ULONG				LibVersion()		{ return VERSION_3DSMAX; }