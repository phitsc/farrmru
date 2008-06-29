#pragma once

///////////////////////////////////////////////////////////////////////////////

#include <string>

#include <list>

#include <shlwapi.h>
#include <crtdbg.h>

///////////////////////////////////////////////////////////////////////////////

class FileList
{
    public:
        typedef std::list<std::string> Filenames;
        typedef Filenames::const_iterator const_iterator;

        enum
        {
            Files       = 1,
            Directories = 2,
        };

        FileList(const std::string& path, const std::string& filter, int typeFilter = Files|Directories)
        {
            // path is too long, needs space to append '/' and 'filter'
            _ASSERT(path.length() < MAX_PATH-1-filter.length());

            // create find pattern (path + '/*.dll');
            char findPattern[MAX_PATH];
            strcpy(findPattern, path.c_str());
            PathAddBackslash(findPattern);
            strcat(findPattern, filter.c_str());

            // create the path again for the found files
            char thePath[MAX_PATH];
            strcpy(thePath, path.c_str());
            PathAddBackslash(thePath);

            // get all the dlls
            WIN32_FIND_DATA findData;
            HANDLE hFindFile = FindFirstFile(findPattern, &findData);
            if(INVALID_HANDLE_VALUE != hFindFile)
            {
                if(useFile(findData, typeFilter))
                {
                    _filenames.push_back(std::string(thePath) + std::string(findData.cFileName));
                }

                while(TRUE == FindNextFile(hFindFile, &findData))
                {
                    if(useFile(findData, typeFilter))
                    {
                        _filenames.push_back(std::string(thePath) + std::string(findData.cFileName));
                    }
                }
				FindClose(hFindFile);
            }
        }

        const_iterator begin() const { return _filenames.begin(); }
        const_iterator end() const { return _filenames.end(); }

        const unsigned long getCount() const { return (unsigned long)_filenames.size(); }

    private:
        static bool useFile(const WIN32_FIND_DATA& findData, int typeFilter)
        {
            bool copyDirectory = ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) &&
                                 ((typeFilter & Directories) == Directories);

            bool copyFile = !((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) && 
                            ((typeFilter & Files) == Files);

            return (copyDirectory || copyFile);
        }

        Filenames _filenames;
};

///////////////////////////////////////////////////////////////////////////////