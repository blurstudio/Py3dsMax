Here are the settings you'll need to compile the plugin for the different configurations,
if you add any other configurations (supporting more versions of Python for example, please
update this readme file with the required settings)

-----------------------------------------------------------------------------------

FOR MAX2010, MAX2011

1: Open the blurPython_2008 project with MSVC++ 2008

2: Setup Environment Variables

* MAX2010SDK	- /path/to/maxsdk/
* PYTHON24		- /path/to/python24/
* PYTHON26_64	- /path/to/python26/

3: Choose configuration

* Configuration				Platform
--------------------------------------
* Max2010_Python24			Win32		(works for both Max2010 and Max2011)
* Max2010x64_Python26		x64			(works for both Max2010 and Max2011)

4: Compile, output goes in [project]/bin/[platform]/[configuration]/

-----------------------------------------------------------------------------------

FOR MAX2012

1: Open the blurPython_2010 project with MSVC++ 2010

2: Setup Environment Variables

* MAX2012SDK	- /path/to/maxsdk/
* PYTHON24		- /path/to/python24/
* PYTHON26_64	- /path/to/python26/

3: Choose configuration

* Configuration				Platform
--------------------------------------
* Max2012_Python24			Win32
* Max2012x64_Python26		x64

4: Compile, output goes in [project]/bin/[platform]/[configuration]/

-----------------------------------------------------------------------------------

FOR MAX2015

1: Open the blurPython_2012 project with MSVC++ 2012

2: Setup Environment Variables

* MAX2015SDK	- /path/to/maxsdk/
* PYTHON27_64	- /path/to/python27/

3: Choose configuration

* Configuration				Platform
--------------------------------------
* Max2015x64_Python27		x64

4: Compile, output goes in [project]/bin/[platform]/[configuration]/