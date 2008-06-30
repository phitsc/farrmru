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
    ignoreUNCPaths = optionsFile.getValue("IgnoreUNCPaths", true);
    ignoreDirectories = optionsFile.getValue("IgnoreDirectories", true);
    sortMode = (SortMode)optionsFile.getValue("SortMode", (long)NoSorting);
}

//-----------------------------------------------------------------------
