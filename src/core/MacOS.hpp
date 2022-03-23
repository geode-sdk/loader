#pragma once

#include <vector>
#include "Platform.hpp"

namespace geode::core::impl {
    class MacOSX : public Platform<MacOSX> {
    public:
    	static inline auto trap = { std::byte(0x0f), std::byte(0x0b) };

        static bool writeMemory(const void* to, const void* from, const size_t size);
        // static bool readMemory(const void* addr, const void* to, const size_t size);
        static std::vector<std::byte> jump(const void* from, const void* to);
        static bool initialize();
        static bool enableSingleStep(void* context);
        static bool disableSingleStep(void* context);
        static void* allocateVM(size_t size);
    };

    using TargetPlatform = Platform<MacOSX>;
}
