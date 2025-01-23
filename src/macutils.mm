#include <Cocoa/Cocoa.h>

extern "C" void showFileProperties(const char* filePath)
{
    @autoreleasepool {
        NSString *path = [NSString stringWithUTF8String:filePath];
        NSURL *fileURL = [NSURL fileURLWithPath:path];
        [[NSWorkspace sharedWorkspace] showPropertiesForFile:fileURL];
    }
}
