//-----------------------------------------------------------------------------
// Copyright (c) 1993 T.Huehn (XXTH)
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#include "ymfm.h"
#include "ymfm_opl.h"
//------------------------------------------------------------------------------
// class OplInterface
//------------------------------------------------------------------------------
// The Interface: Handles raw hardware-level callbacks
class OplInterface : public ymfm::ymfm_interface {
public:
    virtual uint8_t ymfm_external_read(ymfm::access_class type, uint32_t address) override { return 0; }
    virtual void ymfm_external_write(ymfm::access_class type, uint32_t address, uint8_t data) override { }
};
