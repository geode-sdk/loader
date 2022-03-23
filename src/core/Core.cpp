#include "Core.hpp"
#include <map>

#if defined(GEODE_IS_WINDOWS)
    // #include "Windows.hpp"
#elif defined(GEODE_IS_MACOS)
    #include "MacOS.hpp"
#elif defined(GEODE_IS_IOS)
    // #include "iOS.hpp"
#endif

namespace geode::core::impl {
	namespace {
		auto originalBytes() {
			static std::map<void*, std::vector<std::byte>> ret;
			return ret;
		}

		auto unhandledTrampolines() {
			static std::map<void*, void*> ret;
			return ret;
		}
	}

	void* generateRawTrampoline(void* address) {
		static constexpr MAX_TRAMPOLINE_SIZE = 0x40;
		auto trampoline = TargetPlatform::allocateVM(MAX_TRAMPOLINE_SIZE);
		unhandledTrampolines()[address] = trampoline;
		return trampoline;
	}

	void addJump(void* at, void* to) {
		if (unhandledTrampolines().find(at) != unhandledTrampolines().end()) { // trampoline not finished
			// we d-dont have original bytes set up
			const size_t trapSize = TargetPlatform::getTrapSize();
			originalBytes()[at].reserve(trapSize);

			// set up the illegal trap
			std::memcpy((void*)originalBytes().data(), at, trapSize);
			std::memcpy(at, (void*)TargetPlatform::getTrap().data(), trapSize);
		}
		else {
			std::memcpy(at, (void*)TargetPlatform::getJump(at, to), TargetPlatform::getJumpSize(at, to));
		}
	}

	void handleContext(void* context, const void* original, const void* current) {
		if (original == current) { 
			//remove the trap
			auto origBytes = originalBytes()[original];
			TargetPlatform::writeMemory(original, (void*)origBytes.data(), origBytes.size()); 
			originalBytes().erase(original);

			// enable single step
			TargetPlatform::enableSingleStep(context);
			return;
		}
		else {
			auto trampoline = unhandledTrampolines[original];

			const size_t jumpSize = TargetPlatform::getJumpSize(trampoline, original);
			if (current - original >= jumpSize) {
				// if the size is found, copy the contents to vm
				std::memcpy(trampoline, original, current - original);
				std::memcpy((void*)((size_t)trampoline + (current - original)), TargetPlatform::getJump(trampoline, original), current - original);

				unhandledTrampolines().erase(original);

				addJump(original, trampoline);

				TargetPlatform::disableSingleStep(context);
			}
		}
	}
}
