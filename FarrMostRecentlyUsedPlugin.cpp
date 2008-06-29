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

#include <shlobj.h>

//-----------------------------------------------------------------------

Options::Options(const OptionsFile& optionsFile)
{
    update(optionsFile);
}

//-----------------------------------------------------------------------

void Options::update(const OptionsFile& optionsFile)
{
    ignoreDirectories = optionsFile.getValue("IgnoreDirectories", true);
}

//-----------------------------------------------------------------------

FarrMostRecentlyUsedPlugin::FarrMostRecentlyUsedPlugin(const std::string& modulePath, IShellLink* shellLinkRawPtr) :
_optionsFile(modulePath + "\\FarrMostRecentlyUsed.ini"),
_options(_optionsFile),
_shellLink(shellLinkRawPtr)
{
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
                    currentType = line.substr(1);
                }
                else
                {
                    RegistryPaths& registryPaths = _typeToRegistryPaths[currentType];
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

    const OptionStrings optionStrings = extractOptionStrings(rawSearchString);
    if(optionStrings.empty())
    {
        // use recent folder
        addRecentDocuments(itemList);

        sortItems(itemList);
        resolveLinks(itemList);
    }
    else
    {
        // use special type from registry
        OptionStrings::const_iterator optionStringIterator = optionStrings.begin();
        const OptionStrings::const_iterator optionStringsEnd = optionStrings.end();

        for( ; optionStringIterator != optionStringsEnd; ++optionStringIterator)
        {
            const std::string& optionString = *optionStringIterator;

            // is the option a type
            const TypeToRegistryPaths::const_iterator typeIterator = _typeToRegistryPaths.find(optionString);
            if(typeIterator != _typeToRegistryPaths.end())
            {
                const RegistryPaths& registryPaths = typeIterator->second;

                RegistryPaths::const_iterator registryPathIterator = registryPaths.begin();
                const RegistryPaths::const_iterator registryPathsEnd = registryPaths.end();
                for( ; registryPathIterator != registryPathsEnd; ++registryPathIterator)
                {
                    const std::string& registryPath = *registryPathIterator;

                    addMRUs(registryPath, itemList);
                }
            }
        }
    }

    filterItems(itemList, searchString);

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
            const std::string& path = *it;
            itemList.push_back(path);
        }
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addMRUs(const std::string& keyPath, ItemList& itemList)
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
                std::stringstream stream;
                stream << "adding: " << baseKeyName << "\\" << restKey << std::endl;
                OutputDebugString(stream.str().c_str());

                if(hasMRUList(key))
                {
                    addWithMRUList(key, itemList);
                }
                else
                {
                    addWithItemNo(key, itemList);
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

void FarrMostRecentlyUsedPlugin::addWithMRUList(const RegistryKey& registryKey, ItemList& itemList)
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
            char mruValue[MaxMRUValueLength] = { 0 };
            DWORD length = MaxMRUValueLength;
            if(registryKey.queryValue(mruListItem, &type, (BYTE*)mruValue, &length))
            {
                itemList.push_back(mruValue);
            }
        }
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::addWithItemNo(const RegistryKey& registryKey, ItemList& itemList)
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
        itemList.push_back(item);
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

const std::string& FarrMostRecentlyUsedPlugin::getItem(const Items::size_type& index) const
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

FarrMostRecentlyUsedPlugin::OptionStrings FarrMostRecentlyUsedPlugin::extractOptionStrings(const std::string& rawSearchString)
{
    OptionStrings optionStrings;

    std::string::size_type startPos = rawSearchString.find('+');
    while(startPos != std::string::npos)
    {
        const std::string::size_type endPos = rawSearchString.find(' ', startPos);
        
        std::string optionString = rawSearchString.substr(startPos + 1, (endPos != std::string::npos) ? endPos - startPos - 1 : std::string::npos);
        if(!optionString.empty())
        {
            optionStrings.push_back(optionString);
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

    return optionStrings;
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::sortItems(ItemList& itemList)
{
    LastModified lastModified;

    itemList.sort(lastModified);
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::resolveLinks(ItemList& itemList)
{
    ItemList::iterator it = itemList.begin();
    ItemList::iterator end = itemList.end();

    for( ; it != end; ++it)
    {
        std::string& item = *it;

        resolveLink(item);
    }
}

//-----------------------------------------------------------------------

bool FarrMostRecentlyUsedPlugin::isDirectory(const std::string& path)
{
    bool isDirectory = ((GetFileAttributes(path.c_str()) & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
    return isDirectory;
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::filterItems(ItemList& itemList, const std::string& searchString)
{
    if(_options.ignoreDirectories)
    {
        itemList.remove_if(isDirectory);
    }

    if(!searchString.empty())
    {
        FileNameDoesntStartWithSearchstringIgnoringCase fileNamedoesntStartWithSearchstringIgnoringCase(searchString);
        itemList.remove_if(fileNamedoesntStartWithSearchstringIgnoringCase);
    }
}

//-----------------------------------------------------------------------

void FarrMostRecentlyUsedPlugin::resolveLink(std::string& path)
{
    const std::string extension = PathFindExtension(path.c_str());
    if(extension == ".lnk")
    {
        CComPtr<IPersistFile> shellLinkFile;
        _shellLink.QueryInterface(&shellLinkFile);

        if(SUCCEEDED(shellLinkFile->Load(CComBSTR(path.c_str()), STGM_READ)))
        {
            char targetPath[MAX_PATH] = { 0 };
            if(SUCCEEDED(_shellLink->GetPath(targetPath, MAX_PATH, 0, SLGP_UNCPRIORITY)))
            {
                path = targetPath;
            }
        }
    }
}

//-----------------------------------------------------------------------
