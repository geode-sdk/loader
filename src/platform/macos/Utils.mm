#include <utils/platform.hpp>

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
	return std::string([[[NSPasteboard generalPasteboard] stringForType: NSPasteboardTypeString] UTF8String]);
}

ghc::filesystem::path utils::dirs::geode_root() {
	NSString* p = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES)[0];
	return ghc::filesystem::path(p.UTF8String) / [[NSBundle mainBundle] bundleIdentifier].UTF8String;
}

#endif
