#pragma once

#include "OptionsFile.h"
#include "Options.h"

#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>

class RegistryKey;

//-----------------------------------------------------------------------

class FarrMostRecentlyUsedPlugin
{
public:

    // groupName, itemPath
    typedef std::pair<std::string, std::string> Item;
    typedef std::vector<Item> Items;

    FarrMostRecentlyUsedPlugin(const std::string& modulePath, IShellLink* shellLinkRawPtr);

    void search(const std::string& rawSearchString, const std::string& searchString);

    Items::size_type getItemCount() const;
    const Item& getItem(const Items::size_type& index) const;

    void showOptions();

private:
    typedef std::list<Item> ItemList;

    // from file system
    void addRecentDocuments(ItemList& itemList);

    // from registry
    void addMRUs(const std::string& groupName, const std::string& keyPath, ItemList& itemList);
    
    bool hasMRUList(const RegistryKey& registryKey) const;
    void addWithMRUList(const std::string& groupName, const RegistryKey& registryKey, ItemList& itemList);
    void addWithItemNo(const std::string& groupName, const RegistryKey& registryKey, ItemList& itemList);

    void resolveLink(std::string& path);

    typedef std::set<std::string> OrderedStringCollection;

    void sortItemsAlphabetically(ItemList& itemList);
    void sortItemsLastModified(ItemList& itemList);
    void resolveLinks(ItemList& itemList);
    void filterItems(ItemList& itemList, const std::string& searchString, const OrderedStringCollection& extensions);

    void extractOptionsAndExtensions(const std::string& rawSearchString, OrderedStringCollection& options, OrderedStringCollection& extensions) const;

    OrderedStringCollection _farrOptions;

    static void removeInvalidStuff(std::string& path);

    Items _items;

    OptionsFile _optionsFile;
    Options     _options;

    typedef std::vector<std::string> RegistryPaths;
    typedef std::pair<std::string, RegistryPaths> GroupNameAndRegistryPaths;
    typedef std::map<std::string, GroupNameAndRegistryPaths> TypeToGroupNameAndRegistryPaths;
    TypeToGroupNameAndRegistryPaths _typeToGroupNameAndRegistryPaths;

    CComPtr<IShellLink> _shellLink;
    CComPtr<IPersistFile> _shellLinkFile;
};

//-----------------------------------------------------------------------
