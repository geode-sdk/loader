#include "../entry.hpp"

#ifdef GEODE_IS_MACOS

__attribute__((constructor)) void _entry() {
	geodeEntry(nullptr);
}
#endif
