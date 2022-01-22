#include <helpers/platform.hpp>

#ifdef GEODE_IS_MACOS

USE_GEODE_NAMESPACE();

#include <iostream>
#include <sstream>
#include <Cocoa/Cocoa.h>

bool utils::clipboard::write(std::string const& data) {
	[[NSPasteboard generalPasteboard] clearContents];
	[[NSPasteboard generalPasteboard] setString:[NSString stringWithUTF8String: data.c_str()] forType:NSPasteboardTypeString];

    return true;
}

std::string utils::clipboard::read() {
	return [[NSPasteboard generalPasteboard] stringForType: NSPasteboardTypeString];
}

#endif
