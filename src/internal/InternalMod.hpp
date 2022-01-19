#pragma once

class InternalMod;

#include <Mod.hpp>

USE_LILAC_NAMESPACE();

class Lilac;

class InternalMod : public Mod {
    protected:
        friend class Lilac;

        InternalMod();
        virtual ~InternalMod();

    public:
        static InternalMod* get();
};
