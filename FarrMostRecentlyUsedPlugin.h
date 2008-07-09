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
    typedef std::set<std::string> RegistryPaths;
    typedef std::pair<std::string, RegistryPaths> GroupDescriptionAndRegistryPaths;
    typedef std::map<std::string, GroupDescriptionAndRegistryPaths> GroupNameToDescriptionAndRegistryPaths;

    // add items to cache
    void addMenuItems();
    void addMyRecentDocuments();
    void addMruApplications();
    void addUserDefinedGroups();

    static bool hasMRUList(const RegistryKey& registryKey);
    void addGroup(const GroupNameToDescriptionAndRegistryPaths::const_iterator& typeIterator, ItemList& itemList) const;
    void addMRUs(const std::string& groupName, const std::string& keyPath, ItemList& itemList) const;
    static void addMRUsFromRegistry(const std::string& groupName, HKEY baseKey, const std::string& restKeyPath, ItemList& itemList);
    static void addWithMRUList(const std::string& groupName, const RegistryKey& registryKey, ItemList& itemList);
    static void addWithItemNo(const std::string& groupName, const RegistryKey& registryKey, ItemList& itemList);
    void addMRUsFromFile(const std::string& groupName, const std::string& applicationKey, ItemList& itemList) const;

    void addMRUs_OpenOffice(const std::string& groupName, ItemList& itemList) const;
    void addMRUs_NotepadPlusPlus(const std::string& groupName, ItemList& itemList) const;

    void resolveLinks(ItemList& itemList);
    void resolveLink(Item& item);
    void updateFileTimes(ItemList& itemList);
    static void updateFileTime(Item& item);
    static bool isFileTimeNull(const FILETIME& fileTime);

    // sort items
    void sortItems(ItemList& itemList);
    static void sortItemsAlphabetically(ItemList& itemList);
    static void sortItemsByFileTime(ItemList& itemList, int fileTimeType);

    // initialisation
    void processConfigFile(const std::string& configFileName);

    //
    void extractSearchOptions(std::string& searchString, 
                              OrderedStringCollection& options,
                              OrderedStringCollection& groups,
                              OrderedStringCollection& extensions) const;
    void handleForceSortMode(OrderedStringCollection& options);

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

    CComPtr<IShellLink>   _shellLink;
    CComPtr<IPersistFile> _shellLinkFile;

    std::string _recentFolder;
    std::string _appDataFolder;

    // debugging functions
    enum DebugLevel
    {
        Debug_Level0,
        Debug_Level1,
        Debug_Level2,
    };
    DebugLevel _debugLevel;

    void setDebugLevel(const OrderedStringCollection& options);

    void debugOutputMessage(DebugLevel debugLevel, const char* message) const;
    void debugOutputNumber(DebugLevel debugLevel, const char* message, long number) const;
    void debugOutputResultList(DebugLevel debugLevel, const ItemList& itemList) const;
};

//-----------------------------------------------------------------------
