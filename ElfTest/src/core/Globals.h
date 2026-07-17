#pragma once
#include "consoleFunctions/ConsoleTypes.h"
#include "math/mMathFn.h"


constexpr U32 HS2D_MAXLAYERS=1000;

inline F32 LAYER_F(U32 layer) {
    return getMax(layer, HS2D_MAXLAYERS) / F32(HS2D_MAXLAYERS);
}

namespace ElfFlux {
    static String gLastScriptFile = "";
    bool executeFile(const char* fileName, bool noCalls = false, bool journalScript = false) ;
    bool loadScript(String fileName);
    void init();
}

