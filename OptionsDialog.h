#include "resource.h"

#include "OptionsFile.h"

#include <sstream>

//-----------------------------------------------------------------------

class OptionsDialog : public CDialogImpl<OptionsDialog>
{
public:
    enum { IDD = IDD_DIALOG_OPTIONS };

    OptionsDialog(OptionsFile& optionsFile)
        :_optionsFile(optionsFile)
    {}

    BEGIN_MSG_MAP(OptionsDialog)
        MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
        COMMAND_ID_HANDLER(IDOK, onOk)
        COMMAND_ID_HANDLER(IDCANCEL, onCancel)
    END_MSG_MAP()

private:
    LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        initializeCheckBox(_removeUNCFiles, IDC_CHECK_REMOVE_UNC, "RemoveUNCFiles");
        initializeCheckBox(_removeDirectories, IDC_CHECK_REMOVE_DIRECTORIES, "RemoveDirectories");
        initializeCheckBox(_simpleDirectoryCheck, IDC_CHECK_SIMPLE_DIR_CHECK, "SimpleDirectoryCheck");
        initializeCheckBox(_removeNonExistingFiles, IDC_CHECK_REMOVE_NON_EXISTENT, "RemoveNonexistentFiles");
        initializeCheckBox(_ignoreExistenceCheckUNCPaths, IDC_CHECK_DONT_CHECK_UNC_EXISTENCE, "IgnoreExistenceCheckUNCFiles");

        initializeCheckBox(_showGroupName, IDC_CHECK_SHOW_GROUP_NAME, "ShowGroupName");

        _sortNone = GetDlgItem(IDC_RADIO_NOSORTING);
        _sortLastAccessed = GetDlgItem(IDC_RADIO_SORT_DATE_ACCESSED);
        _sortLastModified = GetDlgItem(IDC_RADIO_SORT_DATE_MODIFIED);
        _sortCreated = GetDlgItem(IDC_RADIO_SORT_DATE_CREATED);
        _sortAlphabetically = GetDlgItem(IDC_RADIO_SORT_ALPHA);

        switch(_optionsFile.getValue("SortMode", 1L))
        {
        case Options::Sort_NoSorting:
            _sortNone.SetCheck(BST_CHECKED);
            break;

        case Options::Sort_TimeLastAccessed:
            _sortLastAccessed.SetCheck(BST_CHECKED);
            break;

        case Options::Sort_TimeLastModified:
            _sortLastModified.SetCheck(BST_CHECKED);
            break;

        case Options::Sort_TimeCreated:
            _sortCreated.SetCheck(BST_CHECKED);
            break;

        case Options::Sort_Alphabetically:
            _sortAlphabetically.SetCheck(BST_CHECKED);
            break;
        }

        return 0;
    }

    LRESULT onOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        storeCheckBoxValue(_removeUNCFiles, "RemoveUNCFiles");
        storeCheckBoxValue(_removeDirectories, "RemoveDirectories");
        storeCheckBoxValue(_simpleDirectoryCheck, "SimpleDirectoryCheck");
        storeCheckBoxValue(_removeNonExistingFiles, "RemoveNonexistentFiles");
        storeCheckBoxValue(_ignoreExistenceCheckUNCPaths, "IgnoreExistenceCheckUNCFiles");

        storeCheckBoxValue(_showGroupName, "ShowGroupName");

        if(_sortNone.GetCheck() == BST_CHECKED)
        {
            _optionsFile.setValue("SortMode", (long)Options::Sort_NoSorting);
        }
        else if(_sortLastAccessed.GetCheck() == BST_CHECKED)
        {
            _optionsFile.setValue("SortMode", (long)Options::Sort_TimeLastAccessed);
        }
        else if(_sortLastModified.GetCheck() == BST_CHECKED)
        {
            _optionsFile.setValue("SortMode", (long)Options::Sort_TimeLastModified);
        }
        else if(_sortCreated.GetCheck() == BST_CHECKED)
        {
            _optionsFile.setValue("SortMode", (long)Options::Sort_TimeCreated);
        }
        else if(_sortAlphabetically.GetCheck() == BST_CHECKED)
        {
            _optionsFile.setValue("SortMode", (long)Options::Sort_Alphabetically);
        }

        EndDialog(IDOK);

        return 0;
    }

    LRESULT onCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        EndDialog(IDCANCEL);

        return 0;
    }

    void initializeCheckBox(CButton& button, int id, const char* optionName)
    {
        button = GetDlgItem(id);
        button.SetCheck(_optionsFile.getValue(optionName, false) ? BST_CHECKED : BST_UNCHECKED);
    }

    void storeCheckBoxValue(const CButton& button, const char* optionName)
    {
        _optionsFile.setValue(optionName, (button.GetCheck() == BST_CHECKED));
    }

private:
    OptionsFile& _optionsFile;

    CButton _removeUNCFiles;
    CButton _removeDirectories;
    CButton _simpleDirectoryCheck;
    CButton _removeNonExistingFiles;
    CButton _ignoreExistenceCheckUNCPaths;

    CButton _sortNone;
    CButton _sortLastAccessed;
    CButton _sortLastModified;
    CButton _sortCreated;
    CButton _sortAlphabetically;

    CButton _showGroupName;
};

//-----------------------------------------------------------------------
