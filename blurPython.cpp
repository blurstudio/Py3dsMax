// MAXScript Includes
#include "imports.h";

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
			Py_Finalize();
		}
	}

	return TRUE;
}

// the init_module function is ofund in the studiomax_module file
PyMODINIT_FUNC init_module();

__declspec( dllexport ) void				LibInit()			{ init_module(); }
__declspec( dllexport ) const TCHAR*		LibDescription()	{ return _T( "Py3dsMax Python Extension" ); }
__declspec( dllexport ) ULONG				LibVersion()		{ return VERSION_3DSMAX; }