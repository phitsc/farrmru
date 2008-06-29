#pragma once

///////////////////////////////////////////////////////////////////////////////

class RegistryKey
{
public:
    RegistryKey() : _key(0)
    {}

    ~RegistryKey()
    {
        if(_key != 0)
        {
            RegCloseKey(_key);
        }
    }

    bool open(const RegistryKey& key, const char* subKey, REGSAM sam)
    {
        return open(key._key, subKey, sam);
    }

    bool open(HKEY key, const char* subKey, REGSAM sam)
    {
        long result = RegOpenKeyEx(key, subKey, 0, sam, &_key);
        if(ERROR_SUCCESS == result)
        {
            return true;
        }
        else
        {
            _key = 0;
            return false;
        }
    }

    bool create(HKEY key, const char* subKey, REGSAM sam)
    {
        DWORD disposition;
        long result = RegCreateKeyEx(key, subKey, 0, 0, REG_OPTION_NON_VOLATILE, sam, 0, &_key, &disposition);
        if(ERROR_SUCCESS == result)
        {
            return true;
        }
        else
        {
            _key = 0;
            return false;
        }
    }

    bool enumKey(DWORD index, char* name, DWORD* nameLength, FILETIME* lastWriteTime) const
    {   
        return (ERROR_SUCCESS == RegEnumKeyEx(_key, index, name, nameLength, 0, 0, 0, lastWriteTime));
    }

    bool enumValue(DWORD index, char* name, DWORD* nameLength, DWORD* type, BYTE* szValue, DWORD* valueLength) const
    {   
        return (ERROR_SUCCESS == RegEnumValue(_key, index, name, nameLength, 0, type, szValue, valueLength));
    }

    bool queryValue(const char* valueName, DWORD* type, BYTE* szValue, DWORD* valueLength) const
    {
        return (ERROR_SUCCESS == RegQueryValueEx(_key, valueName, 0, type, (BYTE*)szValue, valueLength));
    }

    bool setValue(const char* valueName, long value)
    {
        long result = RegSetValueEx(_key, valueName, 0, REG_DWORD, (const BYTE*)&value, sizeof value);
        return (ERROR_SUCCESS == result);
    }

    bool setValue(const char* valueName, const char* value)
    {
        long result = RegSetValueEx(_key, valueName, 0, REG_SZ, (const BYTE*)value, strlen(value) + 1);
        return (ERROR_SUCCESS == result);
    }

private:
    HKEY _key;
};

///////////////////////////////////////////////////////////////////////////////
