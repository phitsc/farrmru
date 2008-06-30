#pragma once

//-----------------------------------------------------------------------

class OptionsFile;

//-----------------------------------------------------------------------

struct Options
{
    Options(const OptionsFile& optionsFile);
    void update(const OptionsFile& optionsFile);

    bool ignoreUNCPaths;
    bool ignoreDirectories;

    enum SortMode
    {
        NoSorting = 0,
        TimeLastModified,
        Alphabetically
    };
    SortMode sortMode;
};

//-----------------------------------------------------------------------
