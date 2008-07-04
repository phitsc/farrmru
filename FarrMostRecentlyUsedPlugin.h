#pragma once

#include "OptionsFile.h"
#include "Options.h"
#include "Item.h"

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
    typedef std::vector<Item> Items;

    FarrMostRecentlyUsedPlugin(const std::string& modulePath, IShellLink* shellLinkRawPtr);

    void search(const char* rawSearchString);

    Items::size_type getItemCount() const;
    const Item& getItem(const Items::size_type& index) const;

    bool showGroupName() const
    {
        return _options.showGroupName;
    }

    bool noSorting() const
    {
        return (_sortModeCurrentSearch == Options::Sort_NoSorting);
    }

    void showOptions();

private:
    typedef std::list<Item> ItemList;
    typedef std::set<std::string> OrderedStringCollection;
    typedef std::vector<std::string> RegistryPaths;
    typedef std::pair<std::string, RegistryPaths> GroupNameAndRegistryPaths;
    typedef std::map<std::string, GroupNameAndRegistryPaths> TypeToGroupNameAndRegistryPaths;

    // from registry
    void addMRUs(const std::string& groupName, const std::string& keyPath, ItemList& itemList);
    
    //
    void addMenuItems(ItemList& itemList);
    void addMyRecentDocuments(ItemList& itemList, bool& needsSorting);
    void addMruApplications(ItemList& itemList, const OrderedStringCollection& types);

    bool hasMRUList(const RegistryKey& registryKey) const;
    void addWithMRUList(const std::string& groupName, const RegistryKey& registryKey, ItemList& itemList);
    void addWithItemNo(const std::string& groupName, const RegistryKey& registryKey, ItemList& itemList);
    void addType(const TypeToGroupNameAndRegistryPaths::const_iterator& typeIterator, ItemList& itemList);

    void resolveLink(Item& item);

    void sortItemsAlphabetically(ItemList& itemList);
    void sortItemsLastModified(ItemList& itemList);
    void resolveLinks(ItemList& itemList);
    void filterItems(ItemList& itemList, const std::string& searchString, const OrderedStringCollection& extensions);

    void extractSearchOptions(std::string& searchString, 
                              OrderedStringCollection& options,
                              OrderedStringCollection& types,
                              OrderedStringCollection& extensions) const;

    static void debugOutputResultList(const ItemList& itemList);

    OrderedStringCollection _farrOptions;
    OrderedStringCollection _mruOptions;

    static void removeInvalidStuff(std::string& path);

    Items _items;

    OptionsFile _optionsFile;
    Options     _options;
    
    Options::SortMode _sortModeCurrentSearch;

    TypeToGroupNameAndRegistryPaths _typeToGroupNameAndRegistryPaths;

    CComPtr<IShellLink> _shellLink;
    CComPtr<IPersistFile> _shellLinkFile;
};

//-----------------------------------------------------------------------
