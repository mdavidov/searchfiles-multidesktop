# TODO

## Features to be developed

1. Match whole words or not
1. Match by regular expressions
1. Search by file extension
1. Search by file size (range)
1. Search by file modification and creation dates (ranges)
1. Search within results
1. Add back & forward buttons (to the navigation layout, which is above Search Folder control)
1. READ-ONLY files when deleting: warn the user and/or remove read-only attrib
1. License

## Performance improvements

1. Read in the whole file (up to 100 MB) when searching for words
1. Exclude binary files when searching for words
    * will be selectable by user
    * exclude by extension (.exe, .dll, .o, .so, .obj, .dylib, etc.)
    * exclude by file type (use command like file, otool, etc.)

## DONE

1. WHEN ENTER KEY IS PRESSED: START THE SEARCH !!!
1. INSTALLER
1. ABOUT DIALOG
1. ADD TEXT BOX "Search folder {SF}:" to the left of directory combo box
1. STATICALLY LINK ALL LIBS
