#pragma once

#include <string>

//-----------------------------------------------------------------------

const FILETIME nullFileTime = { 0 };

struct Item
{
    enum Type
    {
        Type_File,
        Type_Folder,
        Type_URL,
        Type_Alias,
    };

    Item(const std::string& groupName_, const std::string& path_, Type type_, 
         const FILETIME& lastAccessTime_ = nullFileTime, const FILETIME& lastModifiedTime_ = nullFileTime, const FILETIME& creationTime_ = nullFileTime)
        :groupName(groupName_), path(path_), type(type_), 
         lastAccessTime(lastAccessTime_), lastModifiedTime(lastModifiedTime_), creationTime(creationTime_)
    {}

    std::string groupName;
    std::string path;
    Type        type;
    FILETIME    lastAccessTime;
    FILETIME    lastModifiedTime;
    FILETIME    creationTime;
};

//-----------------------------------------------------------------------
