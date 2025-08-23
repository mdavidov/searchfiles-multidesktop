#pragma once
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) Milivoj (Mike) DAVIDOV
// All rights reserved.
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
// EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
//
/////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QString>
QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

#define OvSk_FsOp_TOT_TIMER             1000
#define OvSk_FsOp_PER_TIMER             250
#define OvSk_FsOp_GUESS_PROGRESS        20

#define OvSk_FsOp_NEW_LINE              QString("\n")

#define OvSk_FsOp_PROGRESS_TXT          OvSk_FsOp_APP_NAME_TXT
#define OvSk_FsOp_DUPLICATE_TXT         tr("Duplicate")
#define OvSk_FsOp_COPY_2OTHER_TXT       tr("Copy to Other Pane")
#define OvSk_FsOp_MOVE_2OTHER_TXT       tr("Move to Other Pane")
#define OvSk_FsOp_DELETE_2TRASH_TXT     tr("Move to Trash / Recycle Bin")
#define OvSk_FsOp_DELETE_PERM_TXT       tr("Delete Permanently")
#define OvSk_FsOp_SHRED_PERM_TXT        tr("Shred & Delete Permanently")

#define OvSk_FsOp_SELECT_FILES_TXT      tr("Please select some file system items (i.e. files, folders, and/or shortcuts/symlinks) first.")
#define OvSk_FsOp_SELECT_FOLDER_TXT     tr("Please select a folder to search.")
#define OvSk_FsOp_SELECT_1_FOLDER_TXT   tr("Please select only one folder to search.")
#define OvSk_FsOp_SELECT_FOUNDFILES_TXT tr("Please select some found items in the table first.")
#define OvSk_FsOp_SELECT_TOOPEN_TXT     tr("Please select a file or folder to open/run.")

#define OvSk_FsOp_CANCEL_TXT            tr("Cancel")

#define OvSk_FsOp_ANALYZING_TXT         tr("Getting size info...")
#define OvSk_FsOp_NBR_FILES_TXT         tr("Files: %1")
#define OvSk_FsOp_NBR_FOLDERS_TXT       tr("Folders: %1")
#define OvSk_FsOp_TOTAL_SIZE_TXT        tr("Total size: %1")
#define OvSk_FsOp_ELAPSED_TIME_TXT      " Elapsed time: %1"

#define OvSk_FsOp_PERFORMING_TXT        tr("Performing the requested operation on file system items...")

#define OvSk_FsOp_DUPLICATE_SUFFIX_TXT  tr(".copy")

#define OvSk_FsOp_WANT_TO_COPY_Q        tr("Do you want to copy selected items?")
#define OvSk_FsOp_COPY_COMPLETE_TXT     tr("Copy complete.")

#define OvSk_FsOp_COPYING_TXT           tr("Copying items...")
#define OvSk_FsOp_DUPLICATING_TXT       tr("Duplicating items...")
#define OvSk_FsOp_MOVING_TXT            tr("Moving items...")
#define OvSk_FsOp_DELETING_2TRASH_TXT   tr("Moving items to Trash...")
#define OvSk_FsOp_DELETING_PERM_TXT     tr("Deleting items...")
#define OvSk_FsOp_SHREDDING_PERM_TXT    tr("Shredding items...")


// ACTIONS

#define OvSk_FsOp_GO_BACK_ACT_TXT       tr("&Back")
#define OvSk_FsOp_GO_BACK_STS_TIP       tr("Go to the previous folder in navigation history.")

#define OvSk_FsOp_GO_FORWARD_ACT_TXT    tr("&Forward")
#define OvSk_FsOp_GO_FORWARD_STS_TIP    tr("Go to the next folder in navigation history.")

#define OvSk_FsOp_GO_UP_ACT_TXT         tr("&Up")
#define OvSk_FsOp_GO_UP_STS_TIP         tr("Go to the parent folder.")

#define OvSk_FsOp_GO_HOME_ACT_TXT       tr("&Home")
#define OvSk_FsOp_GO_HOME_STS_TIP       tr("Go to the home folder.")

#define OvSk_FsOp_FINDFILES_ACT_TXT     tr("&Search")
#define OvSk_FsOp_FINDFILES_STS_TIP     tr("Search for files, folders and shortcuts/symlinks by name, and/or files containing some text.")

#define OvSk_FsOp_SELECTALL_ACT_TXT     tr("Select &All")
#define OvSk_FsOp_SELECTALL_STS_TIP     tr("Select all items in the current view.")

#define OvSk_FsOp_FAST_COPY_ACT_TXT     tr("&Smart Copy selected items")
#define OvSk_FsOp_FAST_COPY_STS_TIP     tr("Copy selected file system items, skipping files with same size and date modified.")

#define OvSk_FsOp_SIMPLE_COPY_ACT_TXT   tr("Simple C&opy selected items")
#define OvSk_FsOp_SIMPLE_COPY_STS_TIP   tr("Copy all selected file system items to the opposite pane.")

#define OvSk_FsOp_DUPLICATE_ACT_TXT     tr("Duplicate selected items")
#define OvSk_FsOp_DUPLICATE_STS_TIP     tr("Duplicate selected file system items in the same pane/folder.")


#define OvSk_WARN_SHRED_NO_RECOVER      "WARNING: Contents and names of shredded items cannot be restored, "                \
                                        "undeleted or recovered.\n\n"

#define OvSk_WARN_DEL_NO_UNDEL          "WARNING: Deleted items cannot be undeleted. In some cases, using data recovery "   \
                                        "tools, it might be possible to recover them. \n\n"

#define OvSk_NOTE_RECURSIVE             "NOTE: When a folder is selected, all items it contains (files, shortcuts/symlinks "         \
                                        "and sub-folders) will be recursively "

#define OvSk_CONFIRM_SHRED1_TXT         tr("Are you sure you want to SHRED and permanently delete the selected item?\n\n"   \
                                           "%1\n\n"                                                                         \
                                           OvSk_WARN_SHRED_NO_RECOVER                                                       \
                                           OvSk_NOTE_RECURSIVE "shredded. ")

#define OvSk_CONFIRM_SHRED_TXT          tr("Are you sure you want to SHRED and permanently delete selected items?\n\n"      \
                                           OvSk_WARN_SHRED_NO_RECOVER                                                       \
                                           OvSk_NOTE_RECURSIVE "shredded. ")

#define OvSk_CONFIRM_DELPERM1_TXT        tr("Are you sure you want to permanently delete the selected item?\n\n"            \
                                           "%1\n\n"                                                                         \
                                           OvSk_WARN_DEL_NO_UNDEL                                                           \
                                           OvSk_NOTE_RECURSIVE "deleted. ")

#define OvSk_CONFIRM_DELPERM_TXT        tr("Are you sure you want to permanently delete selected items?\n\n"                \
                                           OvSk_WARN_DEL_NO_UNDEL                                                           \
                                           OvSk_NOTE_RECURSIVE "deleted. ")

#if defined(Q_OS_MAC)
#define OvSk_DEL2TRASH_ACT_TXT          tr("&Trash")
#define OvSk_DEL2TRASH_STS_TIP          tr("Move selected file system items to Trash.")
#define OvSk_CONFIRM_DEL2TRASH1_TXT     tr("Are you sure you want to move the selected item to Trash?\n%1")
#define OvSk_CONFIRM_DEL2TRASH_TXT      tr("Are you sure you want to move selected items to Trash?")
#define OvSk_FsOp_NBR_SYMLINKS_TXT      tr("Symbolic links: %1")
#define OvSk_FsOp_SYMLINK_TXT           "Symlink"
#define OvSk_FsOp_SYMLINKS_TXT          "Symlinks"
#define OvSk_FsOp_SYMLINKS_LOW          "symlinks"
#elif defined(_WIN32) || defined(_WIN64)
#define OvSk_DEL2TRASH_ACT_TXT          tr("&Recycle")
#define OvSk_DEL2TRASH_STS_TIP          tr("Move selected items to the Recycle Bin.")
#define OvSk_CONFIRM_DEL2TRASH1_TXT     tr("Are you sure you want to move the selected item to the Recycle Bin?\n%1")
#define OvSk_CONFIRM_DEL2TRASH_TXT      tr("Are you sure you want to move selected items to the Recycle Bin?")
#define OvSk_FsOp_NBR_SYMLINKS_TXT      tr("Shortcuts: %1")
#define OvSk_FsOp_SYMLINK_TXT           "Shortcut"
#define OvSk_FsOp_SYMLINKS_TXT          "Shortcuts"
#define OvSk_FsOp_SYMLINKS_LOW          "shortcuts"
#else
#define OvSk_DEL2TRASH_ACT_TXT          tr("&Trash")
#define OvSk_DEL2TRASH_STS_TIP          tr("Move selected file system items to Trash.")
#define OvSk_CONFIRM_DEL2TRASH1_TXT     tr("Are you sure you want to move the selected item to Trash?\n%1")
#define OvSk_CONFIRM_DEL2TRASH_TXT      tr("Are you sure you want to move selected items to Trash?")
#define OvSk_FsOp_NBR_SYMLINKS_TXT      tr("Symbolic links: %1")
#define OvSk_FsOp_SYMLINK_TXT           "Symlink"
#define OvSk_FsOp_SYMLINKS_TXT          "Symlinks"
#define OvSk_FsOp_SYMLINKS_LOW          "symlinks"
#endif

#define OvSk_FsOp_DELETEPERM_ACT_TXT    tr("&Delete selected items")
#define OvSk_FsOp_DELETEPERM_STS_TIP    tr("Permanently delete selected files/folders/shortcuts " \
                                           "- CANNOT BE UNDONE!")

#define OvSk_FsOp_SHREDPERM_ACT_TXT     tr("S&hred selected")
#define OvSk_FsOp_SHREDPERM_STS_TIP     tr("Shred and permanently delete selected files/folders/shortcuts " \
                                           "- CANNOT BE UNDONE OR RECOVERED!")

#define OvSk_FsOp_GO_MOVE_ACT_TXT       tr("&Move selected")
#define OvSk_FsOp_GO_MOVE_STS_TIP       tr("Move selected file system items to the opposite pane.")

#define OvSk_FsOp_GO_INFO_ACT_TXT       tr("Size &info on selected")
#define OvSk_FsOp_GO_INFO_STS_TIP       tr("Show total size information and number of selected files/folders/shortcuts.")

#define OvSk_FsOp_OPENRUN_ACT_TXT       tr("Open")
#define OvSk_FsOp_OPENRUN_STS_TIP       tr("Open (or execute) the selected file or browse into the folder.")

#define eCod_OPEN_CONT_FOLDER_ACT_TXT   tr("Open folder")
#define eCod_OPEN_CONT_FOLDER_STS_TIP   tr("Open the folder containing the selected file or folder.")

#define eCod_COPY_PATH_ACT_TXT          tr("Copy path")
#define eCod_COPY_PATH_STS_TIP          tr("Copy the full, absolute path to the system clipboard.")

#define eCod_GET_SIZE_ACT_TXT           tr("Get size")
#define eCod_GET_SIZE_STS_TIP           tr("Get the size of the selected file or folder.")

#define eCod_PROPERTIES_ACT_TXT         tr("Properties")
#define eCod_PROPERTIES_STS_TIP         tr("Show file/folder properties.")

#define OvSk_FsOp_HELPLOCAL_ACT_TXT     tr("User manual")
#define OvSk_FsOp_HELPLOCAL_STS_TIP     tr("Open the locally installed or online user manual.")
#define OvSk_FsOp_HELPONLINE_DOC        QString("http://") + QString(OvSk_FsOp_COPMANY_DOMAIN_TXT) + QString("/")

#define OvSk_FsOp_REFRESH_ACT_TXT       tr("Refresh")
#define OvSk_FsOp_REFRESH_STS_TIP       tr("Refresh the file system view.")

#define OvSk_FsOp_HELPABOUT_ACT_TXT     tr("About " OvSk_FsOp_APP_NAME_TXT)
#define OvSk_FsOp_HELPABOUT_STS_TIP     tr("Display version and general info about this app.")

#define OvSk_FsOp_DRIVE_REMOVED_TXT     tr("The item does not seem to exist (any more). Please make sure that " \
                                           "the removable/network drive is properly inserted/connected.")
#define OvSk_FsOp_GEN_ERR_TXT           tr("An error occurred while processing the item.")
#define OvSk_FsOp_COPY_ERR_TXT          tr("An error occurred while copying the item.")
#define OvSk_FsOp_DEL_ERR_TXT           tr("An error occurred while deleting the item.")
#define OvSk_FsOp_SHRED_ERR_TXT         tr("An error occurred while shredding the item.")
#define OvSk_FsOp_SHRED_NAME_ERR_TXT    tr("An error occurred while shredding the item's name.")

#define eCod_SEARCH_BY_TYPE_TIP         tr("Specify which type(s) of items to search for: folders, files, and/or shortcuts.")
#define OvSk_FsOp_NAME_FILTERS_TIP      tr("Space separated file/folder/shortcut name patterns to search for. Search all items if empty.")
#define OvSk_FsOp_CONTAINING_TEXT_TIP   tr("Text to search for in each file's contents. Not applicable to folders. ")
#define OvSk_FsOp_TOP_DIR_TIP           tr("Search Folder {SF}: The folder to deeply/recursively search. ")
#define eCod_EXCL_FILES_BY_NAME_TIP     tr("Exclude files whose name contains this text (case insensitive, simple text, no wild-cards).")
#define eCod_EXCL_FOLDERS_BY_NAME_TIP   tr("Exclude folders whose name equals this text (case insensitive).")
#define eCod_EXCL_FILES_BY_CONTENT_TIP  tr("Exclude files that contain this text.")
#define eCod_EXCL_HIDDEN_ITEMS          tr("Exclude hidden folders, files and shortcuts. Note: ALL sub-folders, files and shortcuts (hidden or not) under a hidden folder are also excluded.")
#define eCod_SHOW_EXCL_OPTS_TIP         tr("Hide exclusion options.")
#define eCod_HIDE_EXCL_OPTS_TIP         tr("Show exclusion options.")
#define eCod_BROWSE_FOLDERS_TIP         tr("Use the system dialog to select a folder and set it as the search folder.")
#define eCod_BROWSE_GO_UP_TIP           tr("Go up in the folder hierarchy and set parent as the search folder.")
#define OvSk_FsOp_DIR_NOT_EXISTS_TXT    tr("The selected folder could not be found. Please check the whole path, and if you are using a removable or network drive make sure it's properly inserted or connected. Not found:\n\n")
#define OvSk_FsOp_SELECT_ITEM_TYPE_TXT  tr("Select at least one filesystem item type please: Files, Folders and/or Shortcuts.")


namespace mmd
{
    class Cfg
    {
    public:
        static const int productLevel;
        static const int productType;
        static const int shredder;
        static const int fileMgr;
        static const bool deepDel;

        static const QString origDirPathKey;

//        static const QString deepDelKey;

//        static const QString deletePartialKey;
//        static const QString copyPermissionsKey;
//        static const QString randomShredValuesKey;
//        static const QString lastBrowsedLeftKey;
//        static const QString lastBrowsedRightKey;
//        static const QString lastBrowsedLeftBKey;
//        static const QString lastBrowsedRightBKey;
//        static const QString useDockWidgetsKey;
//        static const QString NameFilters;

//        static const QString AppGeometry;
//        static const QString AppState;
//        static const QString View0Geometry;
//        static const QString View1Geometry;
//        static const QString View10Geometry;
//        static const QString View11Geometry;

//        static const QString column0Width;
//        static const QString column1Width;
//        static const QString column2Width;
//        static const QString column3Width;

        static QSettings & St();
    };
}
