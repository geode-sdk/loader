#pragma once

#include <Geode.hpp>
#include <modify/Modify.hpp>

USE_GEODE_NAMESPACE();

using namespace geode::modifier;

template <int I>
class MyGarageLayer : public GJGarageLayer {
public:
	bool init();
};
