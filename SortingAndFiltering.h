#pragma once

#include <string>

#ifdef min
#undef min
#endif
#include <algorithm>

//-----------------------------------------------------------------------

class LastModified
{
public:
    bool operator()(const std::string& left, const std::string& right) const
    {
        HANDLE leftFile = openFile(left);
        HANDLE rightFile = openFile(right);
        if((leftFile != 0) && (rightFile != 0))
        {
            BY_HANDLE_FILE_INFORMATION leftInfo = { 0 };
            bool leftOk = (GetFileInformationByHandle(leftFile, &leftInfo) == TRUE);

            BY_HANDLE_FILE_INFORMATION rightInfo = { 0 };
            bool rightOk = (GetFileInformationByHandle(rightFile, &rightInfo) == TRUE);

            if(leftOk && rightOk)
            {
                return (CompareFileTime(&leftInfo.ftLastWriteTime, &rightInfo.ftLastWriteTime) >= 0);
            }
        }

        return false;
    }

private:
    HANDLE openFile(const std::string& fileName) const
    {
        return CreateFile(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    }
};

//-----------------------------------------------------------------------

class FileNameDoesntStartWithSearchstringIgnoringCase
{
public:
    FileNameDoesntStartWithSearchstringIgnoringCase(const std::string& searchString)
        :_searchString(searchString)
    {}

    bool operator()(const std::string& workingString) const
    {
        const size_t charCount = std::min(workingString.length(), _searchString.length());

        return (_strnicmp(PathFindFileName(workingString.c_str()), _searchString.c_str(), charCount) != 0);
    }

private:
    std::string _searchString;
};

//-----------------------------------------------------------------------
