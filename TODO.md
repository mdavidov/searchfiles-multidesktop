# TODO

## Features to be developed

1. Match whole words only
1. Match by regular expressions
1. Search by file size (range)
1. Search by file modification and creation dates (range)
1. Search within results
1. Add back & forward buttons to the navigation layout
1. Read-only files when deleting: warn the user and/or remove read-only attribute
1. License

## Performance improvements

1. DONE: Read in the whole file (up to 200 MB) when searching for words
1. Exclude binary files when searching for words
    * DONE: exclude by file extension == exclude by partial file name (.exe, .dll, .o, .so, .obj, .dylib, etc.)
    * exclude by file type (use command like file, otool, etc.)
