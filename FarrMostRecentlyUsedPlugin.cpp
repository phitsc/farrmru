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

//-----------------------------------------------------------------------

const char* Option_ByName   = "byname";
const char* Option_ByDate   = "bydate";
const char* Option_ByMod    = "bymod";
const char* Option_ByCreate = "bycreate";

//-----------------------------------------------------------------------

FarrMostRecentlyUsedPlugin::FarrMostRecentlyUsedPlugin(const std::string& modulePath, IShellLink* shellLinkRawPtr) :
_rebuildItemCache(true),
_currentMruMode(Mode_None),
_optionsFile(modulePath + "\\FarrMostRecentlyUsed.ini"),
_options(_optionsFile),
_shellLink(shellLinkRawPtr),
_sortModeCurrentSearch(Options::Sort_NoSorting)
{
    _shellLink.QueryInterface(&_shellLinkFile);

    // ignore these, they are used by farr
    _farrOptions.insert("sall");
    _farrOptions.insert("alias");

    _mruOptions.insert(Option_ByName);
    _mruOptions.insert(Option_ByDate);
    _mruOptions.insert(Option_ByMod);
    _mruOptions.insert(Option_ByCreate);

    // 
    const std::string configFileName = modulePath + "\\FarrMostRecentlyUsed.config";
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

                    // extract group name, if present
                    std::string::size_type pos = line.find('|');
                    if(pos != std::string::npos)
                    {
                        currentGroupName = line.substr(0, pos);
                        groupDescription = line.substr(pos + 1);
                    }
                    else
                    {
                        currentGroupName = line;
                    }

                    // adds entry
                    GroupDescriptionAndRegistryPaths& groupDescriptionAndRegistryPaths = _groupNameToDescriptionAndRegistryPaths[currentGroupName];
                    groupDescriptionAndRegistryPaths.first = groupDescription;
                }
                else
                {
                    GroupDescriptionAndRegistryPaths& groupDescriptionAndRegistryPaths = _groupNameToDescriptionAndRegistryPaths[currentGroupName];
                    RegistryPaths& registryPaths = groupDescriptionAndRegistryPaths.second;

                    registryPaths.push_back(line);
                }
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
    "mru"
};
const unsigned long MruAliasesCount = sizeof(MruAliases) / sizeof(MruAliases[0]);

const std::string MyRecentDocumentsAlias = "mrum";
const std::string ProgramsAlias = "mrup";
const std::string AllAlias = "mrua";
const std::string UserAlias = "mruu";
const std::string MenuAlias = "mru";

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

        OutputDebugString(searchString.c_str());

        const bool forceSortByName = (options.find(Option_ByName) != options.end());
        options.erase(Option_ByName);

        const bool forceSortByLastAccessTime = (options.find(Option_ByDate) != options.end());
        options.erase(Option_ByDate);

        const bool forceSortByLastModifiedTime = (options.find(Option_ByMod) != options.end());
        options.erase(Option_ByMod);

        const bool forceSortByCreationTime = (options.find(Option_ByCreate) != options.end());
        options.erase(Option_ByCreate);

        _sortModeCurrentSearch = _options.sortMode;
        if(forceSortByName)
        {
            _sortModeCurrentSearch = Options::Sort_Alphabetically;
        }
        else if(forceSortByLastAccessTime)
        {
            _sortModeCurrentSearch = Options::Sort_TimeLastAccessed;
        }
        else if(forceSortByLastModifiedTime)
        {
            _sortModeCurrentSearch = Options::Sort_TimeLastModified;
        }
        else if(forceSortByCreationTime)
        {
            _sortModeCurrentSearch = Options::Sort_TimeCreated;
        }

        if(_rebuildItemCache)
        {
            _itemCache.clear();

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
                break;

            case Mode_Menu:
                addMenuItems();
                break;
            }

            if(mruMode != Mode_Menu)
            {
                RemoveItemsStage1 removeItemsStage1(_options);
                _itemCache.remove_if(removeItemsStage1);

                updateFileTimes(_itemCache);
            }

            _rebuildItemCache = false;
        }

        ItemList itemList(_itemCache);

        if(mruMode != Mode_Menu)
        {
            const bool noSubstringFiltering = (_options.sortMode == Options::Sort_NoSorting);   // FARR will do the filtering
            RemoveItemsStage2 removeItemsStage2(extensions, groups, searchString, noSubstringFiltering);
            itemList.remove_if(removeItemsStage2);

            const bool noSorting = ((mruMode == Mode_Programs) && (groups.size() == 1)) || (_sortModeCurrentSearch == Options::Sort_NoSorting);
            if(!noSorting)
            {
                sortItems(itemList);
            }

            RemoveDuplicates removeDuplicates;
            itemList.remove_if(removeDuplicates);
        }

        _itemVector.assign(itemList.begin(), itemList.end());
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addMenuItems()
{
    _itemCache.push_back(Item("Alias mrum", "List only 'My Recent Document'|restartsearch mrum", Item::Type_Alias));
    _itemCache.push_back(Item("Alias mrup", "List most recently used items of configured programs|restartsearch mrup", Item::Type_Alias));
    _itemCache.push_back(Item("Alias mrua", "List all most recently used items|restartsearch mrua", Item::Type_Alias));
    _itemCache.push_back(Item("Alias mruu", "List user defined most recently used items|restartsearch mruu", Item::Type_Alias));
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addMyRecentDocuments()
{
    char recentFolder[MAX_PATH] = { 0 };
    if(S_OK == SHGetFolderPath(0, CSIDL_RECENT, 0, SHGFP_TYPE_CURRENT, recentFolder))
    {
        const FileList fileList(recentFolder, "*.*", FileList::Files);

        FileList::const_iterator it = fileList.begin();
        const FileList::const_iterator end = fileList.end();

        for( ; it != end; ++it)
        {
            const FileList::File& file = *it;

            // use links last modified time as target files last access time
            _itemCache.push_back(Item("recent", file.path, (file.type == FileList::File::Type_Directory) ? Item::Type_Folder : Item::Type_File, 
                                      file.lastModifiedTime));
        }

        resolveLinks(_itemCache);

        //debugOutputResultList(_itemCache);
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addMruApplications()
{
    GroupNameToDescriptionAndRegistryPaths::const_iterator groupNameIterator = _groupNameToDescriptionAndRegistryPaths.begin();
    const GroupNameToDescriptionAndRegistryPaths::const_iterator groupNamesEnd = _groupNameToDescriptionAndRegistryPaths.end();

    for( ; groupNameIterator != groupNamesEnd; ++groupNameIterator)
    {
        addType(groupNameIterator, _itemCache);
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addType(const GroupNameToDescriptionAndRegistryPaths::const_iterator& typeIterator, ItemList& itemList)
{
    const std::string& groupName = typeIterator->first;
    const GroupDescriptionAndRegistryPaths& groupDescriptionAndRegistryPaths = typeIterator->second;
    const RegistryPaths& registryPaths = groupDescriptionAndRegistryPaths.second;

    RegistryPaths::const_iterator registryPathIterator = registryPaths.begin();
    const RegistryPaths::const_iterator registryPathsEnd = registryPaths.end();
    for( ; registryPathIterator != registryPathsEnd; ++registryPathIterator)
    {
        const std::string& registryPath = *registryPathIterator;

        addMRUs(groupName, registryPath, itemList);
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addMRUs(const std::string& groupName, const std::string& keyPath, ItemList& itemList)
{
    std::string::size_type backslashPos = keyPath.find('\\');
    if(backslashPos != std::string::npos)
    {
        std::string baseKeyName = keyPath.substr(0, backslashPos);
        std::string restKey = keyPath.substr(backslashPos + 1);

        HKEY baseKey = 0;
        if(baseKeyName == "HKEY_CURRENT_USER")
        {
            baseKey = HKEY_CURRENT_USER;
        }
        else if(baseKeyName == "HKEY_LOCAL_MACHINE")
        {
            baseKey = HKEY_LOCAL_MACHINE;
        }
        else if(baseKeyName == "HKEY_USERS")
        {
            baseKey = HKEY_USERS;
        }

        if(baseKey != 0)
        {
            RegistryKey key;
            if(key.open(baseKey, restKey.c_str(), KEY_READ))
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

                FILETIME null = { 0 };

                itemList.push_back(Item(groupName, mruValue, PathIsURL(mruValue.c_str()) ? Item::Type_URL : Item::Type_File, null));
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
        
        FILETIME null = { 0 };

        itemList.push_back(Item(groupName, item, PathIsURL(item.c_str()) ? Item::Type_URL : Item::Type_File, null));
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
    OptionsDialog dialog(_optionsFile);
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

void FarrMostRecentlyUsedPlugin::updateFileTimes(ItemList& itemList)
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

void FarrMostRecentlyUsedPlugin::debugOutputResultList(const ItemList& itemList)
{
    OutputDebugString("==== Result list ====");

    ItemList::const_iterator it = itemList.begin();
    const ItemList::const_iterator end = itemList.end();

    for( ; it != end; ++it)
    {
        const Item& item = *it;

        std::stringstream stream;
        stream << item.path << " - " << item.lastAccessTime.dwHighDateTime << " " << item.lastAccessTime.dwLowDateTime << "\n";
        OutputDebugString(stream.str().c_str());
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::debugOutputNumber(const char* comment, long number)
{
    std::stringstream stream;
    stream << comment << ": " << number << "\n";
    OutputDebugString(stream.str().c_str());
}

//-----------------------------------------------------------------------
