#pragma once

#include <Geode>

USE_GEODE_NAMESPACE();

class ModSettingsListView : public CCScrollLayerExt {
    protected:
        Mod* m_mod;

        bool init(Mod* mod, float width, float height);

        ModSettingsListView(CCRect const& rect);

        void scrollWheel(float y, float x) override;

    public:
        static ModSettingsListView* create(
            Mod* Mod, float width, float height
        );
};
