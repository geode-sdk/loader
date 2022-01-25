#include "../entry.hpp"

#ifdef GEODE_IS_MACOS

#include <mach-o/dyld.h>
#include <unistd.h>

__attribute__((constructor)) void _entry() {
    char gddir[PATH_MAX];
    uint32_t out = PATH_MAX;
    _NSGetExecutablePath(gddir, &out);

    ghc::filesystem::path gdpath = gddir;

    ghc::filesystem::current_path(gdpath.parent_path().parent_path());

    geodeEntry(nullptr);
}
#endif
