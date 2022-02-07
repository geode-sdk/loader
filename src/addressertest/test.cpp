#define GEODE_WRAPPER_CONCAT(x, y) x##y
#define GEODE_CONCAT(x, y) GEODE_WRAPPER_CONCAT(x, y)
#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <iostream>

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

		using tablemethodptr_t = virtual_meta_t*(MultipleInheritance::*)();

		static MultipleInheritance* instance();

		// template<typename T>
		// static virtual_meta_t* metaOf(T ptr) { 
		// 	static_assert(sizeof(tablemethodptr_t) == sizeof(intptr_t) * 2);
		// 	auto func = reinterpret_cast<tablemethodptr_t&>(ptr);
		// 	return (instance()->*func)(); 
		// }

		template <typename R, typename T, typename ...Ps>
		static virtual_meta_t* metaOf(R(T::*func)(Ps...)) { 
			using method_t = virtual_meta_t*(T::*)();
			// static_assert(sizeof(tablemethodptr_t) == sizeof(intptr_t) * 2);
			// auto func = reinterpret_cast<tablemethodptr_t&>(ptr);
			return (reinterpret_cast<T*>(instance())->*reinterpret_cast<method_t>(func))(); 
		}

		template<typename T>
		static intptr_t pointerOf(T func) {
			return reinterpret_cast<intptr_t&>(func);
		}

		/**
		 * Specialized functionss
		 */
		template <typename R, typename T, typename ...Ps>
		static intptr_t addressOfVirtual(R(T::*func)(Ps...), 
			typename std::enable_if_t<std::is_copy_constructible_v<T> && !std::is_abstract_v<T>>* = 0) {
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

			// metadata of the virtual function
			// LOL!
			auto meta = metaOf(func);
			std::cout << "thunk: " << meta->thunk << " index: " << meta->index << std::endl;

			// [[this + thunk] + offset] is the f we want
			auto address = *(intptr_t*)(*(intptr_t*)(pointerOf(ins) + meta->thunk) + meta->index);

			// And we delete the new instance because we are good girls
			// and we don't leak memories
			operator delete(ins);
			delete meta;

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
			return pointerOf(func);
		}

		template <typename R, typename ...Ps>
		static intptr_t addressOfNonVirtual(R(*func)(Ps...)) {
			return pointerOf(func);
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
		auto meta = Addresser::metaOf(func);
		auto ret = (intptr_t)self + meta->thunk;
		delete meta;
		return (F)(ret);
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


#define GEODE_ADDRESSER_THUNK0_DEFINE(hex) (intptr_t)&f<hex * sizeof(intptr_t), 0>
#define GEODE_ADDRESSER_TABLE_DEFINE(hex) (intptr_t)&ThunkTable<hex * sizeof(intptr_t)>::table

#define GEODE_ADDRESSER_THUNK0_SET() GEODE_ADDRESSER_NEST2(GEODE_ADDRESSER_THUNK0_DEFINE, 0x)
#define GEODE_ADDRESSER_TABLE_SET() GEODE_ADDRESSER_NEST2(GEODE_ADDRESSER_TABLE_DEFINE, 0x)

using namespace geode::addresser;

namespace {
	template<ptrdiff_t index, ptrdiff_t thunk>
	virtual_meta_t* f() {
		return new virtual_meta_t(index, thunk);
	}

	using thunk0_table_t = intptr_t[0x100];
	using thunk_table_t = intptr_t[0x10];
	using table_table_t = intptr_t[0x100];

	template <ptrdiff_t I>
	struct ThunkTable {
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
	struct ThunkTable<0> {
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
    // Class1() = delete;
    // ~Class1() {
    //     std::exit(EXIT_FAILURE);
    // }
    virtual void func1() {}
    virtual int func2() {return 5;}
    virtual bool func3() {return true;}
};

class Class2 {
public:
    // Class2() = delete;
    // ~Class2() {
    //     std::exit(EXIT_FAILURE);
    // }
    virtual void* func4() {return nullptr;}
    virtual long func5() {return -1;}
    virtual void func6() {
    	std::cout << "jdksfh" << std::endl;
    }
};

class Class3 : public Class1, public Class2 {
public:
    // Class3() = delete;
    // ~Class3() {
    //     std::exit(EXIT_FAILURE);
    // }
    virtual bool func3() {return false;}
    virtual void* func4() {return (void*)5;}
    int func7() {
        std::cout << "lol" << std::endl;
        return 2;
    }
    static void func8() {}
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
    // auto a = (void(Class3::*)()) &Class2::func6 ;
    // std::cout << (void*)addresser::getVirtual(a) << std::endl;
    // Class3 e;
    // (e .*a)();
    return 0;
}
