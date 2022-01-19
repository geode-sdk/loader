#pragma once

class InternalMod;

#include <Mod.hpp>

USE_GEODE_NAMESPACE();

class Geode;

class InternalMod : public Mod {
    protected:
        friend class Geode;

        InternalMod();
        virtual ~InternalMod();

    public:
        static InternalMod* get();
};
