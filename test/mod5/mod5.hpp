#pragma once

#include <Geode.hpp>
#include "../mod4/mod4.hpp"

USE_GEODE_NAMESPACE();


template<class Derived, int I>
struct Modify<Derived, MyGarageLayer<I>> : ModifyBase<Modify<Derived, MyGarageLayer<I>>> {
	using Base = MyGarageLayer<I>;
	using ModifyBase<Modify<Derived, Base>>::ModifyBase;
	static void apply() {
		using namespace geode::core::meta;
		if constexpr (compare::init<Derived, Base, bool()>::value) {
			Interface::get()->logInfo(
				"Adding hook at function MyGarageLayer<>::init", 
				Severity::Debug
			);
			Interface::get()->addHook(
				"MyGarageLayer<>::init", 
				(void*)addresser::getVirtual(&Base::init), 
				(void*)wrap::init<DefaultConv, Derived, Base, bool()>::value
			);
		}
	}
};
