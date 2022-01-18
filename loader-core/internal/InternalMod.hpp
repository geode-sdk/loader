#pragma once

#include <interface/Mod.hpp>

USE_LILAC_NAMESPACE();

class Lilac;

class InternalMod : public Mod {
    protected:
        friend class Lilac;

        InternalMod();

    public:
        static InternalMod* get();
};
