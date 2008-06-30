#pragma once

#include <string>
#include <locale>
#include <set>

#ifdef min
#undef min
#endif
#include <algorithm>

#include "Options.h"

typedef std::pair<std::string, std::string> Item;

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

    NeedsToBeRemoved(const Options& options, const OrderedStringCollection& extensions, const std::string& searchString)
        :_options(options), _extensions(extensions), _searchString(searchString)
    {}

    bool operator()(const Item& item) const;

private:
    bool doesntContainSearchstringIgnoringCase(const std::string& path) const;

    static bool isUNCPath(const std::string& path);
    static bool isDirectory(const std::string& path);
    static void tolower(std::string& toConvert);

    const Options& _options;
    const OrderedStringCollection& _extensions;
    const std::string& _searchString;
};

//-----------------------------------------------------------------------
