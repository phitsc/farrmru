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
        const GroupNameToGroup::const_iterator it = _groupNameToGroup.find(groupName);
        if(it != _groupNameToGroup.end())
        {
            const Group& group = it->second;
            
            return group.description;
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
    struct Group
    {
        std::string description;
        std::string pathToIconFile;
        RegistryPaths registryPaths;
    };
    typedef std::map<std::string, Group> GroupNameToGroup;

    // add items to cache
    void addMenuItems();
    void addAvailablePrograms();
    void addMyRecentDocuments();
    void addMruApplications();
    void addUserDefinedGroups();

    static bool hasMRUList(const RegistryKey& registryKey);
    static bool usesSubkeys(const std::string& restKeyPath);
    void addGroup(const GroupNameToGroup::const_iterator& typeIterator, ItemList& itemList) const;
    void addMRUs(const std::string& groupName, const std::string& keyPath, ItemList& itemList) const;
    static void addMRUsFromRegistry(const std::string& groupName, HKEY baseKey, const std::string& restKeyPath, ItemList& itemList);
    static void addWithMRUList(const std::string& groupName, const RegistryKey& registryKey, ItemList& itemList);
    static void addWithItemNo(const std::string& groupName, const RegistryKey& registryKey, ItemList& itemList);
    static void addWithSubkey(const std::string& groupName, const RegistryKey& registryKey, const std::string& valueName, ItemList& itemList);
    void addMRUsFromFile(const std::string& groupName, const std::string& applicationKey, ItemList& itemList) const;

    void addMRUs_OpenOffice(const std::string& groupName, ItemList& itemList) const;
    void addMRUs_NotepadPlusPlus(const std::string& groupName, ItemList& itemList) const;

    void resolveLinks(ItemList& itemList);
    void resolveLink(Item& item);
    void updateFileTimes(ItemList& itemList) const;
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

    static void replacePathVariables(std::string& path);
    static void removeInvalidStuff(std::string& path);
    static void fixAdobePath(std::string& path);
    class IsSameCharacter
    {
    public:
        IsSameCharacter(char character) : _character(character)
        {}

        bool operator()(char character) const
        {
            return (character == _character);
        }

    private:
        char _character;
    };
    bool isInstalled(const Group& group) const;
    bool isInstalled(const std::string& registryPath) const;
    bool isInstalledFile(const std::string& applicationKey) const;
    bool isInstalledOpenOffice() const;
    bool isInstalledNotepadPlusPlus() const;
    static bool isInstalledRegistry(HKEY baseKey, const std::string& restKeyPath);

    // cache all available items (before sorting and filtering)
    ItemList    _itemCache;
    bool        _rebuildItemCache;

    enum MruMode
    {
        Mode_MyRecentDocuments,
        Mode_Programs,
        Mode_All,
        Mode_User,
        Mode_ListAvailablePrograms,
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

    GroupNameToGroup _groupNameToGroup;

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
