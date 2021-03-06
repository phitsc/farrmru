===============================================================================

FARR MostRecentlyUsed plugin - 1.0.1 - 14. May 2009

(c) 2009 Philipp Tschannen

===============================================================================

Description
=====================
Shows the contents of the 'My Recent Documents' folder or a list of most recently used files of applications storing their most recently used files in the registry.


Installation
=====================
Copy all files into the FarrMostRecentlyUsed plugin directory (usually C:\Program Files\FindAndRunRobot\Plugins\FarrMostRecentlyUsed) and restart FARR.


Usage
=====================
Use the following aliases to give you a list of your most recently used files:
- mru  : Shows a simple menu with the below aliases
- mrum : Shows files in your 'My Recent Documents' (XP) or 'Recent Items' (Vista, but see known issues) folder
- mrup : Shows files of configured programs, storing their MRU files in the registry
- mrua : Show a combined list of files out of mrum and mrup
- mruu : Show files from selected groups only (selectable in the options dialog)
- mrul : Show a list of installed and supported applications for further selection

Use the following modifier keywords to change the search:
+byname   : Force sorting by name
+bydate   : Force sorting by date last accessed
+bymod    : Force sorting by date last modified
+bycreate : Force sorting by date created
+.[ext]   : Filter results by file extension [ext]. More than one can be used, e.g. +.h +.cpp

To show a list of most recently used files for selected applications, use the following modifiers:
+acrobat  : Adobe Acrobat Reader (only 9 currently)
+akelpad  : Akelpad
+audacity : Audacity
+dwcs3	  : Dreamweaver CS3
+foxit    : Foxit Reader
+icofx    : IcoFX (only 1.6 currently)
+km       : KMPlayer
+msvc8    : MS Visual Studio 2005
+msvc9    : MS Visual Studio 2008
+npp      : Notepad++
+office3  : Microsoft Office 2003
+office7  : Microsoft Office 2007
+oo2      : OpenOffice 2.0
+paint    : Paint
+photoe   : Photoshop Elements (only 5.0 currently)
+snag     : SnagIt
+sql      : SQL Server
+ted      : TED Notepad
+winrar   : WinRAR
+winuae   : WinUAE
+wmp      : Windows Media Player
+wordpad  : Wordpad

(built-in group for My Recent Documents)
+recent  : My Recent Documents


Options
=====================
Filter
- UNC files : filter out UNC files (files starting with //) from the list
- Nonexistent files : filter out files that don't exist anymore
- Don't check existence of UNC files : what it says. faster.
- Directories : filter out directories
- Simplified directory check : Treat file names without a . as directories. This speeds up directory checking but can give false positives. Only applies to UNC paths. The simplified directory check method is always used for UNC files if 'Don't check existence of UNC files' is selected.

Sort:
- Do not sort : Let FARR do the sorting and filtering.
- By date last accessed : Sort by date last accessed. This is the default.
- By date last modified : Sort by date last modified.
- By date created : Sort by date created. This is here for completeness, don't know if it makes much sense.
- Alphabetically : Sort the result alphabetically.

Display
- Show group name instead of path : Will show the group name instead of the path when listing most recently used files for selected applications.


FarrMostRecentlyUsed.config / FarrMostRecentlyUsed.user
=====================
The FARR MostRecentlyUsed plugin can list most recently used files for many applications that store their most recently used files in the registry or in an .ini file.

The config file has the following format:
+modifier|Group Name|PathToIconFile (usually program.exe)
RegistryKeyToMRUList or PathToIniFilePlusSectionAndKey

RegistryKeyToMRUList typically looks like this:
HKEY_CURRENT_USER\Software\Application Name\Recent File List

PathToIniFilePlusSectionAndKey typically looks like this:
INI_FILE\PathToIniFile.ini|Section|BaseKeyName
within the .ini file you find something like this:
[Section]
BaseKeyName1=c:\blah\mru1.txt
BaseKeyName2=c:\blah\mru2.txt


FarrMostRecentlyUsed.user has the same format as FarrMostRecentlyUsed.config but will never be overwritten with new updates of the plugin. Put your own stuff in here. 

Modifier entries in FarrMostRecentlyUsed.user override identical ones in FarrMostRecentlyUsed.config. This can be used for example if you choose to install an application in a location other than the default (e.g. not in the default 'Program Files' folder). Just duplicate the respective section in your FarrMostRecentlyUsed.user and change the icon path.


Details on File Dates
=====================
1. For UNC files, no dates will be queried if 'Don't check existence of UNC files' is selected, so these will usually always be at the end of the list when sorting by any date (but see point 2 and 3).
2. For files coming from My Recent Documents (they are all .lnk files) the link's modification date will be used as the target files last accessed date. So even UNC files from 'My Recent Documents' will order correctly when sorting by date last accessed.
3. When filtering by one specific application (e.g. mrup +office7), and sorting by date last accessed, the file order as specified in the registry will be used instead of the files' last access dates. So the files appear in the same order as if you would select the recent files menu item of the respective application.


Known issues
=====================
Clearing the 'Recent Items' list on Windows Vista will not make these files disappear from the FARR MRU result list. The FARR MRU plugin uses the user's 'Recent' directory as its source. Windows Vista does not clear that directory when the user selects 'Clear Recent Items List' in the 'Recent Items' context menu. 


Credits
=====================
Using the TinyXml parser ( www.sourceforge.net/projects/tinyxml )

Special thanks to hamradio for the wonderful icons!


Disclaimer
=====================
This plugin is provided 'as is'. No warranty of any kind is expressed or implied. Use at your own risk.


Version history
=====================
1.0.1
- Updated .config file

1.0.0
- Added / Changed the following applications:
  - Added Visio 2007 to Office 2007 (+office7)

0.5.1
- Modifier entries in FarrMostRecentlyUsed.user now override identical ones in FarrMostRecentlyUsed.config.
- Added / Changed the following applications:
  - Added Akelpad (+akelpad) *
  - Added Audacity (+audacity) *
  - Added icon path for TED Notepad *
  - Added icon path for WinRAR *
  - Added icon path for WinUAE *

0.5.0
- Added support for mru items specified in .ini files
- Added new alias mrul which gives a list of supported and installed applications
- User defined result list option now only shows installed applications
- Added / Changed the following applications:
  - Added Dreamweaver CS3 (+dwcs3) *
  - Added IcoFX 1.6 (+icofx)
  - Changed +oo to +oo2 (in expectance of OpenOffice 3)

0.4.0
- Added icons to the simple menu (thanks hamradio!)
- Added support for the following applications:
  - Adobe Acrobat Reader 9 (+acrobat)
  - Notepad++ (+npp)
  - OpenOffice 2.0 (+oo)

0.3.0
- Added simple menu (type mru)
- Added special aliases:
  - mrum (My Recent Documents)
  - mrup (Recent Application documents)
  - mrua (All recent documents)
  - mruu (User selected groups of applications / My Recent Documents)
- Much improved filtering and sorting
- Show icons for UNC files
- Added FarrMostRecentlyUsed.user config file. Like FarrMostRecentlyUsed.config, but will never be overwritten.

0.2.0
- Added check for file existence for local files. UNC paths are currently never checked for existence.
- Added MRU specifications for:
  - Microsoft Office 2003 (+office3) *
  - Photoshop Elements (+photoe) *
  - KMPlayer (+km) *
  - SnagIt (+snag) *
  - SQL Server (+sql) *
  - Paint (+paint)
  - TED Notepad (+ted) *
  - WinRAR (+winrar) *
  - WinUAE (+winuae) *
  - Wordpad (+wordpad) *
- Renamed modifier for Microsoft Office 2007 to '+office7' 

* Note that I did not test these myself, please report if they don't work.

0.1.0
- Initial release