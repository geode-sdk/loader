#include "Windows.hpp"
#include "Core.hpp"

#ifdef GEODE_IS_WINDOWS

#include <Windows.h>

using namespace geode::core::hook;
using namespace geode::core::impl;

namespace {
    LONG WINAPI signalHandler(EXCEPTION_POINTERS* info) {

	#if defined(_WIN64)
        auto current = reinterpret_cast<void*>(info->ContextRecord->Rip);
    #elif defined(_WIN32)
        auto current = reinterpret_cast<void*>(info->ContextRecord->Eip);
    #endif

        handleContext((void*)info, current);

        return EXCEPTION_CONTINUE_EXECUTION;
    }
}

void* Windows::allocateVM(size_t size) {
	// Please somebody implement this
	return nullptr;
}

std::vector<std::byte> Windows::jump(void* from, void* to) {
	constexpr size_t size = sizeof(int) + 1;
	std::vector<std::byte> ret(size);
	ret[0] = std::byte(0xe9);

	int offset = (int)((size_t)to - (size_t)from - size);
	// im too lazy
	((int*)((size_t)ret.data() + 1))[0] = offset;

	return ret;
}

bool Windows::enableSingleStep(void* vcontext) {
	auto info = reinterpret_cast<EXCEPTION_POINTERS*>(vcontext);
#if defined(_WIN64)
    info->ContextRecord->RFlags |= ((QWORD)0x100);
#elif defined(_WIN32)
    info->ContextRecord->EFlags |= ((DWORD)0x100);
#endif
	return true;
}

bool Windows::disableSingleStep(void* vcontext) {
	auto info = reinterpret_cast<EXCEPTION_POINTERS*>(vcontext);
#if defined(_WIN64)
    info->ContextRecord->RFlags &= ~((QWORD)0x100);
#elif defined(_WIN32)
    info->ContextRecord->EFlags &= ~((DWORD)0x100);
#endif
	return true;
}

bool Windows::writeMemory(void* to, void* from, size_t size) {
    DWORD old;
    VirtualProtect(to, size, PAGE_EXECUTE_READWRITE, &old);
    auto res = WriteProcessMemory(GetCurrentProcess(), to, from, size, nullptr);
    VirtualProtect(to, size, old, &old);
    return res;
}

bool Windows::initialize() {
    return AddVectoredExceptionHandler(true, signalHandler);
}

#endif
