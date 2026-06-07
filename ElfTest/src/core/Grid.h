#pragma once
#include "core/fluxBaseObject.h"
#include "render/fluxRender2D.h"
#include "sim/simBase.h"
#include "console/consoleTypes.h"
#include "grid/basicgrid.h"

namespace ElfFlux {

class Grid : public SimSet, public FluxBaseObject {
     typedef SimObject Parent;
public:
    BasicGrid mGrid;


    // virtual void Update(const double& dt) override;
    // virtual void Draw() override;


    // static void initPersistFields();

    bool onAdd() override;
    void onRemove() override;

    SimObject * createPath(Point2F start, Point2F end, const bool smoothPath );


    DECLARE_CONOBJECT(Grid);
};

} //namespace
