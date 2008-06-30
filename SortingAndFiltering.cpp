#include "stdafx.h"
#include "SortingAndFiltering.h"

//-----------------------------------------------------------------------

bool CompareLastModified::operator()(const Item& leftItem, const Item& rightItem) const
{
    HANDLE leftFile = openFile(leftItem.second);
    HANDLE rightFile = openFile(rightItem.second);
    if((leftFile != 0) && (rightFile != 0))
    {
        FILETIME leftModified = { 0 };
        bool leftOk = (GetFileTime(leftFile, 0, 0, &leftModified) == TRUE);

        FILETIME rightModified = { 0 };
        bool rightOk = (GetFileTime(rightFile, 0, 0, &rightModified) == TRUE);

        if(leftOk && rightOk)
        {
            return (CompareFileTime(&leftModified, &rightModified) >= 0);
        }

        CloseHandle(leftFile);
        CloseHandle(rightFile);
    }

    return false;
}

//-----------------------------------------------------------------------

HANDLE CompareLastModified::openFile(const std::string& fileName) const
{
    return CreateFile(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
}

//-----------------------------------------------------------------------

bool CompareName::operator ()(const Item& leftItem, const Item& rightItem) const
{
    return (_stricmp(PathFindFileName(leftItem.second.c_str()), PathFindFileName(rightItem.second.c_str())) < 0);
}

//-----------------------------------------------------------------------

bool NeedsToBeRemoved::operator()(const Item& item) const
{
    if(_options.ignoreUNCPaths && isUNCPath(item.second))
    {
        return true;
    }

    std::string extension = PathFindExtension(item.second.c_str());
    if(!_extensions.empty() && (_extensions.find(extension) == _extensions.end()))
    {
        return true;
    }

    if(_options.ignoreDirectories && isDirectory(item.second))
    {
        return true;
    }

    if(!_searchString.empty() && doesntContainSearchstringIgnoringCase(item.second))
    {
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------

bool NeedsToBeRemoved::isUNCPath(const std::string& path)
{
    bool isNetworkFile = (PathIsUNC(path.c_str()) == TRUE);
    return isNetworkFile;
}

//-----------------------------------------------------------------------

bool NeedsToBeRemoved::isDirectory(const std::string& path)
{
    bool isDirectory = ((GetFileAttributes(path.c_str()) & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
    return isDirectory;
}

//-----------------------------------------------------------------------

bool NeedsToBeRemoved::doesntContainSearchstringIgnoringCase(const std::string& path) const
{
    std::string fileNameOnly = PathFindFileName(path.c_str());
    tolower(fileNameOnly);

    return (fileNameOnly.find(_searchString) == std::string::npos);
}

//-----------------------------------------------------------------------

void NeedsToBeRemoved::tolower(std::string& toConvert)
{
    std::locale loc;
    std::use_facet<std::ctype<char> >(loc).tolower(&toConvert[0], &toConvert[toConvert.length()]);
}

//-----------------------------------------------------------------------
