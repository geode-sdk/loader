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

#define GEODE_ADDRESSER_NEST1(macro, begin)           \
macro(GEODE_CONCAT(begin, 0)),                        \
macro(GEODE_CONCAT(begin, 1)),                        \
macro(GEODE_CONCAT(begin, 2)),                        \
macro(GEODE_CONCAT(begin, 3)),                        \
macro(GEODE_CONCAT(begin, 4)),                        \
macro(GEODE_CONCAT(begin, 5)),                        \
macro(GEODE_CONCAT(begin, 6)),                        \
macro(GEODE_CONCAT(begin, 7)),                        \
macro(GEODE_CONCAT(begin, 8)),                        \
macro(GEODE_CONCAT(begin, 9)),                        \
macro(GEODE_CONCAT(begin, a)),                        \
macro(GEODE_CONCAT(begin, b)),                        \
macro(GEODE_CONCAT(begin, c)),                        \
macro(GEODE_CONCAT(begin, d)),                        \
macro(GEODE_CONCAT(begin, e)),                        \
macro(GEODE_CONCAT(begin, f))  

#define GEODE_ADDRESSER_NEST2(macro, begin)           \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 0)), \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 1)), \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 2)), \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 3)), \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 4)), \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 5)), \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 6)), \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 7)), \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 8)), \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, 9)), \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, a)), \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, b)), \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, c)), \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, d)), \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, e)), \
GEODE_ADDRESSER_NEST1(macro, GEODE_CONCAT(begin, f))  


#define GEODE_ADDRESSER_THUNK0_DEFINE(hex) (intptr_t)&f<hex * sizeof(intptr_t), 0>
#define GEODE_ADDRESSER_TABLE_DEFINE(hex) (intptr_t)&ThunkTable<hex * sizeof(intptr_t)>::table

#define GEODE_ADDRESSER_THUNK0_SET() GEODE_ADDRESSER_NEST2(GEODE_ADDRESSER_THUNK0_DEFINE, 0x)
#define GEODE_ADDRESSER_TABLE_SET() GEODE_ADDRESSER_NEST2(GEODE_ADDRESSER_TABLE_DEFINE, 0x)

using namespace geode::addresser;

namespace {
	template<ptrdiff_t index, ptrdiff_t thunk>
	GEODE_HIDDEN virtual_meta_t f() {
		return {index, thunk};
	}

	using thunk0_table_t = intptr_t[0x100];
	using thunk_table_t = intptr_t[0x10];
	using table_table_t = intptr_t[0x100];

	template <ptrdiff_t I>
	struct GEODE_HIDDEN ThunkTable {
		static inline thunk_table_t table = {
			(intptr_t)&f<0 * sizeof(intptr_t), I>,
			(intptr_t)&f<1 * sizeof(intptr_t), I>,
			(intptr_t)&f<2 * sizeof(intptr_t), I>,
			(intptr_t)&f<3 * sizeof(intptr_t), I>,
			(intptr_t)&f<4 * sizeof(intptr_t), I>,
			(intptr_t)&f<5 * sizeof(intptr_t), I>,
			(intptr_t)&f<6 * sizeof(intptr_t), I>,
			(intptr_t)&f<7 * sizeof(intptr_t), I>,
			(intptr_t)&f<8 * sizeof(intptr_t), I>,
			(intptr_t)&f<9 * sizeof(intptr_t), I>,
			(intptr_t)&f<10 * sizeof(intptr_t), I>,
			(intptr_t)&f<11 * sizeof(intptr_t), I>,
			(intptr_t)&f<12 * sizeof(intptr_t), I>,
			(intptr_t)&f<13 * sizeof(intptr_t), I>,
			(intptr_t)&f<14 * sizeof(intptr_t), I>,
			(intptr_t)&f<15 * sizeof(intptr_t), I>
		};
	};

	template <>
	struct GEODE_HIDDEN ThunkTable<0> {
		static inline thunk0_table_t table = {
			GEODE_ADDRESSER_THUNK0_SET()
		};
	};

	class GEODE_HIDDEN TableTable {
		friend class geode::addresser::Addresser;

		static inline table_table_t table = {
			GEODE_ADDRESSER_TABLE_SET()
		};
	};
}

Addresser* Addresser::instance() {
	return reinterpret_cast<Addresser*>(&TableTable::table);
}
