#pragma once

///////////////////////////////////////////////////////////////////////////////

#include <string>
#include <sstream>
#include <windows.h>

///////////////////////////////////////////////////////////////////////////////

namespace util
{

///////////////////////////////////////////////////////////////////////////////

struct String
{
    // Convert 8-bit characters from the active locale to UTF-8.
    // Text and Result MAY be the same buffer.
    static void UTF8Encode(const std::string& Text, std::string& Result);

    // Convert UTF-8 characters to 8-bit characters from the active locale.
    // UTF8 and Result MAY be the same buffer.
    static void UTF8Decode(const std::string& UTF8, std::string& Result);

    static void tolower(std::string& toConvert);
    static bool equalsNoCase(const std::string& left, const std::string& right);
    static bool isAlpha(char character);
    static bool isAlphanumeric(char character);
    static bool isDigit(char character);
    static char getDecimalPoint();
    static void removeTrailingSpaces(std::string& text);
    static void removePreceedingSpaces(std::string& text);
    static void addTrailingSpace(std::string& text);

    template<typename Type>
    static Type fromString(const std::string& valueAsString)
    {
        std::stringstream stream(valueAsString);
        Type value;
        stream >> value;

        return value;
    }

    template<typename Type>
    static std::string toString(const Type& type)
    {
        std::stringstream stream;
        stream << type;

        return stream.str();
    }

    template<typename CollectionType>
    static void tokenize(const std::string& string, const std::string& delimiters, CollectionType& tokens);

    template<typename CollectionType>
    static void tokenize(const std::string& string, const char delimiter, CollectionType& tokens);

    typedef std::pair<std::string, std::string> StringPair;
    static StringPair split(const std::string& stringToSplit, char delimiter);

    static bool containsSubstringNoCase(const std::string& value, const std::string& substring);

    static std::string getNumberWithOrdinal(const std::string& numberWithoutOrdinal);

    static std::string safeAssignCharPtr(const char* charPtr);

    static std::wstring towstring(const std::string& string);
    static std::string tostring(const std::wstring& string);

    static void copyString(char* buffer, int maxLength, const std::string& string);
};

///////////////////////////////////////////////////////////////////////////////

struct Date
{
    enum FormatDateTimeOptions
    {
        FormatDateTimeOption_None            = 0x0,
        FormatDateTimeOption_DateOnly        = 0x1,
        FormatDateTimeOption_ShortDate       = 0x2,
        FormatDateTimeOption_NoWeekdayName   = 0x4,
        FormatDateTimeOption_ShowSeconds     = 0x8
    };

    static SYSTEMTIME getSystemTimeFromIso8601Time(const std::string& iso8601Time);
    static SYSTEMTIME getLocalTimeFromIso8601Time(const std::string& iso8601Time);
    static std::string getIso8601TimeFromSystemTime(const SYSTEMTIME& dateTime);
    static std::string formatDateTime(const SYSTEMTIME& systemTime, bool dateOnly, bool useShortDateFormat, bool showSeconds);
    static std::string formatDateTime(const std::string& iso8601Time, bool dateOnly, bool useShortDateFormat);
    static std::string formatDateTime(const std::string& iso8601Time, bool dateOnly, bool useShortDateFormat, bool noWeekdayName);
    static std::string formatDateTime(const SYSTEMTIME& systemTime, FormatDateTimeOptions options);
    static SYSTEMTIME parseDateTime(const std::string& dateTimeAsString, bool& hasTime);
    static bool matchesDateTime(const SYSTEMTIME& leftTime, const SYSTEMTIME& rightTime, bool timeMustMatchToo);
    static long compareDateTime(const SYSTEMTIME& leftTime, const SYSTEMTIME& rightTime, bool timeMustMatchToo);
    static void setDay(SYSTEMTIME& dateTime, const std::string& day);
    static void setMonth(SYSTEMTIME& dateTime, const std::string& month);
    static void setTime(SYSTEMTIME& dateTime, const std::string& hour, const std::string& minute, const std::string& amPM);
    static void clearDateTimeIfInvalid(SYSTEMTIME& dateTime);
};

///////////////////////////////////////////////////////////////////////////////

class VersionInfo
{
    public:
        VersionInfo(HMODULE module = 0)
        {
            char filename[MAX_PATH];
            GetModuleFileName(module, filename, MAX_PATH-1);

			initialize(filename);
		}

		// ctor used in SystemUpgrader
		VersionInfo(const char* filename)
		{
			initialize(filename);
		}


        struct FileVersion
        {
            FileVersion() : major(0), minor(0), service(0), build(0) {}

            DWORD major;
            DWORD minor;
            DWORD service;
            DWORD build;

            std::string getAsString() const
            {
                std::stringstream stream;
                stream << major << '.' << minor << '.' << service;

                return stream.str();
            }
        };

        const FileVersion& getFileVersion() const { return _fileVersion; }
        const std::string& getSpecialBuild() const { return _specialBuild; }

    private:
		void initialize(const char* filename);

		FileVersion _fileVersion;
        std::string _specialBuild;
};

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////
