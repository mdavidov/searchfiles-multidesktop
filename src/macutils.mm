#include <Cocoa/Cocoa.h>

extern "C" void showFileProperties(const char* filePath)
{
    @autoreleasepool {
        if (filePath == nullptr) {
            NSLog(@"ERROR: filePath is null");
            return;
        }

        NSString* path = [NSString stringWithUTF8String:filePath];
        if (path == nil) {
            NSLog(@"ERROR: Failed to convert filePath to NSString");
            return;
        }

        NSURL* fileURL = [NSURL fileURLWithPath:path];
        if (fileURL == nil) {
            NSLog(@"ERROR: Failed to create NSURL from path");
            return;
        }

        // if (![[NSWorkspace sharedWorkspace] showPropertiesForFile:fileURL]) {
        //     NSLog(@"ERROR: Failed to show properties for file at %@", path);
        // }
        // if (![[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:fileURLs]) {
        //     NSLog(@"ERROR: Failed to show properties for file at %@", path);
        // }
        NSArray* fileURLs = @[fileURL];
        [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:fileURLs];
    }
}
