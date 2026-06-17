#pragma once

#include <console/simObject.h>
#include <lights/fluxLight.h>


namespace ElfFlux {

    class Light : public SimObject {
        typedef SimObject Parent;
    public:
        FluxLight mLight;

        static void initPersistFields();

        bool onAdd() override;
        void onRemove() override;


        DECLARE_CONOBJECT(Light);
    };
}
