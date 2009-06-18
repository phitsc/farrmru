#include "stdafx.h"
#include "SortingAndFiltering.h"
#include "Item.h"

//-----------------------------------------------------------------------

CompareFiletime::CompareFiletime(FileTimeType fileTimeType)
:_fileTimeType(fileTimeType)
{
}

//-----------------------------------------------------------------------

bool CompareFiletime::operator()(const Item& leftItem, const Item& rightItem) const
{
    bool result = false;

    switch(_fileTimeType)
    {
    case LastAccessed:
        result = (CompareFileTime(&leftItem.lastAccessTime, &rightItem.lastAccessTime) > 0);
        break;

    case LastModifed:
        result = (CompareFileTime(&leftItem.lastModifiedTime, &rightItem.lastModifiedTime) > 0);
        break;

    case Created:
        result = (CompareFileTime(&leftItem.creationTime, &rightItem.creationTime) > 0);
        break;
    }

    return result;
}

//-----------------------------------------------------------------------

bool CompareName::operator ()(const Item& leftItem, const Item& rightItem) const
{
    return (_stricmp(PathFindFileName(leftItem.path.c_str()), PathFindFileName(rightItem.path.c_str())) < 0);
}

//-----------------------------------------------------------------------

bool IsFormatting::operator()(const Item& item) const
{
    return (item.type == Item::Type_Formatting);
}

//-----------------------------------------------------------------------

RemoveItemsStage1::RemoveItemsStage1(const Options& options)
:_options(options)
{
}

//-----------------------------------------------------------------------

bool RemoveItemsStage1::operator()(const Item& item) const
{
    // pass through web stuff and formatting
    if((item.type != Item::Type_URL) && (item.type != Item::Type_Formatting))
    {
        const bool isUNCFile = isUNCPath(item.path);

        if(_options.removeUNCFiles && isUNCFile)
        {
            return true;
        }

        if((isUNCFile && !_options.ignoreExistenceCheckUNCPaths) || (!isUNCFile && _options.removeNonExistingFiles))
        {
            if(!fileExists(item.path))
            {
                return true;
            }
        }

        if(_options.removeDirectories)
        {
            if(isUNCFile)
            {
                // use simple directory check, if existence check for UNC files is ignored
                if(_options.ignoreExistenceCheckUNCPaths || _options.simpleDirectoryCheck)
                {
                    if(isDirectory(item.path, true))
                    {
                        return true;
                    }
                }
                else
                {
                    if(isDirectory(item.path, false))
                    {
                        return true;
                    }
                }
            }
            else
            {
                if(isDirectory(item.path, false))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

//-----------------------------------------------------------------------

bool RemoveItemsStage1::isUNCPath(const std::string& path)
{
    bool isNetworkFile = (PathIsUNC(path.c_str()) == TRUE);
    return isNetworkFile;
}

//-----------------------------------------------------------------------

bool RemoveItemsStage1::fileExists(const std::string& path)
{
    return (PathFileExists(path.c_str()) == TRUE);
}

//-----------------------------------------------------------------------

bool RemoveItemsStage1::isDirectory(const std::string& path, bool simpleCheck)
{
    if(simpleCheck)
    {
        std::string::size_type pos = path.find_last_of(".\\/");
        if(pos != std::string::npos)
        {
            return (path[pos] != '.');
        }
        else
        {
            return true;
        }
    }
    else
    {
        DWORD result = GetFileAttributes(path.c_str());
        if(result != INVALID_FILE_ATTRIBUTES)
        {
            return ((result & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
        }
        else
        {
            return false;
        }
    }
}

//-----------------------------------------------------------------------

RemoveItemsStage2::RemoveItemsStage2(const OrderedStringCollection& extensions, const OrderedStringCollection& groups, const std::string& searchString, bool noSubstringFiltering)
:_extensions(extensions), _groups(groups), _searchString(searchString), _noSubstringFiltering(noSubstringFiltering)
{}

//-----------------------------------------------------------------------

bool RemoveItemsStage2::operator()(const Item& item) const
{
    if(!_groups.empty() && (_groups.find(item.groupName) == _groups.end()))
    {
        return true;
    }

    if(!_extensions.empty())
    {
        std::string extension = PathFindExtension(item.path.c_str());
        tolower(extension);

        if(_extensions.find(extension) == _extensions.end())
        {
            return true;
        }
    }

    if(!_noSubstringFiltering && !_searchString.empty() && doesntContainSearchstringIgnoringCase(item.path))
    {
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------

bool RemoveItemsStage2::doesntContainSearchstringIgnoringCase(const std::string& path) const
{
    std::string fileNameOnly = PathFindFileName(path.c_str());
    tolower(fileNameOnly);

    return (fileNameOnly.find(_searchString) == std::string::npos);
}

//-----------------------------------------------------------------------

void tolower(std::string& toConvert)
{
    std::locale loc;
    std::use_facet<std::ctype<char> >(loc).tolower(&toConvert[0], &toConvert[toConvert.length()]);
}

//-----------------------------------------------------------------------

bool RemoveDuplicates::operator()(const Item& item) const
{
    if(item.type != Item::Type_Formatting)
    {
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
    }

    return false;
}

//-----------------------------------------------------------------------
