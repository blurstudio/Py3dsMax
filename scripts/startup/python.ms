-- Initialize the Python 3dsMax bindings

-- Check for the Python DLL to be installed
if ( python != undefined ) then (
	local scriptPath = pathConfig.resolvePathSymbols "$scripts\\python"
	
	-- Add the $scripts/python path to the sys.path
	python.exec ( "import sys" )
	python.exec ( "if not '" + scriptPath + "' in sys.path: sys.path.append( '" + scriptPath + "' )" )
	
	-- Assign scriptPath as Py3dsMax installPath variable
	python.exec ( "import Py3dsMax" )
	python.exec ( "Py3dsMax.__dict__[ 'installPath' ] = '" + scriptPath + "/'" )
	
	-- Run any files in the $scripts/python/startup folder
	for file in getFiles( scriptPath + "/startup/*.py" ) do python.run( file )
	
	-- Launch the Python Logger
	python.run ( scriptPath + "/max_gui/LoggerWindow.py" )
)
else ( print "BlurPython has not been loaded." )