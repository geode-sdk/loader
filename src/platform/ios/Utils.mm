#include <utils/platform.hpp>

#ifdef GEODE_IS_IOS

USE_GEODE_NAMESPACE();

#include <iostream>
#include <sstream>
#include <UIKit/UIKit.h>

bool utils::clipboard::write(std::string const& data) {
	[UIPasteboard generalPasteboard].string = [NSString stringWithUTF8String: data.c_str()];
    return true;
}

std::string utils::clipboard::read() {
	return std::string([[UIPasteboard generalPasteboard].string UTF8String]);
}

ghc::filesystem::path utils::dirs::geode_root() {
	return ghc::filesystem::path([[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] lastObject].path.UTF8String);
}

void utils::web::openLinkInBrowser(std::string const& url) {
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:[NSString stringWithUTF8String: url.c_str()]]];
}

extern "C" void geode_nslog(uintptr_t c) {
	NSLog(@"cock %lx", c);
}

#endif
