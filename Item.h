#pragma once

#include <string>

//-----------------------------------------------------------------------

struct Item
{
    enum Type
    {
        Type_File,
        Type_Folder,
        Type_URL,
    };

    Item(const std::string& groupName_, const std::string& path_, Type type_)
        :groupName(groupName_), path(path_), type(type_)
    {}

    std::string groupName;
    std::string path;
    Type type;
};

//-----------------------------------------------------------------------
