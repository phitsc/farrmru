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
    removeUNCFiles = optionsFile.getValue("RemoveUNCFiles", false);
    removeDirectories = optionsFile.getValue("RemoveDirectories", true);
    simpleDirectoryCheck = optionsFile.getValue("SimpleDirectoryCheck", true);
    removeNonExistingFiles = optionsFile.getValue("RemoveNonexistentFiles", true);
    ignoreExistenceCheckUNCPaths = optionsFile.getValue("IgnoreExistenceCheckUNCFiles", true);

    sortMode = (SortMode)optionsFile.getValue("SortMode", (long)Sort_TimeLastAccessed);

    showGroupName = optionsFile.getValue("ShowGroupName", false);

    userDefinedGroups = optionsFile.getValue("UserDefinedGroups", "");
}

//-----------------------------------------------------------------------
