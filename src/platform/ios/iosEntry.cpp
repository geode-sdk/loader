#include "../entry.hpp"
#include <utils/platform.hpp>

#ifdef GEODE_IS_IOS

#include <mach-o/dyld.h>
#include <unistd.h>

__attribute__((constructor)) void _entry() {
    char gddir[PATH_MAX];
    uint32_t out = PATH_MAX;
    _NSGetExecutablePath(gddir, &out);

    _geode_ios_base();

    ghc::filesystem::path gdpath = gddir;

    ghc::filesystem::current_path(gdpath.parent_path().parent_path());



    /*for (const auto& dirEntry : ghc::filesystem::recursive_directory_iterator(ghc::filesystem::path("/var/mobile/.geodequeue"))) {
        ghc::filesystem::rename(
            dirEntry.path(),
            geode::utils::dirs::geode_root() / "geode" / "mods" / dirEntry.path().filename()
        );
    }*/

    geodeEntry(nullptr);
}
#endif
