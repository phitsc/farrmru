#pragma once

//-----------------------------------------------------------------------

#include "UserDefinedGroups.h"

//-----------------------------------------------------------------------

class OptionsFile;

//-----------------------------------------------------------------------

struct Options
{
    Options(const OptionsFile& optionsFile);
    void update(const OptionsFile& optionsFile);

    // Filter
    bool removeUNCFiles;
    bool removeDirectories;
    bool simpleDirectoryCheck;
    bool removeNonExistingFiles;
    bool ignoreExistenceCheckUNCPaths;

    // Sorting
    enum SortMode
    {
        Sort_NoSorting = 0,
        Sort_TimeLastAccessed,
        Sort_TimeLastModified,
        Sort_TimeCreated,
        Sort_Alphabetically
    };
    SortMode sortMode;

    enum LbcFormatting
    {
        LbcFormatting_None = 0,
        LbcFormatting_Separate,
        LbcFormatting_Group
    };
    LbcFormatting lbcFormatting;


    // Display
    bool showGroupName;

    // other
    UserDefinedGroups userDefinedGroups;
};

//-----------------------------------------------------------------------
