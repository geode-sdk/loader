#define GEODE_WRAPPER_CONCAT(x, y) x##y
#define GEODE_CONCAT(x, y) GEODE_WRAPPER_CONCAT(x, y)
#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <iostream>

namespace geode::cast {
	template<typename T, typename F>
	static constexpr T reference_cast(F v) {
		return reinterpret_cast<T&>(v);
	}
}

namespace geode::addresser {

	struct virtual_meta_t {
		ptrdiff_t index;
		ptrdiff_t thunk;
		virtual_meta_t(ptrdiff_t index, ptrdiff_t thunk) : index(index), thunk(thunk) {}
	};

	template<typename T>
	inline intptr_t getVirtual(T func);

	template<typename T>
	inline intptr_t getNonVirtual(T func);

	template<typename T, typename F>
	inline F thunkAdjust(T func, F self);

	class Addresser final {
		template <char C>
		struct SingleInheritance {
			virtual ~SingleInheritance() {}
		};
		struct MultipleInheritance : 
			SingleInheritance<'L'>, 
			SingleInheritance<'F'> {
			virtual ~MultipleInheritance() {}
		};

		static MultipleInheritance* instance();

		template <typename R, typename T, typename ...Ps>
		static ptrdiff_t indexOf(R(T::*func)(Ps...)) { 
			using method_t = ptrdiff_t(T::*)();
			return (reinterpret_cast<T*>(instance())->*reinterpret_cast<method_t>(func))(); 
		}

		template <typename R, typename T, typename ...Ps>
		static ptrdiff_t indexOf(R(T::*func)(Ps...) const) { 
			return indexOf(reinterpret_cast<R(T::*)(Ps...)>(func));
		}

		template<typename T>
		static ptrdiff_t thunkOf(T ptr) {
			// msvc
			if (sizeof(T) == sizeof(ptrdiff_t)) return 0;

			// all
			auto thunk = *(reinterpret_cast<ptrdiff_t*>(&ptr)+1);

			// arm
			if (thunk & 1) thunk >>= 1;
			return thunk;
		}

		/**
		 * Specialized functionss
		 */
		template <typename R, typename T, typename ...Ps>
		static intptr_t addressOfVirtual(R(T::*func)(Ps...), 
			typename std::enable_if_t<!std::is_abstract_v<T>>* = 0) {
			using geode::cast::reference_cast;

			// Create a random memory block with the size of T
			// Assign a pointer to that block and cast it to type T*
			uint8_t dum[sizeof(T)] {};
			auto ptr = reinterpret_cast<T*>(dum);
			// Now you have a object of T that actually isn't an object of T and is just a random memory
			// But C++ doesn't know that of course
			// So now you can copy an object that wasn't there in the first place
			// ((oh also get the offsets of the virtual tables))
			auto ins = new T(*ptr);
			// this is how the first human was made


			auto index = indexOf(func);
			auto thunk = thunkOf(func);

			// [[this + thunk] + offset] is the f we want
			auto address = *(intptr_t*)(*(intptr_t*)(reference_cast<intptr_t>(ins) + thunk) + index);

			// #ifdef GEODE_IS_WINDOWS
			// 	// check if first instruction is a jmp, i.e. if the func is a thunk
			// 	if (*reinterpret_cast<uint16_t*>(address) == 0x25ff) {
			// 		// read where the jmp points to and jump there
			// 		address = *reinterpret_cast<uintptr_t*>(address + 2);
			// 		// that then contains the actual address of the func
			// 		address = *reinterpret_cast<uintptr_t*>(address);
			// 	}
			// #endif

			// And we delete the new instance because we are good girls
			// and we don't leak memories
			operator delete(ins);

			return address;
		}

		template <typename R, typename T, typename ...Ps>
		static intptr_t addressOfVirtual(R(T::*func)(Ps...) const,
			typename std::enable_if_t<std::is_copy_constructible_v<T> && !std::is_abstract_v<T>>* = 0) {
			return addressOfVirtual(reinterpret_cast<R(T::*)(Ps...)>(func));
		}

		template <typename R, typename T, typename ...Ps>
		static intptr_t addressOfVirtual(R(T::*func)(Ps...), typename std::enable_if_t<std::is_abstract_v<T>>* = 0) {
			return 0;
		}

		template <typename R, typename T, typename ...Ps>
		static intptr_t addressOfVirtual(R(T::*func)(Ps...) const, typename std::enable_if_t<std::is_abstract_v<T>>* = 0) {
			return 0;
		}

		template <typename R, typename T, typename ...Ps>
		static intptr_t addressOfNonVirtual(R(T::*func)(Ps...) const) {
			return addressOfNonVirtual(reinterpret_cast<R(T::*)(Ps...)>(func));
		}

		template <typename R, typename T, typename ...Ps>
		static intptr_t addressOfNonVirtual(R(T::*func)(Ps...)) {
			return geode::cast::reference_cast<intptr_t>(func);
		}

		template <typename R, typename ...Ps>
		static intptr_t addressOfNonVirtual(R(*func)(Ps...)) {
			return geode::cast::reference_cast<intptr_t>(func);
		}

		template<typename T>
		friend intptr_t getVirtual(T func);

		template<typename T>
		friend intptr_t getNonVirtual(T func);

		template<typename T, typename F>
		friend F thunkAdjust(T func, F self);
	};

	template<typename T>
	inline intptr_t getVirtual(T func) {
		auto addr = Addresser::addressOfVirtual(func);
		return addr;
	}

	template<typename T>
	inline intptr_t getNonVirtual(T func) {
		auto addr = Addresser::addressOfNonVirtual(func);
		return addr;
	}

	template<typename T, typename F>
	inline F thunkAdjust(T func, F self) {
		return (F)((intptr_t)self + Addresser::thunkOf(func));
	}
}

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


#define GEODE_ADDRESSER_THUNK0_DEFINE(hex) (intptr_t)&f<hex * sizeof(intptr_t)>
#define GEODE_ADDRESSER_TABLE_DEFINE(hex) (intptr_t)&ThunkTable::table

#define GEODE_ADDRESSER_THUNK0_SET() GEODE_ADDRESSER_NEST2(GEODE_ADDRESSER_THUNK0_DEFINE, 0x)
#define GEODE_ADDRESSER_TABLE_SET() GEODE_ADDRESSER_NEST2(GEODE_ADDRESSER_TABLE_DEFINE, 0x)

using namespace geode::addresser;

namespace {
	template<ptrdiff_t index>
	ptrdiff_t f() {
		return index;
	}

	using thunk0_table_t = intptr_t[0x100];
	using table_table_t = intptr_t[0x100];

	struct ThunkTable {
		static inline thunk0_table_t table = {
			GEODE_ADDRESSER_THUNK0_SET()
		};
	};

	class TableTable {
		friend class geode::addresser::Addresser;

		static inline table_table_t table = {
			GEODE_ADDRESSER_TABLE_SET()
		};
	};
}

Addresser::MultipleInheritance* Addresser::instance() {
	return reinterpret_cast<Addresser::MultipleInheritance*>(&TableTable::table);
}

class Class1 {
public:
    Class1() = delete;
    ~Class1() {
        std::exit(EXIT_FAILURE);
    }
    virtual int func1() {return 1;}
    virtual int func2() {return 2;}
    virtual int func3() {return 3;}
};

class Class2 {
public:
    Class2() = delete;
    ~Class2() {
        std::exit(EXIT_FAILURE);
    }
    virtual int func4() {return 4;}
    virtual int func5() {return 5;}
    virtual int func6() {return 6;}
};

class Class3 : public Class1, public Class2 {
public:
    Class3() = delete;
    ~Class3() {
        std::exit(EXIT_FAILURE);
    }
    virtual int func3() {return 7;}
    virtual int func4() {return 8;}
    int func7()         {return 9;}
    static int func8()  {return 10;}
};

int main() {
    using namespace geode;
    std::cout << "Class1:\n" << (void*)addresser::getVirtual(&Class1::func1) << std::endl;
    std::cout << (void*)addresser::getVirtual(&Class1::func2) << std::endl;
    std::cout << (void*)addresser::getVirtual(&Class1::func3) << std::endl << std::endl;

    std::cout << "Class2:\n" << (void*)addresser::getVirtual(&Class2::func4) << std::endl;
    std::cout << (void*)addresser::getVirtual(&Class2::func5) << std::endl;
    std::cout << (void*)addresser::getVirtual(&Class2::func6) << std::endl << std::endl;

    std::cout << "Class3:\n" << (void*)addresser::getVirtual(&Class3::func1) << std::endl;
    std::cout << (void*)addresser::getVirtual(&Class3::func2) << std::endl;
    std::cout << (void*)addresser::getVirtual(&Class3::func3) << std::endl;
    std::cout << (void*)addresser::getVirtual(&Class3::func4) << std::endl;
    std::cout << (void*)addresser::getVirtual(&Class3::func5) << std::endl;
    std::cout << (void*)addresser::getVirtual(&Class3::func6) << std::endl;
    std::cout << (void*)addresser::getNonVirtual(&Class3::func7) << std::endl;
    std::cout << (void*)addresser::getNonVirtual(&Class3::func8) << std::endl << std::endl;
    return 0;
}
