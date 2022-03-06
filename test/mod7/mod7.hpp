#include <Geode.hpp>
#include <API.hpp>

#ifdef MOD7_EXPORTING
	#define MOD7_LINK GEODE_EXPORT_FUNC
	// do we want to prepend the mod name for avioding conficts?
#else
	#define MOD7_LINK GEODE_IMPORT_FUNC
#endif

class Mod7Log {
public:
	void logMessage(std::string msg) 
		MOD7_LINK((void(Mod7Log::*)(std::string))&Mod7Log::logMessage, this, msg);

	static void logMessage(int msg, float msg2) 
		MOD7_LINK((void(*)(int, float))&Mod7Log::logMessage, msg, msg2);
};
