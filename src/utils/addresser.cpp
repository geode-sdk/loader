/**
 * Adapted from https://gist.github.com/altalk23/29b97969e9f0624f783b673f6c1cd279
 */

#include <utils/addresser.hpp>
#include <cstdlib>
#include <stddef.h>
#include <Macros.hpp>
#include <loader/Interface.hpp>
#include <loader/Mod.hpp>
#include <loader/Log.hpp>
#include <utils/general.hpp>

#if INT64_MAX == INTPTR_MAX
	#define GEODE_ADDRESSER_NEST1(macro, begin)       \
	macro(GEODE_CONCAT(begin, 0))                     \
	macro(GEODE_CONCAT(begin, 8))         
#else
	#define GEODE_ADDRESSER_NEST1(macro, begin)       \
	macro(GEODE_CONCAT(begin, 0))                     \
	macro(GEODE_CONCAT(begin, 4))                     \
	macro(GEODE_CONCAT(begin, 8))                     \
	macro(GEODE_CONCAT(begin, c))         
#endif

#define GEODE_ADDRESSER_NEST2(macro, begin)           \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 0))  \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 1))  \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 2))  \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 3))  \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 4))  \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 5))  \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 6))  \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 7))  \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 8))  \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 9))  \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, a))  \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, b))  \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, c))  \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, d))  \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, e))  \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, f))  

#define GEODE_ADDRESSER_NEST3(macro, begin)           \
GEODE_ADDRESSER_NEST2(macro, GEODE_CONCAT(begin, 0))  \
GEODE_ADDRESSER_NEST2(macro, GEODE_CONCAT(begin, 1))  \
GEODE_ADDRESSER_NEST2(macro, GEODE_CONCAT(begin, 2))  \
GEODE_ADDRESSER_NEST2(macro, GEODE_CONCAT(begin, 3))  \
GEODE_ADDRESSER_NEST2(macro, GEODE_CONCAT(begin, 4))  \
GEODE_ADDRESSER_NEST2(macro, GEODE_CONCAT(begin, 5))  \
GEODE_ADDRESSER_NEST2(macro, GEODE_CONCAT(begin, 6))  \
GEODE_ADDRESSER_NEST2(macro, GEODE_CONCAT(begin, 7))  \
GEODE_ADDRESSER_NEST2(macro, GEODE_CONCAT(begin, 8))  \
GEODE_ADDRESSER_NEST2(macro, GEODE_CONCAT(begin, 9))  \
GEODE_ADDRESSER_NEST2(macro, GEODE_CONCAT(begin, a))  \
GEODE_ADDRESSER_NEST2(macro, GEODE_CONCAT(begin, b))  \
GEODE_ADDRESSER_NEST2(macro, GEODE_CONCAT(begin, c))  \
GEODE_ADDRESSER_NEST2(macro, GEODE_CONCAT(begin, d))  \
GEODE_ADDRESSER_NEST2(macro, GEODE_CONCAT(begin, e))  \
GEODE_ADDRESSER_NEST2(macro, GEODE_CONCAT(begin, f))  

/**
 * static ptrdiff_t f000() {return 0x000;}
 * static ptrdiff_t f004() {return 0x004;}
 * static ptrdiff_t f008() {return 0x008;}
 * ...
 */
#define GEODE_ADDRESSER_METHOD_DEFINE(hex)           \
inline static ptrdiff_t GEODE_CONCAT(f, hex)() {return GEODE_CONCAT(0x, hex);}

/**
 * (intptr_t)Addresser::f000,
 * (intptr_t)Addresser::f004,
 * (intptr_t)Addresser::f008,
 * ...
 */
#define GEODE_ADDRESSER_TABLE_DEFINE(hex)            \
(intptr_t)GEODE_CONCAT(AddresserTable::f, hex),

#define GEODE_ADDRESSER_METHOD_SET() GEODE_ADDRESSER_NEST3(GEODE_ADDRESSER_METHOD_DEFINE, )
#define GEODE_ADDRESSER_TABLE_SET() GEODE_ADDRESSER_NEST3(GEODE_ADDRESSER_TABLE_DEFINE, )

using namespace geode::addresser;

namespace {
	class GEODE_HIDDEN AddresserTable {
		friend class geode::addresser::Addresser;

		static constexpr ptrdiff_t table_size = 0x1000 / sizeof(intptr_t);

		using table_t = intptr_t[table_size + 0x1]; 
		using tableptr_t = table_t*; 

		GEODE_ADDRESSER_METHOD_SET()
		inline static ptrdiff_t f() {return -1;} //because c++ cries when there is a trailing comma 

		inline static tableptr_t tableptr() {
			static table_t ret = {
				GEODE_ADDRESSER_TABLE_SET()
				(intptr_t)AddresserTable::f
			};
			return &ret;
		}

		
	};
}

geode::addresser::Addresser* geode::addresser::Addresser::instance() {
	static auto ret = AddresserTable::tableptr();
	return reinterpret_cast<geode::addresser::Addresser*>(&ret);
}
