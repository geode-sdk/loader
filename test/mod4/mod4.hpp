#pragma once

#include <Geode.hpp>
#include <modify/Modify.hpp>

USE_GEODE_NAMESPACE();

using namespace geode::modifier;

#ifdef GEODE_IS_WINDOWS
	#ifdef EXPORTING_MOD
		#define GEODE_T4_DLL __declspec(dllexport)
	#else
		#define GEODE_T4_DLL __declspec(dllimport)
	#endif
#else
	#define GEODE_T4_DLL
#endif

#pragma warning(disable: 4275)

template <int I>
class GEODE_T4_DLL MyGarageLayer : public GJGarageLayer {
public:
	bool init();
};

