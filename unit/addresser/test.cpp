#define GEODE_ADDRESSER_TEST
#include <utils/addresser.hpp>
#include <addresser.cpp>
#include <iostream>

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

void exit_assert_impl(bool val, char const* func, int line) {
	if (!val) {
		std::cout << "fail: at " << func << " line " << line << "\n";
		std::exit(EXIT_FAILURE);
	}
}

#define exit_assert(val) exit_assert_impl(val, __FUNCTION__, __LINE__)

int main() {
    using namespace geode;
    using func_t = int(*)();
    auto fun1 = (func_t)addresser::getVirtual(&Class1::func1);
    auto fun2 = (func_t)addresser::getVirtual(&Class1::func2);
    auto fun3 = (func_t)addresser::getVirtual(&Class1::func3);
    auto fun4 = (func_t)addresser::getVirtual(&Class2::func4);
    auto fun5 = (func_t)addresser::getVirtual(&Class2::func5);
    auto fun6 = (func_t)addresser::getVirtual(&Class2::func6);
    auto fun7 = (func_t)addresser::getVirtual(&Class3::func3);
    auto fun8 = (func_t)addresser::getVirtual(&Class3::func4);
    auto fun9 = (func_t)addresser::getNonVirtual(&Class3::func7);
    auto fun10 = (func_t)addresser::getNonVirtual(&Class3::func8);
    exit_assert(fun1() == 1);
    exit_assert(fun2() == 2);
    exit_assert(fun3() == 3);
    exit_assert(fun4() == 4);
    exit_assert(fun5() == 5);
    exit_assert(fun6() == 6);
    exit_assert(fun7() == 7);
    exit_assert(fun8() == 8);
    exit_assert(fun9() == 9);
    exit_assert(fun10() == 10);
    return 0;
}
