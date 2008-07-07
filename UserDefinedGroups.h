#pragma once

//-----------------------------------------------------------------------

#include <string>
#include <set>

//-----------------------------------------------------------------------

class UserDefinedGroups
{
public:
    UserDefinedGroups()
    {}

    UserDefinedGroups(const std::string& userDefinedGroupsString)
    {
        tokenize(userDefinedGroupsString, '|', _userDefinedGroups);
    };

    void operator=(const std::string& userDefinedGroupsString)
    {
        tokenize(userDefinedGroupsString, '|', _userDefinedGroups);
    }

    bool contains(const std::string& groupName) const
    {
        return (_userDefinedGroups.find(groupName) != _userDefinedGroups.end());
    }

private:
    template<typename CollectionType>
    static void tokenize(const std::string& string, const char delimiter, CollectionType& tokens)
    {
        std::string::size_type lastPos = string.find_first_not_of(delimiter, 0);
        std::string::size_type pos = string.find_first_of(delimiter, lastPos);

        while(std::wstring::npos != pos || std::wstring::npos != lastPos)
        {
            tokens.insert(string.substr(lastPos, pos - lastPos));
            lastPos = string.find_first_not_of(delimiter, pos);
            pos = string.find_first_of(delimiter, lastPos);
        }
    }

    typedef std::set<std::string> UserDefinedGroupCollection;
    UserDefinedGroupCollection _userDefinedGroups;
};

//-----------------------------------------------------------------------