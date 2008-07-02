#include "stdafx.h"
#include "SortingAndFiltering.h"
#include "Item.h"

//-----------------------------------------------------------------------

bool CompareLastModified::operator()(const Item& leftItem, const Item& rightItem) const
{
    HANDLE leftFile = openFile(leftItem.path);
    HANDLE rightFile = openFile(rightItem.path);
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
    return (_stricmp(PathFindFileName(leftItem.path.c_str()), PathFindFileName(rightItem.path.c_str())) < 0);
}

//-----------------------------------------------------------------------

NeedsToBeRemoved::NeedsToBeRemoved(const Options& options, Options::SortMode sortMode, const OrderedStringCollection& extensions, const std::string& searchString)
:_options(options), _sortMode(sortMode), _extensions(extensions), _searchString(searchString)
{}

//-----------------------------------------------------------------------

bool NeedsToBeRemoved::operator()(const Item& item) const
{
    // pass through web stuff
    if(item.type != Item::Type_URL)
    {
        if(!_options.includeUNCPaths && isUNCPath(item.path))
        {
            return true;
        }

        std::string extension = PathFindExtension(item.path.c_str());
        if(!_extensions.empty() && (_extensions.find(extension) == _extensions.end()))
        {
            return true;
        }

        if(_options.ignoreDirectories && isDirectory(item))
        {
            return true;
        }
    }

    if((_sortMode != Options::Sort_NoSorting) && !_searchString.empty() && doesntContainSearchstringIgnoringCase(item.path))
    {
        return true;
    }

    // check for duplicates
    std::string path = item.path;
    tolower(path);
    if(_items.find(path) != _items.end())
    {
        return true;
    }
    else
    {
        _items.insert(path);
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

bool NeedsToBeRemoved::isDirectory(const Item& item) const
{
    //return (item.type == Item::Type_Folder);

    if(_options.simpleDirectoryCheck && isUNCPath(item.path))
    {
        std::string::size_type pos = item.path.find_last_of(".\\/");
        if(pos != std::string::npos)
        {
            bool temp = (item.path[pos] != '.');
            if(temp)
            {
                OutputDebugString(item.path.c_str());
            }

            return temp;
        }
        else
        {
            OutputDebugString(item.path.c_str());

            return true;
        }
    }
    else
    {
        DWORD result = GetFileAttributes(item.path.c_str());

        if(result == INVALID_FILE_ATTRIBUTES)
        {
            OutputDebugString("Error");
        }

        bool temp = ((result & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);

            if(temp)
            {
                OutputDebugString(item.path.c_str());
            }

            return temp;
    }
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
