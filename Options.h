#pragma once

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

    // Display
    bool showGroupName;
};

//-----------------------------------------------------------------------
