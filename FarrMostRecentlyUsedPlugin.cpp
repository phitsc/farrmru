#include "stdafx.h"
#include "FarrMostRecentlyUsedPlugin.h"

#include "OptionsDialog.h"
#include "OptionsFile.h"
#include "RegistryKey.h"
#include "FileList.h"
#include "SortingAndFiltering.h"

#include <algorithm>
#include <sstream>
#include <fstream>
#include <map>

#include "tinyxml/tinyxml.h"

//-----------------------------------------------------------------------

const char* Option_ByName   = "byname";
const char* Option_ByDate   = "bydate";
const char* Option_ByMod    = "bymod";
const char* Option_ByCreate = "bycreate";

const char* Option_DebugLevel0 = "debug0!";
const char* Option_DebugLevel1 = "debug1!";
const char* Option_DebugLevel2 = "debug2!";

const char* MyRecentDocuments_GroupName = "recent";

//-----------------------------------------------------------------------

FarrMostRecentlyUsedPlugin::FarrMostRecentlyUsedPlugin(const std::string& modulePath, IShellLink* shellLinkRawPtr) :
_rebuildItemCache(true),
_currentMruMode(Mode_None),
_optionsFile(modulePath + "\\FarrMostRecentlyUsed.ini"),
_options(_optionsFile),
_shellLink(shellLinkRawPtr),
_sortModeCurrentSearch(Options::Sort_NoSorting),
_debugLevel(Debug_Level0)
{
    _shellLink.QueryInterface(&_shellLinkFile);

    char recentFolder[MAX_PATH] = { 0 };
    if(S_OK == SHGetFolderPath(0, CSIDL_RECENT, 0, SHGFP_TYPE_CURRENT, recentFolder))
    {
        _recentFolder = recentFolder;
    }

    char appDataFolder[MAX_PATH] = { 0 };
    if(S_OK == SHGetFolderPath(0, CSIDL_APPDATA, 0, SHGFP_TYPE_CURRENT, appDataFolder))
    {
        _appDataFolder = appDataFolder;
    }

    // ignore these, they are used by farr
    _farrOptions.insert("sall");
    _farrOptions.insert("alias");

    _mruOptions.insert(Option_ByName);
    _mruOptions.insert(Option_ByDate);
    _mruOptions.insert(Option_ByMod);
    _mruOptions.insert(Option_ByCreate);
    
    _mruOptions.insert(Option_DebugLevel0);
    _mruOptions.insert(Option_DebugLevel1);
    _mruOptions.insert(Option_DebugLevel2);

    //
    const std::string configFileName = modulePath + "\\FarrMostRecentlyUsed.config";

    // save old .config file
    const std::string oldConfigFileName = configFileName + ".old";
    if(CopyFile(configFileName.c_str(), oldConfigFileName.c_str(), TRUE) == FALSE)
    {
        // we're probably on Vista and can't write into the Program Files folder, copy to temp instead
        char tempPath[MAX_PATH];
        GetTempPath(MAX_PATH, tempPath);
        PathAppend(tempPath, "FarrMostRecentlyUsed.config.old");
        if(CopyFile(configFileName.c_str(), tempPath, TRUE))
        {
            OutputDebugString(tempPath);
        }
    }

    processConfigFile(configFileName);

    const std::string userConfigFileName = modulePath + "\\FarrMostRecentlyUsed.user";
    processConfigFile(userConfigFileName);
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::processConfigFile(const std::string& configFileName)
{
    std::ifstream mruSpecificationsFile(configFileName.c_str());
    if(mruSpecificationsFile.is_open())
    {
        std::string currentGroupName;

        while(!mruSpecificationsFile.eof())
        {
            const unsigned long MaxLineLength = 350;
            char lineBuffer[MaxLineLength] = { 0 };
            mruSpecificationsFile.getline(lineBuffer, MaxLineLength);

            std::string line(lineBuffer);
            if(!line.empty() && (line[0] != '#'))
            {
                if(line[0] == '+')
                {
                    // remove +
                    line.erase(0, 1);

                    std::string groupDescription;
                    std::string pathToIconFile;

                    // extract group description, if present
                    std::string::size_type pos = line.find('|');
                    if(pos != std::string::npos)
                    {
                        currentGroupName = line.substr(0, pos);

                        // and possibly path to icon file
                        std::string::size_type pos2 = line.find('|', pos + 1);
                        if(pos2 != std::string::npos)
                        {
                            groupDescription = line.substr(pos + 1, pos2 - pos - 1);
                            pathToIconFile = line.substr(pos2 + 1);

                            replacePathVariables(pathToIconFile);
                        }
                        else
                        {
                            groupDescription = line.substr(pos + 1);
                        }
                    }
                    else
                    {
                        currentGroupName = line;
                    }

                    tolower(currentGroupName);
                    if(currentGroupName == MyRecentDocuments_GroupName)
                    {
                        currentGroupName.clear();
                    }

                    // adds entry
                    if(!currentGroupName.empty())
                    {
                        Group& group = _groupNameToGroup[currentGroupName];
                        group.description = groupDescription;
                        group.pathToIconFile = pathToIconFile;
                    }
                }
                else
                {
                    if(!currentGroupName.empty())
                    {
                        Group& group = _groupNameToGroup[currentGroupName];
                        RegistryPaths& registryPaths = group.registryPaths;

                        registryPaths.insert(line);
                    }
                }
            }
        }
    }
}

//-----------------------------------------------------------------------

struct PathVariableToFolder
{
    const char* pathVariable;
    int         folder;
};

const PathVariableToFolder PathVariablesToFolders[] =
{
    { "%PROGRAMFILES%", CSIDL_PROGRAM_FILES },
    { "%SYSTEMDIR%", CSIDL_SYSTEM }
};

const unsigned long PathVariablesToFoldersCount = sizeof(PathVariablesToFolders) / sizeof(PathVariablesToFolders[0]);

void FarrMostRecentlyUsedPlugin::replacePathVariables(std::string& path)
{
    for(unsigned long index = 0; index < PathVariablesToFoldersCount; ++index)
    {
        const std::string::size_type pos = path.find(PathVariablesToFolders[index].pathVariable);
        if(pos != std::string::npos)
        {
            char folder[MAX_PATH] = { 0 };
            if(S_OK == SHGetFolderPath(0, PathVariablesToFolders[index].folder, 0, SHGFP_TYPE_CURRENT, folder))
            {
                path.replace(pos, strlen(PathVariablesToFolders[index].pathVariable), folder);
            }
        }
    }
}

//-----------------------------------------------------------------------

const char* MruAliases[] = {
    "mrum",
    "mrup",
    "mrua",
    "mruu",
    "mrul",
    "mru"
};
const unsigned long MruAliasesCount = sizeof(MruAliases) / sizeof(MruAliases[0]);

void FarrMostRecentlyUsedPlugin::search(const char* rawSearchString)
{
    std::string searchString(rawSearchString);
    tolower(searchString);

    MruMode mruMode = Mode_None;
    for(unsigned long aliasIndex = 0; aliasIndex < MruAliasesCount; ++aliasIndex)
    {
        std::string alias(MruAliases[aliasIndex]);
        if(_strnicmp(searchString.c_str(), alias.c_str(), alias.length()) == 0)
        {
            mruMode = (MruMode)aliasIndex;
            searchString.erase(0, alias.length() + 1 /* also remove the space, if there is one */);
            break;
        }
    }

    if(mruMode != Mode_None)
    {
        if(mruMode != _currentMruMode)
        {
            _currentMruMode = mruMode;
            _rebuildItemCache = true;
        }

        OrderedStringCollection options;
        OrderedStringCollection groups;
        OrderedStringCollection extensions;
        extractSearchOptions(searchString, options, groups, extensions);

        setDebugLevel(options);

        _sortModeCurrentSearch = _options.sortMode;
        handleForceSortMode(options);

        if(_rebuildItemCache)
        {
            _itemCache.clear();

            debugOutputNumber(Debug_Level1, "Mode", mruMode);

            switch(mruMode)
            {
            case Mode_None:
                break;

            case Mode_MyRecentDocuments:
                addMyRecentDocuments();
                break;

            case Mode_Programs:
                addMruApplications();
                break;

            case Mode_All:
                addMyRecentDocuments();
                addMruApplications();
                break;

            case Mode_User:
                addUserDefinedGroups();
                break;

            case Mode_ListAvailablePrograms:
                addAvailablePrograms();
                break;

            case Mode_Menu:
                addMenuItems();
                break;
            }

            debugOutputNumber(Debug_Level1, "Updated item cache. Item count", _itemCache.size());
            debugOutputResultList(Debug_Level2, _itemCache);

            switch(mruMode)
            {
            case Mode_MyRecentDocuments:
            case Mode_Programs:
            case Mode_All:
            case Mode_User:
                {
                    RemoveItemsStage1 removeItemsStage1(_options);
                    _itemCache.remove_if(removeItemsStage1);

                    debugOutputNumber(Debug_Level1, "After filter stage 1. Item count", _itemCache.size());
                    debugOutputResultList(Debug_Level2, _itemCache);

                    updateFileTimes(_itemCache);
                }
                break;
            }

            _rebuildItemCache = false;
        }

        ItemList itemList(_itemCache);

        switch(mruMode)
        {
        case Mode_MyRecentDocuments:
        case Mode_Programs:
        case Mode_All:
        case Mode_User:
            {
                const bool noSubstringFiltering = (_options.sortMode == Options::Sort_NoSorting);   // FARR will do the filtering
                RemoveItemsStage2 removeItemsStage2(extensions, groups, searchString, noSubstringFiltering);
                itemList.remove_if(removeItemsStage2);

                debugOutputNumber(Debug_Level1, "After filter stage 2. Item count", itemList.size());
                debugOutputResultList(Debug_Level2, itemList);

                const bool noSorting = ((mruMode == Mode_Programs) && (groups.size() == 1)) || (_sortModeCurrentSearch == Options::Sort_NoSorting);
                if(!noSorting)
                {
                    sortItems(itemList);

                    debugOutputNumber(Debug_Level1, "After sorting. Item count", itemList.size());
                    debugOutputResultList(Debug_Level2, itemList);
                }

                RemoveDuplicates removeDuplicates;
                itemList.remove_if(removeDuplicates);

                debugOutputNumber(Debug_Level1, "After filter stage 3. Item count", itemList.size());
                debugOutputResultList(Debug_Level2, itemList);
            }
        }

        _itemVector.assign(itemList.begin(), itemList.end());
    }
    else
    {
        _itemVector.clear();
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::handleForceSortMode(OrderedStringCollection& options)
{
    if(options.find(Option_ByName) != options.end())
    {
        _sortModeCurrentSearch = Options::Sort_Alphabetically;
    }
    else if(options.find(Option_ByDate) != options.end())
    {
        _sortModeCurrentSearch = Options::Sort_TimeLastAccessed;
    }
    else if(options.find(Option_ByMod) != options.end())
    {
        _sortModeCurrentSearch = Options::Sort_TimeLastModified;
    }
    else if(options.find(Option_ByCreate) != options.end())
    {
        _sortModeCurrentSearch = Options::Sort_TimeCreated;
    }

    options.erase(Option_ByName);
    options.erase(Option_ByDate);
    options.erase(Option_ByMod);
    options.erase(Option_ByCreate);
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addMenuItems()
{
    _itemCache.push_back(Item("Alias mrum|FarrMRU_MyRecentDocuments.ico", "List only 'My Recent Documents'|restartsearch mrum", Item::Type_Alias));
    _itemCache.push_back(Item("Alias mrup|FarrMRU_ConfiguredApps.ico", "List most recently used items of configured programs|restartsearch mrup", Item::Type_Alias));
    _itemCache.push_back(Item("Alias mrua|FarrMRU_All.ico", "List all most recently used items|restartsearch mrua", Item::Type_Alias));
    _itemCache.push_back(Item("Alias mruu|FarrMRU_UserDefined.ico", "List user defined most recently used items|restartsearch mruu", Item::Type_Alias));
    _itemCache.push_back(Item("Alias mrul|FarrMRU_ListInstalledApps.ico", "List all supported and installed applications|restartsearch mrul", Item::Type_Alias));
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addAvailablePrograms()
{
    GroupNameToGroup::const_iterator groupNameIterator = _groupNameToGroup.begin();
    const GroupNameToGroup::const_iterator groupNamesEnd = _groupNameToGroup.end();

    for( ; groupNameIterator != groupNamesEnd; ++groupNameIterator)
    {
        const Group& group = groupNameIterator->second;

        if(isInstalled(group))
        {
            const Group& group = groupNameIterator->second;

            std::string groupName = "Alias mrup +" + groupNameIterator->first;
            if(!group.pathToIconFile.empty())
            {
                groupName += "|" + group.pathToIconFile;
            }
            const std::string path = "List mru items of " + group.description + "|restartsearch mrup +" + groupNameIterator->first;
            _itemCache.push_back(Item(groupName, path, Item::Type_Alias));
        }
    }
}

//-----------------------------------------------------------------------

bool FarrMostRecentlyUsedPlugin::isInstalled(const Group& group) const
{
    const RegistryPaths& registryPaths = group.registryPaths;

    RegistryPaths::const_iterator registryPathIterator = registryPaths.begin();
    const RegistryPaths::const_iterator registryPathsEnd = registryPaths.end();
    for( ; registryPathIterator != registryPathsEnd; ++registryPathIterator)
    {
        const std::string& registryPath = *registryPathIterator;

        if(isInstalled(registryPath))
        {
            return true;
        }
    }

    return false;
}

//-----------------------------------------------------------------------

bool FarrMostRecentlyUsedPlugin::isInstalled(const std::string& keyPath) const
{
    std::string::size_type backslashPos = keyPath.find('\\');
    if(backslashPos != std::string::npos)
    {
        const std::string baseKeyName = keyPath.substr(0, backslashPos);
        const std::string restKey = keyPath.substr(backslashPos + 1);

        if(baseKeyName == "HKEY_CURRENT_USER")
        {
            return isInstalledRegistry(HKEY_CURRENT_USER, restKey);
        }
        else if(baseKeyName == "HKEY_LOCAL_MACHINE")
        {
            return isInstalledRegistry(HKEY_LOCAL_MACHINE, restKey);
        }
        else if(baseKeyName == "HKEY_USERS")
        {
            return isInstalledRegistry(HKEY_USERS, restKey);
        }
        else if(baseKeyName == "FARR_MRU")
        {
            return isInstalledFile(restKey);
        }
    }

    return false;
}

//-----------------------------------------------------------------------

bool FarrMostRecentlyUsedPlugin::isInstalledRegistry(HKEY baseKey, const std::string& restKeyPath)
{
    std::string::size_type pipePos = restKeyPath.rfind('|');

    const std::string subKey = restKeyPath.substr(0, pipePos);

    RegistryKey key;
    return key.open(baseKey, subKey.c_str(), KEY_READ);
}

//-----------------------------------------------------------------------

bool FarrMostRecentlyUsedPlugin::isInstalledFile(const std::string& applicationKey) const
{
    if(applicationKey == "openoffice")
    {
        return isInstalledOpenOffice();
    }
    else if(applicationKey == "npp")
    {
        return isInstalledNotepadPlusPlus();
    }
    else
    {
        return false;
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addMyRecentDocuments()
{
    if(!_recentFolder.empty())
    {
        debugOutputMessage(Debug_Level1, _recentFolder.c_str());

        const FileList fileList(_recentFolder, "*.*", FileList::Files);

        FileList::const_iterator it = fileList.begin();
        const FileList::const_iterator end = fileList.end();

        for( ; it != end; ++it)
        {
            const FileList::File& file = *it;

            // use links last modified time as target files last access time
            _itemCache.push_back(Item(MyRecentDocuments_GroupName, file.path, (file.type == FileList::File::Type_Directory) ? Item::Type_Folder : Item::Type_File, 
                                      file.lastModifiedTime));
        }

        resolveLinks(_itemCache);

        //debugOutputResultList(_itemCache);
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addMruApplications()
{
    GroupNameToGroup::const_iterator groupNameIterator = _groupNameToGroup.begin();
    const GroupNameToGroup::const_iterator groupNamesEnd = _groupNameToGroup.end();

    for( ; groupNameIterator != groupNamesEnd; ++groupNameIterator)
    {
        addGroup(groupNameIterator, _itemCache);
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addUserDefinedGroups()
{
    if(_options.userDefinedGroups.contains(MyRecentDocuments_GroupName))
    {
        addMyRecentDocuments();
    }

    GroupNameToGroup::const_iterator groupNameIterator = _groupNameToGroup.begin();
    const GroupNameToGroup::const_iterator groupNamesEnd = _groupNameToGroup.end();

    for( ; groupNameIterator != groupNamesEnd; ++groupNameIterator)
    {
        const std::string& groupName = groupNameIterator->first;
        if(_options.userDefinedGroups.contains(groupName))
        {
            addGroup(groupNameIterator, _itemCache);
        }
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addGroup(const GroupNameToGroup::const_iterator& typeIterator, ItemList& itemList) const
{
    const std::string& groupName = typeIterator->first;
    const Group& group = typeIterator->second;
    const RegistryPaths& registryPaths = group.registryPaths;

    RegistryPaths::const_iterator registryPathIterator = registryPaths.begin();
    const RegistryPaths::const_iterator registryPathsEnd = registryPaths.end();
    for( ; registryPathIterator != registryPathsEnd; ++registryPathIterator)
    {
        const std::string& registryPath = *registryPathIterator;

        addMRUs(groupName, registryPath, itemList);
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addMRUs(const std::string& groupName, const std::string& keyPath, ItemList& itemList) const
{
    std::string::size_type backslashPos = keyPath.find('\\');
    if(backslashPos != std::string::npos)
    {
        const std::string baseKeyName = keyPath.substr(0, backslashPos);
        const std::string restKey = keyPath.substr(backslashPos + 1);

        if(baseKeyName == "HKEY_CURRENT_USER")
        {
            addMRUsFromRegistry(groupName, HKEY_CURRENT_USER, restKey, itemList);
        }
        else if(baseKeyName == "HKEY_LOCAL_MACHINE")
        {
            addMRUsFromRegistry(groupName, HKEY_LOCAL_MACHINE, restKey, itemList);
        }
        else if(baseKeyName == "HKEY_USERS")
        {
            addMRUsFromRegistry(groupName, HKEY_USERS, restKey, itemList);
        }
        else if(baseKeyName == "FARR_MRU")
        {
            addMRUsFromFile(groupName, restKey, itemList);
        }
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addMRUsFromRegistry(const std::string& groupName, HKEY baseKey, const std::string& restKeyPath, ItemList& itemList)
{
    if(usesSubkeys(restKeyPath))
    {
        std::string::size_type pipePos = restKeyPath.rfind('|');

        const std::string subKey = restKeyPath.substr(0, pipePos);

        RegistryKey key;
        if(key.open(baseKey, subKey.c_str(), KEY_READ))
        {
            const std::string valueName = restKeyPath.substr(pipePos + 1);

            addWithSubkey(groupName, key, valueName, itemList);
        }
    }
    else
    {
        RegistryKey key;
        if(key.open(baseKey, restKeyPath.c_str(), KEY_READ))
        {
            if(hasMRUList(key))
            {
                addWithMRUList(groupName, key, itemList);
            }
            else
            {
                addWithItemNo(groupName, key, itemList);
            }
        }
    }
}

//-----------------------------------------------------------------------

bool FarrMostRecentlyUsedPlugin::hasMRUList(const RegistryKey& registryKey)
{
    const DWORD MaxMRUListLength = 100;
    char mruList[MaxMRUListLength] = { 0 };
    DWORD type;
    DWORD length = MaxMRUListLength;
    return registryKey.queryValue("MRUList", &type, (BYTE*)mruList, &length);
}

//-----------------------------------------------------------------------

bool FarrMostRecentlyUsedPlugin::usesSubkeys(const std::string& restKeyPath)
{
    std::string::size_type pipePos = restKeyPath.rfind('|');

    return (pipePos != std::string::npos);
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addWithMRUList(const std::string& groupName, const RegistryKey& registryKey, ItemList& itemList)
{
    const DWORD MaxMRUListLength = 100;
    char mruList[MaxMRUListLength] = { 0 };
    DWORD type;
    DWORD length = MaxMRUListLength;
    if(registryKey.queryValue("MRUList", &type, (BYTE*)mruList, &length))
    {
        const unsigned long mruListItemCount = strlen(mruList);
        for(unsigned long index = 0; index < mruListItemCount; ++index)
        {
            char mruListItem[2] = { mruList[index], 0 };

            const DWORD MaxMRUValueLength = MAX_PATH;
            char mruValueBuffer[MaxMRUValueLength] = { 0 };
            DWORD length = MaxMRUValueLength;
            if(registryKey.queryValue(mruListItem, &type, (BYTE*)mruValueBuffer, &length))
            {
                std::string mruValue(mruValueBuffer);

                itemList.push_back(Item(groupName, mruValue, PathIsURL(mruValue.c_str()) ? Item::Type_URL : Item::Type_File));
            }
        }
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addWithItemNo(const std::string& groupName, const RegistryKey& registryKey, ItemList& itemList)
{
    const DWORD MaxValueNameLength = 100;
    char valueName[MaxValueNameLength] = { 0 };
    DWORD length = MaxValueNameLength;

    const DWORD MaxMRUValueLength = MAX_PATH;
    DWORD type;
    char value[MaxMRUValueLength] = { 0 };
    DWORD valueLength = MaxMRUValueLength;

    typedef std::map<unsigned long, std::string> OrderedItems;
    OrderedItems orderedItems;

    DWORD index = 0;
    while(registryKey.enumValue(index, valueName, &length, &type, (BYTE*)value, &valueLength))
    {
        std::string valueNameString(valueName);
        std::string::size_type startOfNumber = valueNameString.find_first_of("0123456789");

        std::stringstream stream(valueNameString.substr(startOfNumber));
        unsigned long itemNumber;
        stream >> itemNumber;

        std::string item(value);

        removeInvalidStuff(item);

        orderedItems.insert(OrderedItems::value_type(itemNumber, item));

        ++index;
        length = MaxValueNameLength;
        valueLength = MaxMRUValueLength;
    }

    OrderedItems::const_iterator it = orderedItems.begin();
    OrderedItems::const_iterator end = orderedItems.end();
    for( ; it != end; ++it)
    {
        const std::string& item = it->second;
        
        itemList.push_back(Item(groupName, item, PathIsURL(item.c_str()) ? Item::Type_URL : Item::Type_File));
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addWithSubkey(const std::string& groupName, const RegistryKey& registryKey, const std::string& valueName, ItemList& itemList)
{
    const DWORD MaxKeyNameLength = 100;
    char keyName[MaxKeyNameLength] = { 0 };
    DWORD keyNameLength = MaxKeyNameLength;

    typedef std::map<unsigned long, std::string> OrderedItems;
    OrderedItems orderedItems;

    FILETIME lastWriteTime = { 0 };

    DWORD index = 0;
    while(registryKey.enumKey(index, keyName, &keyNameLength, &lastWriteTime))
    {
        RegistryKey subKey;
        if(subKey.open(registryKey, keyName, KEY_READ))
        {
            std::string keyNameString(keyName);
            std::string::size_type startOfNumber = keyNameString.find_first_of("0123456789");

            std::stringstream stream(keyNameString.substr(startOfNumber));
            unsigned long itemNumber;
            stream >> itemNumber;

            DWORD type;
            const DWORD MaxMRUValueLength = MAX_PATH;
            char mruValueBuffer[MaxMRUValueLength] = { 0 };
            DWORD length = MaxMRUValueLength;
            if(subKey.queryValue(valueName.c_str(), &type, (BYTE*)mruValueBuffer, &length))
            {
                std::string item(mruValueBuffer);

                fixAdobePath(item);

                orderedItems.insert(OrderedItems::value_type(itemNumber, item));
            }
        }

        ++index;
        keyNameLength = MaxKeyNameLength;
    }

    OrderedItems::const_iterator it = orderedItems.begin();
    OrderedItems::const_iterator end = orderedItems.end();
    for( ; it != end; ++it)
    {
        const std::string& item = it->second;
        
        itemList.push_back(Item(groupName, item, PathIsURL(item.c_str()) ? Item::Type_URL : Item::Type_File));
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addMRUsFromFile(const std::string& groupName, const std::string& applicationKey, ItemList& itemList) const
{
    if(applicationKey == "openoffice")
    {
        addMRUs_OpenOffice(groupName, itemList);
    }
    else if(applicationKey == "npp")
    {
        addMRUs_NotepadPlusPlus(groupName, itemList);
    }
}

//-----------------------------------------------------------------------

const char* OpenOffice_RecentFile = "\\OpenOffice.org2\\user\\registry\\data\\org\\openoffice\\Office\\Common.xcu";

const char* OpenOffice_History = "History";
const char* OpenOffice_List = "List";
const char* OpenOffice_URL = "URL";

bool FarrMostRecentlyUsedPlugin::isInstalledOpenOffice() const
{
    if(!_appDataFolder.empty())
    {
        char recentFile[MAX_PATH] = { 0 };
        strcpy(recentFile, _appDataFolder.c_str());
        PathAppend(recentFile, OpenOffice_RecentFile);

        return (PathFileExists(recentFile) == TRUE);
    }
    else
    {
        return false;
    }
}

void FarrMostRecentlyUsedPlugin::addMRUs_OpenOffice(const std::string& groupName, ItemList& itemList) const
{
    if(!_appDataFolder.empty())
    {
        char recentFile[MAX_PATH] = { 0 };
        strcpy(recentFile, _appDataFolder.c_str());
        PathAppend(recentFile, OpenOffice_RecentFile);

        TiXmlDocument document;
        if(document.LoadFile(recentFile))
        {
            const TiXmlElement* rootElement = document.RootElement();

            const TiXmlElement* historyElement;
            for(historyElement = rootElement->FirstChildElement(); historyElement != 0; historyElement = historyElement->NextSiblingElement())
            {
                // find History element
                const char* attributeValue = historyElement->Attribute("oor:name");
                if((attributeValue != 0) && (strcmp(attributeValue, OpenOffice_History) == 0))
                {
                    // find List element
                    const TiXmlElement* listElement;
                    for(listElement = historyElement->FirstChildElement(); listElement != 0; listElement = listElement->NextSiblingElement())
                    {
                        const char* attributeValue = listElement->Attribute("oor:name");
                        if((attributeValue != 0) && (strcmp(attributeValue, OpenOffice_List) == 0))
                        {
                            // iterate over recent items
                            const TiXmlElement* recentItemElement;
                            for(recentItemElement = listElement->FirstChildElement(); recentItemElement != 0; recentItemElement = recentItemElement->NextSiblingElement())
                            {
                                // and get URL (file)
                                const TiXmlElement* urlElement;
                                for(urlElement = recentItemElement->FirstChildElement(); urlElement != 0; urlElement = urlElement->NextSiblingElement())
                                {
                                    const char* attributeValue = urlElement->Attribute("oor:name");
                                    if((attributeValue != 0) && (strcmp(attributeValue, OpenOffice_URL) == 0))
                                    {
                                        const TiXmlElement* valueElement = urlElement->FirstChildElement();

                                        const char* value = valueElement->GetText();
                                        if(value != 0)
                                        {
                                            if(UrlIsFileUrl(value) == TRUE)
                                            {
                                                char mruValue[MAX_PATH] = { 0 };
                                                DWORD length = MAX_PATH;
                                                if(SUCCEEDED(PathCreateFromUrl(value, mruValue, &length, 0)))
                                                {
                                                    itemList.push_back(Item(groupName, mruValue, Item::Type_File));
                                                }
                                            }
                                            else
                                            {
                                                itemList.push_back(Item(groupName, value, PathIsURL(value) ? Item::Type_URL : Item::Type_File));
                                            }
                                        }

                                        break;
                                    }
                                }
                            }

                            break;
                        }
                    }

                    break;
                }
            }
        }
    }
}

//-----------------------------------------------------------------------

const char* Npp_RecentFile = "\\Notepad++\\config.xml";

const char* Npp_History = "History";

bool FarrMostRecentlyUsedPlugin::isInstalledNotepadPlusPlus() const
{
    if(!_appDataFolder.empty())
    {
        char recentFile[MAX_PATH] = { 0 };
        strcpy(recentFile, _appDataFolder.c_str());
        PathAppend(recentFile, Npp_RecentFile);

        return (PathFileExists(recentFile) == TRUE);
    }
    else
    {
        return false;
    }
}

void FarrMostRecentlyUsedPlugin::addMRUs_NotepadPlusPlus(const std::string& groupName, ItemList& itemList) const
{
    if(!_appDataFolder.empty())
    {
        char recentFile[MAX_PATH] = { 0 };
        strcpy(recentFile, _appDataFolder.c_str());
        PathAppend(recentFile, Npp_RecentFile);

        TiXmlDocument document;
        if(document.LoadFile(recentFile))
        {
            ItemList temporaryList;

            const TiXmlElement* rootElement = document.RootElement();

            const TiXmlElement* element;
            for(element = rootElement->FirstChildElement(); element != 0; element = element->NextSiblingElement())
            {
                // find History element
                const char* nodeName = element->Value();
                if((nodeName != 0) && (strcmp(nodeName, Npp_History) == 0))
                {
                    // iterator over recent files
                    const TiXmlElement* fileElement;
                    for(fileElement = element->FirstChildElement(); fileElement != 0; fileElement = fileElement->NextSiblingElement())
                    {
                        const char* filename = fileElement->Attribute("filename");
                        if(filename != 0)
                        {
                            temporaryList.push_back(Item(groupName, filename, PathIsURL(filename) ? Item::Type_URL : Item::Type_File));
                        }
                    }

                    break;
                }
            }

            updateFileTimes(temporaryList);
            sortItemsByFileTime(temporaryList, CompareFiletime::LastAccessed);

            itemList.insert(itemList.end(), temporaryList.begin(), temporaryList.end());
        }
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::removeInvalidStuff(std::string& path)
{
    const std::string::size_type pos = path.find('*');
    if(pos != std::string::npos)
    {
        path.erase(0, pos + 1);
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::fixAdobePath(std::string& path)
{
    if(!path.empty())
    {
        std::replace_if(path.begin(), path.end(), IsSameCharacter('/'), '\\');

        if(path[0] == '\\')
        {
            // is it something like \C\file.txt
            if((path.length() > 4) && (path[2] == '\\'))
            {
                // remove leading backslash
                path.erase(0, 1);

                // then insert :
                path.insert(1, ":");
            }
            else
            {
                // assume UNC, insert another backslash
                path.insert(0, "\\");
            }
        }
    }
}

//-----------------------------------------------------------------------

FarrMostRecentlyUsedPlugin::ItemVector::size_type FarrMostRecentlyUsedPlugin::getItemCount() const
{
    return _itemVector.size();
}

//-----------------------------------------------------------------------

const Item& FarrMostRecentlyUsedPlugin::getItem(const ItemVector::size_type& index) const
{
    return _itemVector[index];
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::showOptions()
{
    OptionsDialog::Groups groups;

    groups.push_back(OptionsDialog::Group(MyRecentDocuments_GroupName, "My Recent Documents"));

    GroupNameToGroup::const_iterator it = _groupNameToGroup.begin();
    GroupNameToGroup::const_iterator end = _groupNameToGroup.end();

    for( ; it != end; ++it)
    {
        const Group& group = it->second;

        if(isInstalled(group))
        {
            groups.push_back(OptionsDialog::Group(it->first, group.description));
        }
    }

    OptionsDialog dialog(_optionsFile, groups);
    if(dialog.DoModal() == IDOK)
    {
        _options.update(_optionsFile);

        _rebuildItemCache = true;
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::extractSearchOptions(std::string& searchString,
                                                      OrderedStringCollection& options, 
                                                      OrderedStringCollection& groups,
                                                      OrderedStringCollection& extensions) const
{
    std::string::size_type startPos = searchString.find('+');
    while(startPos != std::string::npos)
    {
        const std::string::size_type endPos = searchString.find(' ', startPos);
        
        std::string optionString = searchString.substr(startPos + 1, (endPos != std::string::npos) ? endPos - startPos - 1 : std::string::npos);
        if(!optionString.empty())
        {
            if(optionString[0] == '.')
            {
                extensions.insert(optionString);
            }
            else
            {
                if(_mruOptions.find(optionString) != _mruOptions.end())
                {
                    options.insert(optionString);
                }
                else if(_farrOptions.find(optionString) == _farrOptions.end())
                {
                    groups.insert(optionString);
                }
            }
        }

        searchString.erase(startPos, (endPos != std::string::npos) ? endPos - startPos + 1 : std::string::npos);

        if(endPos == std::string::npos)
        {
            break;
        }
        else
        {
            startPos = searchString.find('+');
        }
    }

    // remove trailing space
    if(!searchString.empty() && (searchString[searchString.length() - 1] == ' '))
    {
        searchString.erase(searchString.length() - 1);
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::sortItems(ItemList& itemList)
{
    switch(_sortModeCurrentSearch)
    {
    case Options::Sort_TimeLastAccessed:
        sortItemsByFileTime(itemList, CompareFiletime::LastAccessed);
        break;

    case Options::Sort_TimeLastModified:
        sortItemsByFileTime(itemList, CompareFiletime::LastModifed);
        break;

    case Options::Sort_TimeCreated:
        sortItemsByFileTime(itemList, CompareFiletime::Created);
        break;

    case Options::Sort_Alphabetically:
        sortItemsAlphabetically(itemList);
        break;

    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::sortItemsByFileTime(ItemList& itemList, int fileTimeType)
{
    CompareFiletime compareFiletime((CompareFiletime::FileTimeType)fileTimeType);
    itemList.sort(compareFiletime);
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::sortItemsAlphabetically(ItemList& itemList)
{
    CompareName compareName;
    itemList.sort(compareName);
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::resolveLinks(ItemList& itemList)
{
    ItemList::iterator it = itemList.begin();
    ItemList::iterator end = itemList.end();

    for( ; it != end; ++it)
    {
        Item& item = *it;

        resolveLink(item);
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::resolveLink(Item& item)
{
    if(strcmp(PathFindExtension(item.path.c_str()), ".lnk") == 0)
    {
        if(SUCCEEDED(_shellLinkFile->Load(CComBSTR(item.path.c_str()), STGM_READ)))
        {
            char targetPath[MAX_PATH] = { 0 };
            WIN32_FIND_DATA findData = { 0 };
            if(SUCCEEDED(_shellLink->GetPath(targetPath, MAX_PATH, 0/*&findData*/, SLGP_UNCPRIORITY)))
            {
                item.path = targetPath;

                bool isDirectory = ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
                item.type = (isDirectory ? Item::Type_Folder : Item::Type_File);
            }
        }
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::updateFileTimes(ItemList& itemList) const
{
    ItemList::iterator it = itemList.begin();
    ItemList::iterator end = itemList.end();

    for( ; it != end; ++it)
    {
        Item& item = *it;

        const bool isUNCPath = (PathIsUNC(item.path.c_str()) == TRUE);

        if(!isUNCPath || !_options.ignoreExistenceCheckUNCPaths)
        {
            updateFileTime(item);
        }
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::updateFileTime(Item& item)
{
    HANDLE file = CreateFile(item.path.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(file != 0)
    {
        FILETIME lastAccessTime = { 0 };
        FILETIME lastWriteTime = { 0 };
        FILETIME creationTime = { 0 };
        if(GetFileTime(file, &creationTime, &lastAccessTime, &lastWriteTime) == TRUE)
        {
            if(isFileTimeNull(item.creationTime))
            {
                item.creationTime = creationTime;
            }
            if(isFileTimeNull(item.lastAccessTime))
            {
                item.lastAccessTime = lastAccessTime;
            }
            if(isFileTimeNull(item.lastModifiedTime))
            {
                item.lastModifiedTime = lastWriteTime;
            }
        }

        CloseHandle(file);
    }
}

//-----------------------------------------------------------------------

bool FarrMostRecentlyUsedPlugin::isFileTimeNull(const FILETIME& fileTime)
{
    return ((fileTime.dwHighDateTime == 0) && (fileTime.dwLowDateTime == 0));
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::setDebugLevel(const OrderedStringCollection& options)
{
    DebugLevel oldDebugLevel = _debugLevel;

    if(options.find(Option_DebugLevel0) != options.end())
    {
        _debugLevel = Debug_Level0;
    }
    else if(options.find(Option_DebugLevel1) != options.end())
    {
        _debugLevel = Debug_Level1;
    }
    else if(options.find(Option_DebugLevel2) != options.end())
    {
        _debugLevel = Debug_Level2;
    }

    if(_debugLevel != oldDebugLevel)
    {
        std::stringstream stream;
        stream << "FARR MRU: Debug Level " << _debugLevel << "\n";
        OutputDebugString(stream.str().c_str());
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::debugOutputMessage(DebugLevel debugLevel, const char* message) const
{
    if(_debugLevel >= debugLevel)
    {
        std::stringstream stream;
        stream << "FARR MRU: " << message << "\n";

        OutputDebugString(stream.str().c_str());
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::debugOutputResultList(DebugLevel debugLevel, const ItemList& itemList) const
{
    if(_debugLevel >= debugLevel)
    {
        OutputDebugString("FARR MRU: Begin result list\n");

        ItemList::const_iterator it = itemList.begin();
        const ItemList::const_iterator end = itemList.end();

        for( ; it != end; ++it)
        {
            const Item& item = *it;

            std::stringstream stream;
            stream << item.path << "\n";
            OutputDebugString(stream.str().c_str());
        }

        OutputDebugString("FARR MRU: End result list\n");
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::debugOutputNumber(DebugLevel debugLevel, const char* message, long number) const
{
    if(_debugLevel >= debugLevel)
    {
        std::stringstream stream;
        stream << "FARR MRU: " << message << ": " << number << "\n";

        OutputDebugString(stream.str().c_str());
    }
}

//-----------------------------------------------------------------------
