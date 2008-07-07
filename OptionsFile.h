#pragma once

#include <string>
#include <sstream>

//-----------------------------------------------------------------------

class OptionsFile
{
public:
    OptionsFile(const std::string& filePath)
        :_filePath(filePath)
    {
    }

    long getValue(const std::string& optionName, long defaultValue) const
    {
        return GetPrivateProfileInt("Options", optionName.c_str(), defaultValue, _filePath.c_str());
    }

    void setValue(const std::string& optionName, long value)
    {
        std::stringstream stream;
        stream << value;

        WritePrivateProfileString("Options", optionName.c_str(), stream.str().c_str(), _filePath.c_str());
    }

    bool getValue(const std::string& optionName, bool defaultValue) const
    {
        return (GetPrivateProfileInt("Options", optionName.c_str(), defaultValue ? 1 : 0, _filePath.c_str()) != 0);
    }

    void setValue(const std::string& optionName, bool value)
    {
        std::stringstream stream;
        stream << (value ? 1 : 0);

        WritePrivateProfileString("Options", optionName.c_str(), stream.str().c_str(), _filePath.c_str());
    }

    std::string getValue(const std::string& optionName, const char* defaultValue) const
    {
        const unsigned long MaxValueLength = 4096;
        char value[MaxValueLength] = { 0 };
        GetPrivateProfileString("Options", optionName.c_str(), defaultValue, value, MaxValueLength, _filePath.c_str());

        return value;
    }

    void setValue(const std::string& optionName, const std::string& value)
    {
        WritePrivateProfileString("Options", optionName.c_str(), value.c_str(), _filePath.c_str());
    }

private:
    std::string _filePath;
};

//-----------------------------------------------------------------------
