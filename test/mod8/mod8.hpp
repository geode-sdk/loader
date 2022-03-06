#include "../mod7/mod7.hpp"

#ifdef MOD8_EXPORTING
	#define MOD8_LINK GEODE_EXPORT_FUNC
	// do we want to prepend the mod name for avioding conficts?
#else
	#define MOD8_LINK GEODE_IMPORT_FUNC
#endif

class Mod8Log {
public:
	void logMessage(std::string msg) 
		MOD8_LINK(&Mod8Log::logMessage, this, msg);
};
