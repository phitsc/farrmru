#pragma once

#include <set>

//-----------------------------------------------------------------------

class IsURL
{
public:
    IsURL()
    {
        _webProtocols.insert("http://");
        _webProtocols.insert("https://");
        _webProtocols.insert("ftp://");
    }

    bool operator()(const std::string& item) const
    {
        OrderedStringCollection::const_iterator it = _webProtocols.begin();
        OrderedStringCollection::const_iterator end = _webProtocols.end();

        for( ; it != end; ++it)
        {
            const std::string& webProtocol = *it;

            if(_stricmp(item.substr(0, webProtocol.length()).c_str(), webProtocol.c_str()) == 0)
            {
                return true;
            }
        }

        return false;
    }

private:
    typedef std::set<std::string> OrderedStringCollection;
    OrderedStringCollection _webProtocols;
};

//-----------------------------------------------------------------------
