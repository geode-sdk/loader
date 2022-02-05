#pragma once

#include <Geode.hpp>

USE_GEODE_NAMESPACE();

#ifdef EXPORTING_MOD
    #define MOD_DLL GEODE_WINDOWS(__declspec(dllexport))
#else
    #define MOD_DLL GEODE_WINDOWS(__declspec(dllimport))
#endif

void MOD_DLL logMessage(std::string_view const& msg);
