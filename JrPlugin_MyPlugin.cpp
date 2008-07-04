//-----------------------------------------------------------------------
// JrPlugin_MyPlugin
//-----------------------------------------------------------------------



//-----------------------------------------------------------------------
// VS wants this
#include "stdafx.h"
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// System includes
#include <stdio.h>
#include <string>
#include <sstream>
//-----------------------------------------------------------------------


#include "FarrMostRecentlyUsedPlugin.h"
#include "RegistryKey.h"



//-----------------------------------------------------------------------
// our partner header which includes the funcs we need in the right way
#include "JrPlugin_MyPlugin.h"
//-----------------------------------------------------------------------


//-----------------------------------------------------------------------
// see JrPluginFuncs_FARR.h for function definitions we will be implementing here
//-----------------------------------------------------------------------





//-----------------------------------------------------------------------
// global state info
E_SearchStateT current_searchstate=E_SearchState_Stopped;
E_ResultAvailableStateT resultsavailabletype=E_ResultAvailableState_None;
int numresultsavailable=0;
BOOL current_lockstate=FALSE;
bool isready=true;
//
//
// farr-specific function pointer so we can call to inform the host when we have results
Fp_GlobalPluginCallback_NotifySearchStateChanged callbackfp_notifysearchstatechanged=NULL;
//-----------------------------------------------------------------------





































//-----------------------------------------------------------------------
// These functions are our plugin-specific versions of the generic
//  functions exported by the generic DLL shell.
// See JrPlugin_GenericShell.h and .cpp for descriptions of what these
//  functions are supposed to do.
//-----------------------------------------------------------------------

HINSTANCE dllInstanceHandle = 0;

//-----------------------------------------------------------------------
BOOL MyPlugin_DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* /*lpReserved*/)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        dllInstanceHandle = hinst;
        break;

    case DLL_PROCESS_DETACH:
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;
    }

    // success
    return TRUE;
}
//-----------------------------------------------------------------------

FarrMostRecentlyUsedPlugin* farrMostRecentlyUsedPlugin = 0;

IShellLink* shellLinkRawPtr = 0;

//-----------------------------------------------------------------------

BOOL MyPlugin_DoInit()
{
    if(SUCCEEDED(CoInitializeEx(0, COINIT_APARTMENTTHREADED)))
    {
        HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&shellLinkRawPtr);
        if(SUCCEEDED(hr))
        {
            char modulePath[MAX_PATH] = { 0 };
            GetModuleFileName(dllInstanceHandle, modulePath, MAX_PATH);
            PathRemoveFileSpec(modulePath);

            farrMostRecentlyUsedPlugin = new FarrMostRecentlyUsedPlugin(modulePath, shellLinkRawPtr);

            return TRUE;
        }
    }

    return FALSE;
}


BOOL MyPlugin_DoShutdown()
{
    if(farrMostRecentlyUsedPlugin != 0)
    {
        delete farrMostRecentlyUsedPlugin;
        farrMostRecentlyUsedPlugin = 0;

        shellLinkRawPtr->Release();

        CoUninitialize();

        // success
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
//-----------------------------------------------------------------------




//-----------------------------------------------------------------------
BOOL MyPlugin_GetStrVal(const char* varname,char *destbuf, int /*maxlen*/)
{
    // FARR: default values for FARR fields
    if (strcmp(varname,DEF_FieldName_RegexStr) == 0)
    {
        strcpy(destbuf,ThisPlugin_FARR_DefaultRegex);
        return TRUE;
    }
    if (strcmp(varname,DEF_FieldName_RegexFilterStr) == 0)
    {
        strcpy(destbuf,ThisPlugin_FARR_DefaultRegexFilter);
        return TRUE;
    }
    if (strcmp(varname,DEF_FieldName_KeywordStr) == 0)
    {
        strcpy(destbuf,ThisPlugin_FARR_DefaultKeywordStr);
        return TRUE;
    }
    if (strcmp(varname,DEF_FieldName_ScoreStr) == 0)
    {
        strcpy(destbuf,ThisPlugin_FARR_DefaultScoreStr);
        return TRUE;
    }
    if (strcmp(varname,DEF_FieldName_AliasStr) == 0)
    {
        strcpy(destbuf,ThisPlugin_FARR_DefaultAlias);
        return TRUE;
    }


    // not found
    return FALSE;
}
//-----------------------------------------------------------------------



//-----------------------------------------------------------------------
BOOL MyPlugin_SetStrVal(const char* varname, void *val)
{
    // farr host will pass us function pointer we will call
    if (strcmp(varname, DEF_FieldName_NotifySearchCallbackFp) == 0)
        callbackfp_notifysearchstatechanged = (Fp_GlobalPluginCallback_NotifySearchStateChanged)val;

    return FALSE;
}
//-----------------------------------------------------------------------



//-----------------------------------------------------------------------
BOOL MyPlugin_SupportCheck(const char* testname, int /*version*/)
{
    // ATTN: we support farr interface
    if (strcmp(testname, DEF_FARRAPI_IDENTIFIER) == 0)
        return TRUE;

    // otherwise we don't support it
    return FALSE;
}
//-----------------------------------------------------------------------




//-----------------------------------------------------------------------
BOOL MyPlugin_DoAdvConfig()
{
    // success

    farrMostRecentlyUsedPlugin->showOptions();

    return TRUE;
}


BOOL MyPlugin_DoShowReadMe()
{
    // by default show the configured readme file
    std::string fname = std::string(dlldir) + std::string("\\") + std::string(ThisPlugin_ReadMeFile);
    ShellExecuteA(NULL, "open", fname.c_str(), NULL, NULL, SW_SHOWNORMAL);

    // success
    return TRUE;
}
//-----------------------------------------------------------------------



//-----------------------------------------------------------------------
BOOL MyPlugin_SetState(int /*newstateval*/)
{
    // usually there is nothing to do here

    // success
    return TRUE;
}
//-----------------------------------------------------------------------

























//-----------------------------------------------------------------------
// These functions are FARR-SPECIFIC and have no counterparts in the
//  generic DLL shell.
//
// Note from the function declarations that these functions are EXPORTED
//  from the DLL so that the host can call them.
//-----------------------------------------------------------------------


//-----------------------------------------------------------------------
// Host informs us that a search is begining and what the searchstring is
// The Plugin DLL could do all "searching" here (if there is indeed any to do)
// And compute all results within this function, OR simply begin a threaded asynchronous search now.
//
// Normally the string you care about is searchstring_lc_nokeywords, thats the search string with all special +modifier keywords removed, and
//  the alias keyword for this plugin removed, so all thats left is the string that should effect the results of the search.
//
// returns TRUE only if the plugin decides now that no more searching should be done by any other plugin or builtin
//
// NOTE: if asynchronous searching is to be done, make sure to only set current_searchstate=E_SearchState_Stopped when done!
//
PREFUNCDEF BOOL EFuncName_Inform_SearchBegins(const char* searchstring_raw, const char* searchstring_lc_nokeywords, BOOL explicitinvocation)
{
    if (!explicitinvocation)
        return FALSE;

    OutputDebugString("EFuncName_Inform_SearchBegins");

    return DoFarrSearchBegin(searchstring_raw, searchstring_lc_nokeywords);
}
//-----------------------------------------------------------------------





//-----------------------------------------------------------------------
// Host informs us that regular expression match **HAS** occured, and to begin doing that we need to do on such a match
// The Plugin DLL could do all "searching" here (if there is indeed any to do)
// And compute all results within this function, OR simply begin a threaded asynchronous search now.
//
// To access the group capture strings, use regexcharps[1] to match the first group, etc.
//
// returns TRUE only if the plugin decides now that no more searching should be done by any other plugin or builtin
//
// NOTE: if asynchronous searching is to be done, make sure to only set current_searchstate=E_SearchState_Stopped when done!
//
PREFUNCDEF BOOL EFuncName_Inform_RegexSearchMatch(const char* searchstring_raw, const char* searchstring_lc_nokeywords, int /*regexgroups*/, char** /*regexcharps*/)
{
    OutputDebugString("EFuncName_Inform_RegexSearchMatch");

    // call farr search - nothing to do really for this plugin but return all results
    return DoFarrSearchBegin(searchstring_raw, searchstring_lc_nokeywords);
}
//-----------------------------------------------------------------------





//-----------------------------------------------------------------------
// Host informs us that a search has completed or been interrupted, and we should stop any processing we might be doing assynchronously
//
// NOTE: make sure to set current_searchstate=E_SearchState_Stopped;
//
PREFUNCDEF void EFuncName_Inform_SearchEnds()
{
    // stop search and set this
    current_searchstate = E_SearchState_Stopped;

    // ATTN: test clear results
    numresultsavailable = 0;
    resultsavailabletype = E_ResultAvailableState_None;

    // notify host that our state has changed
    ExecuteCallback_SearchStateChanged();
}
//-----------------------------------------------------------------------



//-----------------------------------------------------------------------
// Host informs us that our window is about to appear
PREFUNCDEF void EFuncName_Inform_WindowIsHiding(HWND /*hwndp*/)
{
}


// Host informs us that our window is about to disappear
PREFUNCDEF void EFuncName_Inform_WindowIsUnHiding(HWND /*hwndp*/)
{
}
//-----------------------------------------------------------------------



//-----------------------------------------------------------------------
// Host is asking us if we want a certain feature or call
// We should use a switch function here to tell what is being asked
//
// Returns TRUE or FALSE depending on whether we want the feature
//
PREFUNCDEF BOOL EFuncName_Ask_WantFeature(E_WantFeaturesT featureid)
{
    switch (featureid)
    {
    case E_WantFeatures_searchinput_regex:
        // do we want to search on regular expression matches
        return TRUE;
    case E_SupportFeatures_searchinput_explicit:
        // do we want to search on alias match
        return TRUE;
    case E_SupportFeatures_searchinput_all:
        // do we want to search on all search expressions
        return FALSE;
    case E_SupportFeatures_tryhandle_trigger:
        // do we want to try to handle triggers when a result is launched
        return FALSE;
    case E_SupportFeatures_addinfo_files:
        // do we want to try to handle triggers when a result is launched
        return FALSE;
    case E_SupportFeatures_scoreall_files:
        // do we want to try to score EVERY file considered (SLOWS DOWN SEARCH!)
        return FALSE;
    case E_SupportFeatures_scoregood_files:
        // do we want to try to score files after other scoring has determined they are a viable result
        return FALSE;
    case E_SupportFeatures_dominate_results:
        // do we want our results to dominate and hide any others
        return FALSE;
    }

    // fell through - so not supported
    return FALSE;
}
//-----------------------------------------------------------------------



//-----------------------------------------------------------------------
// Host wants to know the search state of the plugin
// returns:  E_SearchState_Stopped=0 || E_SearchState_Searching=1
//
// NOTE: this value will be based on a dll global variable
//
PREFUNCDEF E_SearchStateT EFuncName_Ask_SearchState()
{
    // this will be tracked elsewhere
    return current_searchstate;
}
//-----------------------------------------------------------------------



//-----------------------------------------------------------------------
// Host wants to know what kind if any results we have available
//
// NOTE: return is from (see JrPluginFuncs_FARR.h) enum E_ResultAvailableStateT { E_ResultAvailableState_None=0, E_ResultAvailableState_ItemResuls=1, E_ResultAvailableState_DisplayText=2, E_ResultAvailableState_DisplayHtml=3, E_ResultAvailableState_Grid=4, E_ResultAvailableState_WinTakeover=5 };
//
PREFUNCDEF E_ResultAvailableStateT EFuncName_Ask_IsAnyResultAvailable()
{
    // this will be tracked elsewhere
    return resultsavailabletype;
}


// Host wants to know how many item results are available
PREFUNCDEF int EFuncName_Ask_HowManyItemResultsAreReady()
{
    // this will be tracked elsewhere
    return numresultsavailable;
}
//-----------------------------------------------------------------------



//-----------------------------------------------------------------------
// Host calls this before iterating through the result requests
// It will be TRUE on lock before retrieving
//  and then FALSE when all done retrieving
// In this way the dll should refrain from altering the results until retrieval is done
// One way to do this for asynchronous searching is to MOVE results to a return store
//  on TRUE lock, and then free on FALSE.
//
// Returns TRUE on success
//
PREFUNCDEF BOOL EFuncName_Request_LockResults(BOOL dolock)
{
    // set lock
    current_lockstate = dolock;

    if (dolock == false)
    {
        // on unlocking they have retrieved all the results, so CLEAR the results or they will be found again
        numresultsavailable = 0;
        resultsavailabletype = E_ResultAvailableState_None;
        // notify host that our state has changed
        ExecuteCallback_SearchStateChanged();
    }

    // success
    return TRUE;
}
//-----------------------------------------------------------------------




//-----------------------------------------------------------------------
// Host is requesting one of our results
// Fill the destination buffers without exceding maxlen
//
// resultpostprocmodep is from enum E_ResultPostProcessingT { E_ResultPostProcessing_ImmediateDisplay = 0, E_ResultPostProcessing_AddScore = 1, E_ResultPostProcessing_MatchAgainstSearch = 2};
//
// Returns TRUE on success
//
PREFUNCDEF BOOL EFuncName_Request_ItemResultByIndex(int resultindex, char *destbuf_path, char *destbuf_caption, char *destbuf_groupname, char *destbuf_iconfilename, void** tagvoidpp, int maxlen, E_ResultPostProcessingT *resultpostprocmodep, int* /*scorep*/, E_EntryTypeT *entrytypep)
{
    // clear values sample:
    *tagvoidpp = NULL;

    // how result is handled
    // return E_ResultPostProcessing_ImmediateDisplay if we have done the filtering and the result should be displayed
    // return E_ResultPostProcessing_AddScore if the host should evaluate the result and add a score to it but still be forced to display it
    // return E_ResultPostProcessing_MatchAgainstSearch if the host should evaluate the result and decide whether to show it based on match of search words
    //
    // in this case we just return all bookmarks and let host do the matching
    *resultpostprocmodep = (farrMostRecentlyUsedPlugin->noSorting() ? E_ResultPostProcessing_MatchAgainstSearch : E_ResultPostProcessing_ImmediateDisplay);

    // do we want to modify the score assigned by host?  usually not.
    // *scorep+=100;

    // ok fill the result data

    const Item& item = farrMostRecentlyUsedPlugin->getItem(resultindex);

    // type (file is same as url)
    switch(item.type)
    {
    case Item::Type_File:
        *entrytypep = E_EntryType_FILE;
        break;

    case Item::Type_Folder:
        *entrytypep = E_EntryType_FOLDER;
        break;

    case Item::Type_URL:
        *entrytypep = E_EntryType_URL;
        break;

    case Item::Type_Alias:
        *entrytypep = E_EntryType_ALIAS;
        break;

    default:
        *entrytypep = E_EntryType_UNKNOWN;
        break;
    }

    if(item.type == E_EntryType_ALIAS)
    {
        strncpy(destbuf_groupname, item.groupName.c_str(), maxlen);

        const std::string& pathName = item.path;
        const std::string::size_type pipePos = pathName.find('|');

        strncpy(destbuf_caption, pathName.substr(0, pipePos).c_str(), maxlen);
        strncpy(destbuf_path, pathName.substr(pipePos + 1).c_str(), maxlen);
    }
    else
    {
        const bool showGroupName = farrMostRecentlyUsedPlugin->showGroupName();

        strncpy(destbuf_groupname, showGroupName ? item.groupName.c_str() : "", maxlen);
        strncpy(destbuf_caption, PathFindFileName(item.path.c_str()), maxlen);
        strncpy(destbuf_path, item.path.c_str(), maxlen);

        //if(PathIsUNC(item.path.c_str()))
        //{
        //    strncpy(destbuf_iconfilename, getIconPathForUNCFile(item.path).c_str(), maxlen);
        //}
        //else if(item.type == Item::Type_URL)
        //{
        //    strncpy(destbuf_iconfilename, getIconPathForUNCFile("a.html").c_str(), maxlen);
        //}
        //else
        //{
        strncpy(destbuf_iconfilename, item.path.c_str(), maxlen);
        //}
    }

    // ok filled one
    return TRUE;
}
//-----------------------------------------------------------------------




//-----------------------------------------------------------------------
// Host is asking us to return a char* to a LONG result string
//
// NOTE: It is up to us to free this char* when the lock is set to false above OR
//  on the next call, or on search end, but we MUST keep it valid until then.
PREFUNCDEF BOOL EFuncName_Request_TextResultCharp(char **charp)
{
    // signify we have none
    *charp=NULL;

    // none returned
    return FALSE;
}
//-----------------------------------------------------------------------



//-----------------------------------------------------------------------
// Host is calling us with a result item about to be triggered
// We can choose here to takeover the launching of it, OR simply do something and let normal launching continue
//
// Return TRUE to takeover launching and prevent all other further launching
// or FALSE to continue launching after we return
//
PREFUNCDEF BOOL EFuncName_Allow_ProcessTrigger(const char* /*destbuf_path*/, const char* /*destbuf_caption*/, const char* /*destbuf_groupname*/, int /*pluginid*/, int /*thispluginid*/, int /*score*/, E_EntryTypeT /*entrytype*/, void* /*tagvoidp*/, BOOL* /*closeafterp*/)
{
    // does this plugin want to take over launching of this result?
    return FALSE;
}
//-----------------------------------------------------------------------



//-----------------------------------------------------------------------
// Host is asking us if we want to modify the score of an item
// Original score in in *score, we can now modify it if we want
//
// Return TRUE if we modified it, or FALSE if not
//
// NOTE: set score to -9999 to eliminate it from results
//
PREFUNCDEF BOOL EFuncName_Do_AdjustResultScore(const char* /*itempath*/, int* /*scorep*/)
{
    // we didnt modify it
    return FALSE;
}
//-----------------------------------------------------------------------




























//---------------------------------------------------------------------------
// Helper Functions the plugin calls
//---------------------------------------------------------------------------





//-----------------------------------------------------------------------

void ExecuteCallback_SearchStateChanged()
{
    // tell the host that our state or resultcount has changed
    if (callbackfp_notifysearchstatechanged)
    {
        callbackfp_notifysearchstatechanged(hostptr,numresultsavailable,current_searchstate);
    }
}

//-----------------------------------------------------------------------

BOOL DoFarrSearchBegin(const char* searchstring_raw, const char* /*searchstring_lc_nokeywords*/)
{
    // FARR search -- all we really do is return all bookmarks in our store, and let the farr program filter them based on what user types
    if (!isready)
    {
        // statusbar
        std::string busymsg = std::string(ThisPlugin_DisplayName) + std::string(" ") + std::string(_T("is busy.."));
        callbackfp_set_strval(hostptr, "statusbar", (TCHAR*)busymsg.c_str());
        // search can continue by others?
        return FALSE;
    }

    // start and end of search state
    current_searchstate = E_SearchState_Searching;

    ExecuteCallback_SearchStateChanged();

    //
    farrMostRecentlyUsedPlugin->search(searchstring_raw);

    // ok now results are available right away
    // IMPORTANT: here is where you modify the code to specify how many results are available
    numresultsavailable = (int)farrMostRecentlyUsedPlugin->getItemCount();
    resultsavailabletype = E_ResultAvailableState_ItemResuls;

    // done
    current_searchstate = E_SearchState_Stopped;

    // notify host that our state has changed
    ExecuteCallback_SearchStateChanged();

    // search can continue by others
    return FALSE;
}

//-----------------------------------------------------------------------

std::string getIconPathForUNCFile(const std::string& uncPath)
{
    std::string pathToIcon;

    RegistryKey extensionKey;
    if(extensionKey.open(HKEY_CLASSES_ROOT, PathFindExtension(uncPath.c_str()), KEY_READ))
    {
        const DWORD MaxApplicationNameLength = 100;
        char applicationName[MaxApplicationNameLength] = { 0 };
        DWORD type;
        DWORD length = MaxApplicationNameLength;
        if(extensionKey.queryValue(0, &type, (BYTE*)applicationName, &length))
        {
            RegistryKey applicationKey;
            if(applicationKey.open(HKEY_CLASSES_ROOT, applicationName, KEY_READ))
            {
                RegistryKey defaultIconKey;
                if(defaultIconKey.open(applicationKey, "DefaultIcon", KEY_READ))
                {
                    char pathToIconBuffer[MAX_PATH];
                    DWORD type;
                    DWORD length = MAX_PATH;
                    if(defaultIconKey.queryValue(0, &type, (BYTE*)pathToIconBuffer, &length))
                    {
                        pathToIcon = pathToIconBuffer;
                        std::string::size_type commaPos = pathToIcon.rfind(',');
                        if(commaPos != std::string::npos)
                        {
                            pathToIcon.erase(commaPos);

                            OutputDebugString(pathToIcon.c_str());
                        }
                    }
                }
            }

        }
    }

    return pathToIcon;
}

//-----------------------------------------------------------------------
