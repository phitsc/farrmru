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

class CompareLastModified
{
public:
    bool operator()(const Item& leftItem, const Item& rightItem) const;

private:
    HANDLE openFile(const std::string& fileName) const;
};

//-----------------------------------------------------------------------

class CompareName
{
public:
    bool operator()(const Item& leftItem, const Item& rightItem) const;
};

//-----------------------------------------------------------------------

class NeedsToBeRemoved
{
public:
    typedef std::set<std::string> OrderedStringCollection;

    NeedsToBeRemoved(const Options& options, Options::SortMode sortMode, const OrderedStringCollection& extensions, const std::string& searchString);

    bool operator()(const Item& item) const;

private:
    bool doesntContainSearchstringIgnoringCase(const std::string& path) const;

    static bool isUNCPath(const std::string& path);
    bool fileExists(const Item& item) const;
    bool isDirectory(const Item& item) const;
    static void tolower(std::string& toConvert);

    const Options& _options;
    const Options::SortMode _sortMode;
    const OrderedStringCollection& _extensions;
    const std::string& _searchString;

    typedef std::set<std::string> Items;
    mutable Items _items;
};

//-----------------------------------------------------------------------
