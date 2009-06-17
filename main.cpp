#include "main.h"
#include "FarrMostRecentlyUsedPlugin.h"

#include <string>
#include <iostream>

HINSTANCE dllInstanceHandle = 0;

int main(int argc, char* argv[])
{
    if(SUCCEEDED(CoInitializeEx(0, COINIT_APARTMENTTHREADED)))
    {
        IShellLink* shellLinkRawPtr = 0;

        HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&shellLinkRawPtr);
        if(SUCCEEDED(hr))
        {
            char temporaryDirectory[MAX_PATH] = { 0 };

            GetTempPath(MAX_PATH, temporaryDirectory);

            char modulePath[MAX_PATH] = { 0 };
            GetModuleFileName(NULL, modulePath, MAX_PATH);
            PathRemoveFileSpec(modulePath);

            FarrMostRecentlyUsedPlugin farrMostRecentlyUsedPlugin(modulePath, shellLinkRawPtr);

            std::string searchString;
            for(int index = 1; index < argc; ++index)
            {
                if(!searchString.empty())
                {
                    searchString += " ";
                }

                searchString += argv[index];
            }

            farrMostRecentlyUsedPlugin.search(searchString.c_str());

            const FarrMostRecentlyUsedPlugin::ItemVector::size_type itemCount = farrMostRecentlyUsedPlugin.getItemCount();
            for(FarrMostRecentlyUsedPlugin::ItemVector::size_type index = 0; index < itemCount; ++index)
            {
                const Item& item = farrMostRecentlyUsedPlugin.getItem(index);

                std::cout << item.path << "\n";
            }

            return 0;
        }
    }

    return 1;
}