#include <utils/platform.hpp>
#include <Geode.hpp>

#ifdef GEODE_IS_ANDROID

USE_GEODE_NAMESPACE();

#include <iostream>
#include <sstream>

bool utils::clipboard::write(std::string const& data) {
    return false;
}

std::string utils::clipboard::read() {
    return "";
}

ghc::filesystem::path utils::dirs::geodeRoot() {
    return ghc::filesystem::path("/sdcard/geode/");
}

bool utils::dirs::openFolder(ghc::filesystem::path const& path) {
	return false;
}

void geode::utils::web::openLinkInBrowser(std::string const& url) {
    return;
}

#endif
