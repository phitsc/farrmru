#pragma once

#include <string>
#include <locale>
#include <set>

#ifdef min
#undef min
#endif
#include <algorithm>

#include "Options.h"

//-----------------------------------------------------------------------

struct Item;

//-----------------------------------------------------------------------

class CompareFiletime
{
public:
    enum FileTimeType
    {
        LastAccessed,
        LastModifed,
        Created
    };

    CompareFiletime(FileTimeType fileTimeType);

    bool operator()(const Item& leftItem, const Item& rightItem) const;

private:
    FileTimeType _fileTimeType;
};

//-----------------------------------------------------------------------

class CompareName
{
public:
    bool operator()(const Item& leftItem, const Item& rightItem) const;
};

//-----------------------------------------------------------------------

class IsFormatting
{
public:
    bool operator()(const Item& item) const;
};

//-----------------------------------------------------------------------

class RemoveItemsStage1
{
public:
    RemoveItemsStage1(const Options& options);

    bool operator()(const Item& item) const;

private:
    const Options& _options;

    static bool isUNCPath(const std::string& path);
    static bool fileExists(const std::string& path);
    static bool isDirectory(const std::string& path, bool simpleCheck);
};

//-----------------------------------------------------------------------

class RemoveItemsStage2
{
public:
    typedef std::set<std::string> OrderedStringCollection;

    RemoveItemsStage2(const OrderedStringCollection& extensions, const OrderedStringCollection& groups, const std::string& searchString, bool noSubstringFiltering);

    bool operator()(const Item& item) const;

private:
    bool doesntContainSearchstringIgnoringCase(const std::string& path) const;

    const OrderedStringCollection& _extensions;
    const OrderedStringCollection& _groups;
    const std::string&             _searchString;
    bool                           _noSubstringFiltering;
};

//-----------------------------------------------------------------------

class RemoveDuplicates
{
public:
    bool operator()(const Item& item) const;

private:
    typedef std::set<std::string> Items;
    mutable Items _items;
};

//-----------------------------------------------------------------------

void tolower(std::string& toConvert);

//-----------------------------------------------------------------------
