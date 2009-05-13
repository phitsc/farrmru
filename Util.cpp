#include "Util.h"

#include <locale>
#include <vector>

///////////////////////////////////////////////////////////////////////////////

namespace util
{

///////////////////////////////////////////////////////////////////////////////

void String::UTF8Encode(const std::string& Text, std::string& Result)
{
    int InputChars = strlen(Text.c_str())+0;        // MSVCP strings tend to append extra nulls, so don't process them.

    // We need to first convert the ASCII input to Unicode before we can convert it to UTF-8...
    int UnicodeChars = MultiByteToWideChar(CP_ACP, 0, Text.c_str(), InputChars, 0, 0);
    LPWSTR UnicodeBuffer = (LPWSTR)alloca(UnicodeChars*sizeof(WCHAR));
    MultiByteToWideChar(CP_ACP, 0, Text.c_str(), InputChars, UnicodeBuffer, UnicodeChars);

    // Now that we've got everything translated to Unicode, we can (finally) convert it to UTF-8.
    int UTF8Chars = WideCharToMultiByte(CP_UTF8, 0, UnicodeBuffer, UnicodeChars, 0, 0, 0, 0);
    LPSTR UTF8Buffer = (LPSTR)alloca(UTF8Chars);
    WideCharToMultiByte(CP_UTF8, 0, UnicodeBuffer, UnicodeChars, UTF8Buffer, UTF8Chars, 0, 0);

    // Store it in the std::string passed in.  Note that this string will be null terminated just like
    // any other C-style string, but it'll have the special encodings for multibyte characters.
    Result.assign(UTF8Buffer, UTF8Chars);
}

///////////////////////////////////////////////////////////////////////////////

void String::UTF8Decode(const std::string& UTF8, std::string& Result)
{
    const int InputBytes = strlen(UTF8.c_str())+0;        // MSVCP strings tend to append extra nulls, so don't process them.

    // Again, we need to convert the UTF-8 string to Unicode before we can convert it to 8-bit.
    int UnicodeChars = MultiByteToWideChar(CP_UTF8, 0, UTF8.c_str(), InputBytes, 0, 0);
    LPWSTR UnicodeBuffer = (LPWSTR)alloca(UnicodeChars*sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, UTF8.c_str(), InputBytes, UnicodeBuffer, UnicodeChars);

    // Now that we've got everything translated to Unicode, we can convert it to 8-bit characters.
    int SingleByteChars = WideCharToMultiByte(CP_ACP, 0, UnicodeBuffer, UnicodeChars, 0, 0, 0, 0);
    LPSTR SingleByteBuffer = (LPSTR)alloca(SingleByteChars);
    WideCharToMultiByte(CP_ACP, 0, UnicodeBuffer, UnicodeChars, SingleByteBuffer, SingleByteChars, 0, 0);

    Result.assign(SingleByteBuffer, SingleByteChars);
}

///////////////////////////////////////////////////////////////////////////////

void String::tolower(std::string& toConvert)
{
    std::locale loc("");
    std::use_facet<std::ctype<char> >(loc).tolower(&toConvert[0], &toConvert[toConvert.length()]);
}

///////////////////////////////////////////////////////////////////////////////

bool String::equalsNoCase(const std::string& left, const std::string& right)
{
    return (_strcmpi(left.c_str(), right.c_str()) == 0);
}

///////////////////////////////////////////////////////////////////////////////

bool String::isAlpha(char character)
{
    std::locale loc("");
    return std::isalpha(character, loc);
}

///////////////////////////////////////////////////////////////////////////////

bool String::isAlphanumeric(char character)
{
    std::locale loc("");
    return std::isalnum(character, loc);
}

///////////////////////////////////////////////////////////////////////////////

bool String::isDigit(char character)
{
    std::locale loc("");
    return std::isdigit(character, loc);
}

///////////////////////////////////////////////////////////////////////////////

char String::getDecimalPoint()
{
    std::locale loc("");
    return std::use_facet<std::numpunct<char> >(loc).decimal_point();
}

///////////////////////////////////////////////////////////////////////////////

void String::removeTrailingSpaces(std::string& text)
{
    if(!text.empty() && (text[text.length() - 1] == ' '))
    {
        const std::string::size_type pos = text.find_last_not_of(' ');
        text.erase(pos + 1);
    }
}

///////////////////////////////////////////////////////////////////////////////

void String::removePreceedingSpaces(std::string& text)
{
    if(!text.empty() && (text[0] == ' '))
    {
        const std::string::size_type pos = text.find_first_not_of(' ');
        text.erase(0, pos);
    }
}

///////////////////////////////////////////////////////////////////////////////

void String::addTrailingSpace(std::string& text)
{
    if(!text.empty() && (text[text.length() - 1] != ' '))
    {
        text.push_back(L' ');
    }
}

///////////////////////////////////////////////////////////////////////////////

template<typename CollectionType>
void String::tokenize(const std::string& string, const std::string& delimiters, CollectionType& tokens)
{
    // Skip delimiters at beginning.
    std::string::size_type lastPos = string.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos = string.find_first_of(delimiters, lastPos);

    while(std::string::npos != pos || std::string::npos != lastPos)
    {
        // Found a token, add it to the collection
        tokens.push_back(string.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = string.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = string.find_first_of(delimiters, lastPos);
    }
}

///////////////////////////////////////////////////////////////////////////////

template<typename CollectionType>
void String::tokenize(const std::string& string, const char delimiter, CollectionType& tokens)
{
    // Skip delimiters at beginning.
    std::string::size_type lastPos = string.find_first_not_of(delimiter, 0);
    // Find first "non-delimiter".
    std::string::size_type pos = string.find_first_of(delimiter, lastPos);

    while(std::string::npos != pos || std::string::npos != lastPos)
    {
        // Found a token, add it to the collection
        tokens.push_back(string.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = string.find_first_not_of(delimiter, pos);
        // Find next "non-delimiter"
        pos = string.find_first_of(delimiter, lastPos);
    }
}

///////////////////////////////////////////////////////////////////////////////

String::StringPair String::split(const std::string& stringToSplit, char delimiter)
{
    const std::string::size_type pos = stringToSplit.find(delimiter);
    if(pos != std::string::npos)
    {
        return StringPair(stringToSplit.substr(0, pos), stringToSplit.substr(pos + 1));
    }
    else
    {
        return StringPair(stringToSplit, "");
    }
}

///////////////////////////////////////////////////////////////////////////////

bool String::containsSubstringNoCase(const std::string& value, const std::string& substring)
{
    std::string valueLower(value);
    tolower(valueLower);

    std::string substringLower(substring);
    tolower(substringLower);

    if(valueLower.find(substringLower) != std::string::npos)
    {
        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////////

std::string String::getNumberWithOrdinal(const std::string& numberWithoutOrdinal)
{
    std::string result;

    if(!numberWithoutOrdinal.empty())
    {
        result = numberWithoutOrdinal;

        const long number = fromString<long>(numberWithoutOrdinal);
        if(number >= 21)
        {
            const long lastDigit = fromString<long>(numberWithoutOrdinal.substr(numberWithoutOrdinal.length() - 1));
            switch(lastDigit)
            {
            case 1:
                result += "st";
                break;

            case 2:
                result += "nd";
                break;

            case 3:
                result += "rd";
                break;

            default:
                result += "th";
                break;
            }
        }
        else if(number >= 0)
        {
            switch(number)
            {
            case 1:
                result += "st";
                break;

            case 2:
                result += "nd";
                break;

            case 3:
                result += "rd";
                break;

            default:
                result += "th";
                break;
            }
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////

std::string String::safeAssignCharPtr(const char* charPtr)
{
    if(charPtr != 0)
    {
        return std::string(charPtr);
    }
    else
    {
        return std::string();
    }
}

///////////////////////////////////////////////////////////////////////////////

std::wstring String::towstring(const std::string& string)
{
    std::wstring result;
    result.resize(string.length() + 1);

    std::locale loc;
    std::use_facet<std::ctype<wchar_t> >(loc)._Widen_s(&string[0], &string[string.length()], &result[0], string.length());

    result[string.length()] = L'\0';
    result.resize(string.length());

    return result;
}

///////////////////////////////////////////////////////////////////////////////

std::string String::tostring(const std::wstring& string)
{
    std::string result;
    result.resize(string.length() + 1);

    std::locale loc;
    std::use_facet<std::ctype<wchar_t> >(loc)._Narrow_s(&string[0], &string[string.length()], ' ', &result[0], string.length());

    result[string.length()] = '\0';
    result.resize(string.length());

    return result;
}

///////////////////////////////////////////////////////////////////////////////

void String::copyString(char* buffer, int maxLength, const std::string& string)
{
#pragma warning (push)
#pragma warning (disable : 4996)
    const std::string::size_type numberOfCharactersCopied = string.copy(buffer, maxLength - 1);
    buffer[numberOfCharactersCopied] = '\0';
#pragma warning (pop)
}

///////////////////////////////////////////////////////////////////////////////

SYSTEMTIME Date::getSystemTimeFromIso8601Time(const std::string& iso8601Time)
{
    if(!iso8601Time.empty())
    {
        const std::string::size_type pos1 = iso8601Time.find('T');
        const std::string date = iso8601Time.substr(0, pos1);
        const std::string::size_type pos2 = iso8601Time.find('Z', pos1 + 1);
        const std::string time = iso8601Time.substr(pos1 + 1, (pos2 != std::string::npos) ? (pos2 - pos1 - 1) : std::string::npos);

        typedef std::vector<std::string> Strings;

        Strings dateParts;
        String::tokenize(date, '-', dateParts);
        if(dateParts.size() == 1)
        {
            dateParts.resize(3);
            dateParts[0] = date.substr(0, 4);
            dateParts[1] = date.substr(4, 2);
            dateParts[2] = date.substr(6, 2);
        }

        Strings timeParts;
        String::tokenize(time, ':', timeParts);
        if(timeParts.size() == 1)
        {
            timeParts.resize(3);
            timeParts[0] = time.substr(0, 2);
            timeParts[1] = time.substr(2, 2);
            timeParts[2] = time.substr(3, 2);
        }

        SYSTEMTIME systemTime = 
        {
            String::fromString<WORD>(dateParts[0]),
            String::fromString<WORD>(dateParts[1]),
            0,
            String::fromString<WORD>(dateParts[2]),
            String::fromString<WORD>(timeParts[0]),
            String::fromString<WORD>(timeParts[1]),
            String::fromString<WORD>(timeParts[2]),
            0
        };

        return systemTime;
    }
    else
    {
        SYSTEMTIME systemTime = { 30000, 1, 1, 1 };

        return systemTime;
    }
}

///////////////////////////////////////////////////////////////////////////////

std::string Date::getIso8601TimeFromSystemTime(const SYSTEMTIME& dateTime)
{
    TIME_ZONE_INFORMATION timeZoneInfo = { 0 };
    GetTimeZoneInformation(&timeZoneInfo);

    SYSTEMTIME systemTime = { 0 };
    TzSpecificLocalTimeToSystemTime(&timeZoneInfo, &dateTime, &systemTime);

    std::stringstream stream;
    stream << systemTime.wYear << "-" << systemTime.wMonth << "-" << systemTime.wDay << "T" << systemTime.wHour << ":" << systemTime.wMinute << ":" << systemTime.wSecond << "Z";

    return stream.str();
}

///////////////////////////////////////////////////////////////////////////////

std::string Date::formatDateTime(const SYSTEMTIME& systemTime, bool dateOnly, bool useShortDateFormat, bool showSeconds)
{
    int options = FormatDateTimeOption_None;

    if(dateOnly)
    {
        options |= FormatDateTimeOption_DateOnly;
    }
    if(useShortDateFormat)
    {
        options |= FormatDateTimeOption_ShortDate;
    }
    if(showSeconds)
    {
        options |= FormatDateTimeOption_ShowSeconds;
    }

    return formatDateTime(systemTime, (FormatDateTimeOptions)options);
}

///////////////////////////////////////////////////////////////////////////////

std::string Date::formatDateTime(const std::string& iso8601Time, bool dateOnly, bool useShortDateFormat)
{
    return formatDateTime(iso8601Time, dateOnly, useShortDateFormat, false);
}

///////////////////////////////////////////////////////////////////////////////

SYSTEMTIME Date::getLocalTimeFromIso8601Time(const std::string& iso8601Time)
{
    const SYSTEMTIME systemTime = getSystemTimeFromIso8601Time(iso8601Time);

    TIME_ZONE_INFORMATION timeZoneInfo = { 0 };
    GetTimeZoneInformation(&timeZoneInfo);

    SYSTEMTIME localTime = { 0 };
    SystemTimeToTzSpecificLocalTime(&timeZoneInfo, &systemTime, &localTime);

    return localTime;
}

///////////////////////////////////////////////////////////////////////////////

std::string Date::formatDateTime(const std::string& iso8601Time, bool dateOnly, bool useShortDateFormat, bool noWeekdayName)
{
    const SYSTEMTIME localTime = getLocalTimeFromIso8601Time(iso8601Time);

    int options = FormatDateTimeOption_None;

    if(dateOnly)
    {
        options |= FormatDateTimeOption_DateOnly;
    }
    if(useShortDateFormat)
    {
        options |= FormatDateTimeOption_ShortDate;
    }
    if(noWeekdayName)
    {
        options |= FormatDateTimeOption_NoWeekdayName;
    }

    return formatDateTime(localTime, (FormatDateTimeOptions)options);
}

///////////////////////////////////////////////////////////////////////////////

std::string Date::formatDateTime(const SYSTEMTIME& systemTime, FormatDateTimeOptions options)
{
    static SYSTEMTIME empty = { 0 };

    if(memcmp(&systemTime, &empty, sizeof(SYSTEMTIME)) != 0)
    {
        const int MaxLength = 100;

        const bool useShortFormat = ((options & FormatDateTimeOption_ShortDate) == FormatDateTimeOption_ShortDate);
        const DWORD dateFormat = (useShortFormat ? DATE_SHORTDATE : DATE_LONGDATE);

        char date[MaxLength];

        const bool noWeekdayName = ((options & FormatDateTimeOption_NoWeekdayName) == FormatDateTimeOption_NoWeekdayName);
        if(noWeekdayName)
        {
            char buffer[MaxLength];
            GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLONGDATE, buffer, MaxLength);

            std::string dateFormatString(buffer);
            const std::string weekdayNameFormatString("ddd");
            const std::string::size_type begin = dateFormatString.find(weekdayNameFormatString);
            if(begin != std::string::npos)
            {
                std::string::size_type offset = begin + weekdayNameFormatString.length();
                if(offset < dateFormatString.length())
                {
                    if(dateFormatString[offset] == 'd')
                    {
                        offset++;
                    }
                }
                const std::string::size_type end = dateFormatString.find_first_of("dMyg", offset);
                dateFormatString.erase(begin, end);
            }

            GetDateFormat(LOCALE_USER_DEFAULT, 0, &systemTime, dateFormatString.c_str(), date, MaxLength);
        }
        else
        {
            GetDateFormat(LOCALE_USER_DEFAULT, dateFormat, &systemTime, 0, date, MaxLength);
        }

        char time[MaxLength];
        const bool showSeconds = ((options & FormatDateTimeOption_ShowSeconds) == FormatDateTimeOption_ShowSeconds);
        GetTimeFormat(LOCALE_USER_DEFAULT, (showSeconds ? 0 : TIME_NOSECONDS), &systemTime, 0, time, MaxLength);

        const bool dateOnly = ((options & FormatDateTimeOption_DateOnly) == FormatDateTimeOption_DateOnly);
        if(dateOnly)
        {
            return date;
        }
        else
        {
            return (std::string(date) + " " + time);
        }
    }
    else
    {
        return "";
    }
}

///////////////////////////////////////////////////////////////////////////////

SYSTEMTIME Date::parseDateTime(const std::string& dateTimeAsString, bool& hasTime)
{
    SYSTEMTIME dateTime = { 0 };

    std::string dateTimeLowercase(dateTimeAsString);
    String::tolower(dateTimeLowercase);

    hasTime = false;

    if(dateTimeLowercase == "today")
    {
        GetLocalTime(&dateTime);
    }
    else if(dateTimeLowercase == "tomorrow")
    {
        GetLocalTime(&dateTime);

        FILETIME fileTime = { 0 };
        SystemTimeToFileTime(&dateTime, &fileTime);

        __int64 fileTimeLarge;
        memcpy(&fileTimeLarge, &fileTime, sizeof(__int64));

        //const __int64 nano100SecInWeek=(__int64)10000000*60*60*24*7;
        //const __int64 nano100SecInDay=(__int64)10000000*60*60*24;
        //const __int64 nano100SecInHour=(__int64)10000000*60*60;
        //const __int64 nano100SecInMin=(__int64)10000000*60;
        //const __int64 nano100SecInSec=(__int64)10000000; 

        const __int64 nano100SecInDay = (__int64)10000000 * 60 * 60 * 24;
        fileTimeLarge += (1 * nano100SecInDay);

        memcpy(&fileTime, &fileTimeLarge, sizeof(__int64));

        FileTimeToSystemTime(&fileTime, &dateTime);
    }
    else
    {
        typedef std::vector<std::string> Strings;
        Strings dateTimeParts;

        String::tokenize(dateTimeAsString, "./-: ", dateTimeParts);

        static const WORD MinYear = 1999;

        std::locale loc("");
        std::time_get<char>::dateorder order = std::use_facet<std::time_get<char> >(loc).date_order();

        switch(order)
        {
        case std::time_base::dmy:
        case std::time_base::mdy: 
            {
                if(dateTimeParts.size() >= 2)
                {
                    GetLocalTime(&dateTime);

                    if(order == std::time_base::dmy)
                    {
                        setDay(dateTime, dateTimeParts[0]);
                        setMonth(dateTime, dateTimeParts[1]);
                    }
                    else
                    {
                        setDay(dateTime, dateTimeParts[1]);
                        setMonth(dateTime, dateTimeParts[0]);
                    }
                }

                if(dateTimeParts.size() >= 3)
                {
                    WORD year = String::fromString<WORD>(dateTimeParts[2]);
                    if(year > MinYear)
                    {
                        dateTime.wYear = year;

                        if(dateTimeParts.size() >= 5)
                        {
                            setTime(dateTime, dateTimeParts[3], dateTimeParts[4], (dateTimeParts.size() == 6) ? dateTimeParts[5] : "");
                            hasTime = true;
                        }
                    }
                    else if(dateTimeParts.size() >= 4)
                    {
                        setTime(dateTime, dateTimeParts[2], dateTimeParts[3], (dateTimeParts.size() == 5) ? dateTimeParts[4] : "");
                        hasTime = true;
                    }
                }
            }
            break;

        case std::time_base::ydm: 
            break;

        case std::time_base::ymd: 
            break;

        default:
            break;
        }

        clearDateTimeIfInvalid(dateTime);

    }

    return dateTime;
}

///////////////////////////////////////////////////////////////////////////////

long Date::compareDateTime(const SYSTEMTIME& leftTime, const SYSTEMTIME& rightTime, bool timeMustMatchToo)
{
    SYSTEMTIME left(leftTime);
    SYSTEMTIME right(rightTime);

    if(!timeMustMatchToo)
    {
        left.wHour = 12;
        left.wMinute = 0;
        left.wSecond = 0;
        left.wMilliseconds = 0;

        right.wHour = 12;
        right.wMinute = 0;
        right.wSecond = 0;
        right.wMilliseconds = 0;
    }

    FILETIME leftFileTime;
    SystemTimeToFileTime(&left, &leftFileTime);

    FILETIME rightFileTime;
    SystemTimeToFileTime(&right, &rightFileTime);

    return CompareFileTime(&leftFileTime, &rightFileTime);
}

///////////////////////////////////////////////////////////////////////////////

bool Date::matchesDateTime(const SYSTEMTIME& leftTime, const SYSTEMTIME& rightTime, bool timeMustMatchToo)
{
    return (compareDateTime(leftTime, rightTime, timeMustMatchToo) == 0);
}

///////////////////////////////////////////////////////////////////////////////

void Date::setDay(SYSTEMTIME& dateTime, const std::string& day)
{
    dateTime.wDay = String::fromString<WORD>(day);
}

///////////////////////////////////////////////////////////////////////////////

void Date::setMonth(SYSTEMTIME& dateTime, const std::string& month)
{
    if(!month.empty() && isdigit(month[0]))
    {
        dateTime.wMonth = String::fromString<WORD>(month);
    }
    else
    {
        std::basic_stringstream<char> monthName;
        std::ios_base::iostate state = 0;
        tm time = { 0 };

        monthName << month;
        std::locale loc("");
        monthName.imbue(loc);
        std::basic_istream<char>::_Iter i = std::use_facet<std::time_get<char> >(loc).get_monthname(std::basic_istream<char>::_Iter(monthName.rdbuf()), std::basic_istream<char>::_Iter(0), monthName, state, &time);

        if(!(state & std::ios_base::failbit))
        {
            dateTime.wMonth = (WORD)(time.tm_mon + 1);
        }
        else
        {
            dateTime.wMonth = 0;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void Date::setTime(SYSTEMTIME& dateTime, const std::string& hour, const std::string& minute, const std::string& amPM)
{
    dateTime.wHour = String::fromString<WORD>(hour);
    dateTime.wMinute = String::fromString<WORD>(minute);

    if(amPM == "pm")
    {
        dateTime.wHour += 12;
    }
}

///////////////////////////////////////////////////////////////////////////////

void Date::clearDateTimeIfInvalid(SYSTEMTIME& dateTime)
{
    FILETIME dummy = { 0 };
    if(SystemTimeToFileTime(&dateTime, &dummy) == FALSE)
    {
        memset(&dateTime, 0, sizeof(SYSTEMTIME));
    }
}

///////////////////////////////////////////////////////////////////////////////

void VersionInfo::initialize(const char* filename)
{
    typedef std::vector<std::string> Strings;

    DWORD handle;
    DWORD bufferSize = GetFileVersionInfoSize(filename, &handle);
    if(bufferSize > 0)
    {
        BYTE* buffer = new BYTE[bufferSize];
        BOOL ok = GetFileVersionInfo(filename, 0, bufferSize, buffer);
        if(ok)
        {
            char* info;
            UINT  len;

            // get file version
            ok = VerQueryValue(buffer, "\\StringFileInfo\\000004b0\\FileVersion", (void**)&info, &len);
            if(ok)
            {
                std::string fileVersion(info, len);
                Strings versionStrings;
                util::String::tokenize(fileVersion, ",.", versionStrings);

                if(versionStrings.size() > 0)
                {
                    std::stringstream stream;
                    stream << versionStrings[0];
                    stream >> _fileVersion.major;
                }

                if(versionStrings.size() > 1)
                {
                    std::stringstream stream;
                    stream << versionStrings[1];
                    stream >> _fileVersion.minor;
                }

                if(versionStrings.size() > 2)
                {
                    std::stringstream stream;
                    stream << versionStrings[2];
                    stream >> _fileVersion.service;
                }

                if(versionStrings.size() > 3)
                {
                    std::stringstream stream;
                    stream << versionStrings[3];
                    stream >> _fileVersion.build;
                }
            }

            // get special build
            ok = VerQueryValue(buffer, "\\StringFileInfo\\000004b0\\SpecialBuild", (void**)&info, &len);
            if(ok)
            {
                _specialBuild.assign(info, len - 1);
            }

            delete [] buffer;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////
