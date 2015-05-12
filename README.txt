Py3dsMax allows two way access between the maxscript and python interpreters. In maxscript you can import python modules and classes create objects set values etc. In python you can access maxscript commands get and set values. 

Some non-pythonic functionality does not translate to python like the by-reference character "&". To work around these limitations you can generally define a helper maxscript struct like the pyhelper class.

# Compile Settings

There are several environment variables you will need to set to be able to compile. Most of them contain the year of the max version you are trying to comile for. For example to compile for Max 2015 you need to create a environment variable "MAX2015SDK" pointing to the maxsdk folder of the sdk(C:\Program Files\Autodesk\3ds Max 2015 SDK\maxsdk).

Required Environment variables:
1. MAX[year]SDK: The Max SDK's location. should point to the maxsdk folder. year is the year number of the max build you are trying to build.
2. PYTHON[ver][_64]: The folder containing python.exe. ver is the python version without a decimal. _64 should be specified for 64bit builds, and should be omitted for 32bit builds.
3. MAX[Year]OUT: If defined as a post-build process the .dlx file will be copied to this folder. This variable is optional, and is only supported in Max 2015 and newer.

Here is a breakdown of the Visual Studio requirements required to build this package. If a binary compatible version is listed you can use the same plugin compiled for that version.

|  | 2012 | 2013 | 2014 | 2015 | 2016 |
|------------------------|------------------------------------------------|---------------------------------------------------|----------------------|-----------------------------|-----------------------------|
| Visual Studio | 2008 Service Pack 1 | 2010 Service Pack 1 | 2010 Service Pack 1 | Visual Studio 2012 Update 4 | Visual Studio 2012 Update 4 |
| Binary Compatible |  |  |  |  | 2015 |
| 32bit Python Version | PYTHON24 | PYTHON27 | Not Supported | Not Supported | Not Supported |
| 64bit Python Version | PYTHON26_64 PYTHON27_64 | PYTHON27_64 | PYTHON27_64 | PYTHON27_64 | PYTHON27_64 |
| Configuration/Platform | Max2012_Python24/Win32 Max2012x64_Python26/x64 | Max2013x32_Python27/Win32 Max2013x64_Python27/x64 | Max2014_Python27/x64 | Max2015x64_Python27/x64 | Max2015x64_Python27/x64 |

Compiled output goes in [project]/bin/[platform]/Max[year]_Python[ver]/ or for older versions it may go into [project]/bin/[platform]/[configuration]/
