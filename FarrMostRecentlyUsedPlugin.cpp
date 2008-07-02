#include "stdafx.h"
#include "FarrMostRecentlyUsedPlugin.h"

#include "OptionsDialog.h"
#include "OptionsFile.h"
#include "RegistryKey.h"
#include "FileList.h"
#include "SortingAndFiltering.h"
#include "IsURL.h"

#include <algorithm>
#include <sstream>
#include <fstream>
#include <map>

//-----------------------------------------------------------------------

const IsURL isURL;

//-----------------------------------------------------------------------

FarrMostRecentlyUsedPlugin::FarrMostRecentlyUsedPlugin(const std::string& modulePath, IShellLink* shellLinkRawPtr) :
_optionsFile(modulePath + "\\FarrMostRecentlyUsed.ini"),
_options(_optionsFile),
_shellLink(shellLinkRawPtr),
_sortModeCurrentSearch(Options::Sort_NoSorting)
{
    _shellLink.QueryInterface(&_shellLinkFile);

    // ignore these, they are used by farr
    _farrOptions.insert("sall");
    _farrOptions.insert("alias");

    // 
    const std::string configFileName = modulePath + "\\FarrMostRecentlyUsed.config";
    std::ifstream mruSpecificationsFile(configFileName.c_str());
    if(mruSpecificationsFile.is_open())
    {
        std::string currentType;

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

                    std::string groupName;

                    // extract group name, if present
                    std::string::size_type pos = line.find('|');
                    if(pos != std::string::npos)
                    {
                        currentType = line.substr(0, pos);
                        groupName = line.substr(pos + 1);
                    }
                    else
                    {
                        currentType = line;
                    }

                    // adds entry
                    GroupNameAndRegistryPaths& groupNameAndRegistryPaths = _typeToGroupNameAndRegistryPaths[currentType];
                    groupNameAndRegistryPaths.first = groupName;
                }
                else
                {
                    GroupNameAndRegistryPaths& groupNameAndRegistryPaths = _typeToGroupNameAndRegistryPaths[currentType];
                    RegistryPaths& registryPaths = groupNameAndRegistryPaths.second;

                    registryPaths.push_back(line);
                }
            }
        }
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::search(const std::string& rawSearchString, const std::string& searchString)
{
    ItemList itemList;

    OrderedStringCollection options;
    OrderedStringCollection extensions;
    extractOptionsAndExtensions(rawSearchString, options, extensions);

    const bool forceSortByName = (options.find("byname") != options.end());
    options.erase("byname");

    const bool forceSortByDate = (options.find("bydate") != options.end());
    options.erase("bydate");

    _sortModeCurrentSearch = _options.sortMode;
    if(forceSortByName)
    {
        _sortModeCurrentSearch = Options::Sort_Alphabetically;
    }
    else if(forceSortByDate)
    {
        _sortModeCurrentSearch = Options::Sort_TimeLastModified;
    }

    bool needsSorting = (_sortModeCurrentSearch != 0);

    if(options.empty())
    {
        // use recent folder
        addRecentDocuments(itemList);

        if(needsSorting && (_sortModeCurrentSearch == Options::Sort_TimeLastModified))
        {
            sortItemsLastModified(itemList);

            needsSorting = false;
        }

        resolveLinks(itemList);
    }
    else
    {
        // use special type from registry
        OrderedStringCollection::const_iterator optionIterator = options.begin();
        const OrderedStringCollection::const_iterator optionEnd = options.end();

        for( ; optionIterator != optionEnd; ++optionIterator)
        {
            const std::string& option = *optionIterator;

            // is the option a known type
            const TypeToGroupNameAndRegistryPaths::const_iterator typeIterator = _typeToGroupNameAndRegistryPaths.find(option);
            if(typeIterator != _typeToGroupNameAndRegistryPaths.end())
            {
                const GroupNameAndRegistryPaths& groupNameAndRegistryPaths = typeIterator->second;
                const std::string& groupName = groupNameAndRegistryPaths.first;
                const RegistryPaths& registryPaths = groupNameAndRegistryPaths.second;

                RegistryPaths::const_iterator registryPathIterator = registryPaths.begin();
                const RegistryPaths::const_iterator registryPathsEnd = registryPaths.end();
                for( ; registryPathIterator != registryPathsEnd; ++registryPathIterator)
                {
                    const std::string& registryPath = *registryPathIterator;

                    addMRUs(groupName, registryPath, itemList);
                }
            }
        }

        if(needsSorting && (_sortModeCurrentSearch == Options::Sort_TimeLastModified))
        {
            needsSorting = false;
        }
    }

    filterItems(itemList, searchString, extensions);

    //debugOutputResultList(itemList);

    if(needsSorting && (_sortModeCurrentSearch == Options::Sort_Alphabetically))
    {
        sortItemsAlphabetically(itemList);
    }

    _items.assign(itemList.begin(), itemList.end());
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addRecentDocuments(ItemList& itemList)
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
            itemList.push_back(Item("", file.path, (file.type == FileList::File::Type_Directory) ? Item::Type_Folder : Item::Type_File));
        }
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

bool FarrMostRecentlyUsedPlugin::hasMRUList(const RegistryKey& registryKey) const
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
                itemList.push_back(Item(groupName, mruValue, isURL(mruValue) ? Item::Type_URL : Item::Type_File));
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
        itemList.push_back(Item(groupName, item, isURL(item) ? Item::Type_URL : Item::Type_File));
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

FarrMostRecentlyUsedPlugin::Items::size_type FarrMostRecentlyUsedPlugin::getItemCount() const
{
    return _items.size();
}

//-----------------------------------------------------------------------

const Item& FarrMostRecentlyUsedPlugin::getItem(const Items::size_type& index) const
{
    return _items[index];
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::showOptions()
{
    OptionsDialog dialog(_optionsFile);
    if(dialog.DoModal() == IDOK)
    {
        _options.update(_optionsFile);
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::extractOptionsAndExtensions(const std::string& rawSearchString, OrderedStringCollection& options, OrderedStringCollection& extensions) const
{
    std::string::size_type startPos = rawSearchString.find('+');
    while(startPos != std::string::npos)
    {
        const std::string::size_type endPos = rawSearchString.find(' ', startPos);
        
        std::string optionString = rawSearchString.substr(startPos + 1, (endPos != std::string::npos) ? endPos - startPos - 1 : std::string::npos);
        if(!optionString.empty())
        {
            if(optionString[0] == '.')
            {
                extensions.insert(optionString);
            }
            else
            {
                if(_farrOptions.find(optionString) == _farrOptions.end())
                {
                    options.insert(optionString);
                }
            }
        }

        if(endPos == std::string::npos)
        {
            break;
        }
        else
        {
            startPos = rawSearchString.find('+', endPos + 1);
        }
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::sortItemsLastModified(ItemList& itemList)
{
    CompareLastModified compareLastModified;
    itemList.sort(compareLastModified);
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

void FarrMostRecentlyUsedPlugin::filterItems(ItemList& itemList, const std::string& searchString, const OrderedStringCollection& extensions)
{
    NeedsToBeRemoved needsToBeRemoved(_options, _sortModeCurrentSearch, extensions, searchString);
    itemList.remove_if(needsToBeRemoved);
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
            if(SUCCEEDED(_shellLink->GetPath(targetPath, MAX_PATH, &findData, SLGP_UNCPRIORITY)))
            {
                bool isDirectory = ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);

                //if(isDirectory)
                //{
                //    OutputDebugString(targetPath);
                //}

                item.path = targetPath;
                item.type = (isDirectory ? Item::Type_Folder : Item::Type_File);
            }
        }
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::debugOutputResultList(const ItemList& itemList)
{
    OutputDebugString("==== Result list ====");

    ItemList::const_iterator it = itemList.begin();
    const ItemList::const_iterator end = itemList.end();

    for( ; it != end; ++it)
    {
        const std::string& itemPath = it->path;
        OutputDebugString(itemPath.c_str());
    }
}