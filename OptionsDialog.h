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
    void initializeCheckBox(CButton& button, int id, const char* optionName, bool defaultValue)
    {
        button = GetDlgItem(id);
        button.SetCheck(_optionsFile.getValue(optionName, defaultValue) ? BST_CHECKED : BST_UNCHECKED);
    }

    void storeCheckBoxValue(const CButton& button, const char* optionName)
    {
        _optionsFile.setValue(optionName, (button.GetCheck() == BST_CHECKED));
    }

    LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        initializeCheckBox(_ignoreDirectoriesCheck, IDC_CHECK_IGNORE_DIRECTORIES, "IgnoreDirectories", true);
        initializeCheckBox(_includeUNCPathsCheck, IDC_CHECK_INCLUDE_NETWORK_FILES, "IncludeUNCPaths", true);
        initializeCheckBox(_simpelDirectoryCheckCheck, IDC_CHECK_SIMPLE_DIR_CHECK, "SimpleDirectoryCheck", true);
        initializeCheckBox(_showGroupNameCheck, IDC_CHECK_SHOW_GROUP_NAME, "ShowGroupName", false);

        _sortNoneRadio = GetDlgItem(IDC_RADIO_NOSORTING);
        _sortLastAccessedRadio = GetDlgItem(IDC_RADIO_SORT_DATELASTMODIFIED);
        _sortAlphabeticallyRadio = GetDlgItem(IDC_RADIO_SORT_ALPHA);

        switch(_optionsFile.getValue("SortMode", 0L))
        {
        case 0:
            _sortNoneRadio.SetCheck(BST_CHECKED);
            break;

        case 1:
            _sortLastAccessedRadio.SetCheck(BST_CHECKED);
            break;

        case 2:
            _sortAlphabeticallyRadio.SetCheck(BST_CHECKED);
            break;
        }

        return 0;
    }

    LRESULT onOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        storeCheckBoxValue(_ignoreDirectoriesCheck, "IgnoreDirectories");
        storeCheckBoxValue(_includeUNCPathsCheck, "IncludeUNCPaths");
        storeCheckBoxValue(_simpelDirectoryCheckCheck, "SimpleDirectoryCheck");
        storeCheckBoxValue(_showGroupNameCheck, "ShowGroupName");

        if(_sortNoneRadio.GetCheck() == BST_CHECKED)
        {
            _optionsFile.setValue("SortMode", 0L);
        }
        else if(_sortLastAccessedRadio.GetCheck() == BST_CHECKED)
        {
            _optionsFile.setValue("SortMode", 1L);
        }
        else if(_sortAlphabeticallyRadio.GetCheck() == BST_CHECKED)
        {
            _optionsFile.setValue("SortMode", 2L);
        }

        EndDialog(IDOK);

        return 0;
    }
    LRESULT onCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        EndDialog(IDCANCEL);

        return 0;
    }

private:
    OptionsFile& _optionsFile;

    CButton _ignoreDirectoriesCheck;
    CButton _includeUNCPathsCheck;
    CButton _simpelDirectoryCheckCheck;
    CButton _showGroupNameCheck;

    CButton _sortNoneRadio;
    CButton _sortLastAccessedRadio;
    CButton _sortAlphabeticallyRadio;
};

//-----------------------------------------------------------------------
