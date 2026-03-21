//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#include <curl/curl.h>

namespace FluxNet {

    static bool gCurlInitialized = false;
    inline void initCurl() {
        if ( gCurlInitialized ) return;
        curl_global_init(CURL_GLOBAL_DEFAULT);
        gCurlInitialized = true;
    }

    inline void shutdownCurl( ) {
        if (!gCurlInitialized) return;
        curl_global_cleanup();
        gCurlInitialized = true;
    }
};
