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
		static constexpr size_t MAX_TRAMPOLINE_SIZE = 0x40;
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
			std::memcpy((void*)originalBytes()[at].data(), at, trapSize);
			std::memcpy(at, (void*)TargetPlatform::getTrap().data(), trapSize);
		}
		else {
			std::memcpy(at, (void*)TargetPlatform::getJump(at, to).data(), TargetPlatform::getJumpSize(at, to));
		}
	}

	void handleContext(void* context, void* original, void* current) {
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
			auto trampoline = unhandledTrampolines()[original];

			const size_t jumpSize = TargetPlatform::getJumpSize(trampoline, original);
			const size_t difference = (size_t*)current - (size_t*)original;
			if (difference >= jumpSize) {
				// if the size is found, copy the contents to vm
				std::memcpy(trampoline, original, difference);
				std::memcpy((void*)((size_t)trampoline + difference), (void*)TargetPlatform::getJump(trampoline, original).data(), difference);

				unhandledTrampolines().erase(original);

				addJump(original, trampoline);

				TargetPlatform::disableSingleStep(context);
			}
		}
	}
}

bool geode::core::hook::initialize() {
	return geode::core::impl::TargetPlatform::initialize();
}
