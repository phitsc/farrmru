#include "stdafx.h"
#include "Options.h"
#include "OptionsFile.h"

//-----------------------------------------------------------------------

Options::Options(const OptionsFile& optionsFile)
{
    update(optionsFile);
}

//-----------------------------------------------------------------------

void Options::update(const OptionsFile& optionsFile)
{
    ignoreDirectories = optionsFile.getValue("IgnoreDirectories", true);
    includeUNCPaths = optionsFile.getValue("IncludeUNCPaths", true);
    simpleDirectoryCheck = optionsFile.getValue("SimpleDirectoryCheck", false);
    showGroupName = optionsFile.getValue("ShowGroupName", false);
    sortMode = (SortMode)optionsFile.getValue("SortMode", (long)Sort_NoSorting);
}

//-----------------------------------------------------------------------
