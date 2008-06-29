#pragma once

#include "OptionsFile.h"

#include <string>
#include <vector>
#include <map>
#include <list>

class RegistryKey;

//-----------------------------------------------------------------------

struct Options
{
    Options(const OptionsFile& optionsFile);
    void update(const OptionsFile& optionsFile);

    bool ignoreNetworkFiles;
    bool ignoreDirectories;
};

//-----------------------------------------------------------------------

class FarrMostRecentlyUsedPlugin
{
    typedef std::vector<std::string> Items;

public:
    FarrMostRecentlyUsedPlugin(const std::string& modulePath, IShellLink* shellLinkRawPtr);

    void search(const std::string& rawSearchString, const std::string& searchString);

    Items::size_type getItemCount() const;
    const std::string& getItem(const Items::size_type& index) const;

    void showOptions();

private:
    typedef std::list<std::string> ItemList;

    // from file system
    void addRecentDocuments(ItemList& itemList);

    // from registry
    void addMRUs(const std::string& keyPath, ItemList& itemList);
    
    bool hasMRUList(const RegistryKey& registryKey) const;
    void addWithMRUList(const RegistryKey& registryKey, ItemList& itemList);
    
    void addWithItemNo(const RegistryKey& registryKey, ItemList& itemList);

    void resolveLink(std::string& path);

    void sortItems(ItemList& itemList);
    void resolveLinks(ItemList& itemList);
    void filterItems(ItemList& itemList, const std::string& searchString);

    typedef std::vector<std::string> OptionStrings;
    static OptionStrings extractOptionStrings(const std::string& rawSearchString);

    static void removeInvalidStuff(std::string& path);

    static bool isNetworkFile(const std::string& path);
    static bool isDirectory(const std::string& path);

    Items _items;

    OptionsFile _optionsFile;
    Options     _options;

    typedef std::vector<std::string> RegistryPaths;
    typedef std::map<std::string, RegistryPaths> TypeToRegistryPaths;
    TypeToRegistryPaths _typeToRegistryPaths;

    CComPtr<IShellLink> _shellLink;
};

//-----------------------------------------------------------------------
