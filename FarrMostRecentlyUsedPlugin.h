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
    FarrMostRecentlyUsedPlugin(const std::string& modulePath, IShellLink* shellLinkRawPtr);

    void search(const char* rawSearchString);

    typedef std::vector<Item> ItemVector;
    ItemVector::size_type getItemCount() const;
    const Item& getItem(const ItemVector::size_type& index) const;

    bool showGroupName() const
    {
        return _options.showGroupName;
    }

    bool noSorting() const
    {
        return (_sortModeCurrentSearch == Options::Sort_NoSorting);
    }

    const std::string getGroupDescription(const std::string& groupName) const
    {
        const GroupNameToDescriptionAndRegistryPaths::const_iterator it = _groupNameToDescriptionAndRegistryPaths.find(groupName);
        if(it != _groupNameToDescriptionAndRegistryPaths.end())
        {
            const GroupDescriptionAndRegistryPaths& groupDescriptionAndRegistryPaths = it->second;
            
            return groupDescriptionAndRegistryPaths.first;
        }
        else
        {
            return "";
        }
    }

    void showOptions();

private:
    typedef std::list<Item> ItemList;
    typedef std::set<std::string> OrderedStringCollection;
    typedef std::vector<std::string> RegistryPaths;
    typedef std::pair<std::string, RegistryPaths> GroupDescriptionAndRegistryPaths;
    typedef std::map<std::string, GroupDescriptionAndRegistryPaths> GroupNameToDescriptionAndRegistryPaths;

    // add items to cache
    void addMenuItems();
    void addMyRecentDocuments();
    void addMruApplications();

    static bool hasMRUList(const RegistryKey& registryKey);
    static void addMRUs(const std::string& groupName, const std::string& keyPath, ItemList& itemList);
    static void addWithMRUList(const std::string& groupName, const RegistryKey& registryKey, ItemList& itemList);
    static void addWithItemNo(const std::string& groupName, const RegistryKey& registryKey, ItemList& itemList);
    static void addType(const GroupNameToDescriptionAndRegistryPaths::const_iterator& typeIterator, ItemList& itemList);

    void resolveLinks(ItemList& itemList);
    void resolveLink(Item& item);
    void updateFileTimes(ItemList& itemList);
    static void updateFileTime(Item& item);
    static bool isFileTimeNull(const FILETIME& fileTime);

    // sort items
    void sortItems(ItemList& itemList);
    static void sortItemsAlphabetically(ItemList& itemList);
    static void sortItemsByFileTime(ItemList& itemList, int fileTimeType);
    

    void extractSearchOptions(std::string& searchString, 
                              OrderedStringCollection& options,
                              OrderedStringCollection& groups,
                              OrderedStringCollection& extensions) const;

    // debugging functions
    static void debugOutputResultList(const ItemList& itemList);
    static void debugOutputNumber(const char* comment, long number);

    //
    OrderedStringCollection _farrOptions;
    OrderedStringCollection _mruOptions;

    static void removeInvalidStuff(std::string& path);

    // cache all available items (before sorting and filtering)
    ItemList    _itemCache;
    bool        _rebuildItemCache;

    enum MruMode
    {
        Mode_MyRecentDocuments,
        Mode_Programs,
        Mode_All,
        Mode_User,
        Mode_Menu,
        Mode_None,
    };
    // rebuild item cache if one these change
    MruMode     _currentMruMode;

    // item vector for item retrieval by FARR 
    ItemVector  _itemVector;

    OptionsFile _optionsFile;
    Options     _options;
    
    Options::SortMode _sortModeCurrentSearch;

    GroupNameToDescriptionAndRegistryPaths _groupNameToDescriptionAndRegistryPaths;

    CComPtr<IShellLink> _shellLink;
    CComPtr<IPersistFile> _shellLinkFile;
};

//-----------------------------------------------------------------------
