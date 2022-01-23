#include "../entry.hpp"

#ifdef GEODE_IS_MACOS

__attribute__((constructor)) void _inject() {
	printf("we load\n"); //sex
	geodeEntry(NULL);
}
#endif