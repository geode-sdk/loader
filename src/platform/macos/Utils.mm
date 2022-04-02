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
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));
	// utils::clipboard::write(cwd);
	return ghc::filesystem::path(cwd);
}


void utils::web::openLinkInBrowser(std::string const& url) {
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String: url.c_str()]]];
}

#endif
