#pragma once

//-----------------------------------------------------------------------

class OptionsFile;

//-----------------------------------------------------------------------

struct Options
{
    Options(const OptionsFile& optionsFile);
    void update(const OptionsFile& optionsFile);

    bool ignoreDirectories;
    bool includeUNCPaths;
    bool simpleDirectoryCheck;
    bool showGroupName;

    enum SortMode
    {
        Sort_NoSorting = 0,
        Sort_TimeLastModified,
        Sort_Alphabetically
    };
    SortMode sortMode;
};

//-----------------------------------------------------------------------
