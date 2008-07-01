===============================================================================

FARR MostRecentlyUsed plugin - 0.1.0 - 1. July 2008

(c) 2008 Philipp Tschannen

===============================================================================

Description
============
Shows the contents of the 'My Recent Documents' folder or a list of most recently used files of applications storing their most recently used files in the registry.


Installation
============
Copy all files into the FarrMostRecentlyUsed plugin directory (usually C:\Program Files\FindAndRunRobot\Plugins\FarrMostRecentlyUsed) and restart FARR.


Usage
============
Typing mru in FARR will give you a list of your most recently used files.

Use the following modifier keywords to change the search:
+byname : Force sorting by name
+bydate : Force sorting by date last modified
+.[ext] : Filter results by file extension [ext]. More than one can be used, e.g. +.h +.cpp

To show a list of most recently used files for selected applications, use the following modifers:
+msvc8  : Show most recently used MS Visual Studio 2005 files
+msvc9  : Show most recently used MS Visual Studio 2008 files
+office : Show most recently used Microsoft Office 2007 files
+wmp    : Show most recently used Windows Media Player files
+foxit  : Show most recently used Foxit Reader files


Options
============
- Ignore directories : Don't include directories
- Include UNC paths  : Include files starting with //
- Simplified directory check : Treat files names without a . as directories. This speeds up directory checking but can give false positives. Only applies to UNC paths.
- Show group name instead of path : Will show the group name instead of the path when listing most recently used files for selected applications.

Sorting:
- Do not sort : Let FARR do the sorting and filtering.
- By date last modifed : Sort by date last modified. For My Recent Documents, this uses the file link's date (not the target file's date). For selected applications, this uses the sort order as defined in the registry.
- Alphabetically : Sort the result alphabetically.


FarrMostRecentlyUsed.config
============
The FARR MostRecentlyUsed plugin can list most recently used files for all applications that store their most recently used files in the registry by listing these files in a certain application defined registry key.

The config file has the following format:
+modifier|Group Name
RegistryKeyToMRUList


Known issues
============
Clearing the 'Recent Items' list on Windows Vista will not make these files dissapear from the FARR MRU result list. The FARR MRU plugin uses the user's 'Recent' directory as its source. Windows Vista does not clear that directory when the user selects 'Clear Recent Items List' in the 'Recent Items' context menu. 


Credits
============
The icon is from FamFamFam's silk icon set: http://www.famfamfam.com/lab/icons/silk/


Disclaimer
============
This plugin is provided 'as is'. No warranty of any kind is expressed or implied. Use at your own risk.