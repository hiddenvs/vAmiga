// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Paula::Paula()
{
    setDescription("Paula");
    
    // Register sub components
    registerSubcomponents(vector<HardwareComponent *> {
        
        &audioUnit
    });
    
    // Register snapshot items
    registerSnapshotItems(vector<SnapshotItem> {
        { &clock,  sizeof(clock),  0 },
        { &intreq, sizeof(intreq), 0 },
        { &intena, sizeof(intena), 0 },
    });
    
}

void
Paula::_powerOn()
{
    
}

void
Paula::_setWarp(bool value)
{
    /* Warping has the unavoidable drawback that audio playback gets out of
     * sync. To cope with this issue, we ramp down the volume when entering
     * warp mode and fade in smoothly when warping ends.
     */
    if (value) {
        
        audioUnit.rampDown();
        
    } else {
        
        audioUnit.rampUp();
        audioUnit.alignWritePtr();
    }
}

PaulaInfo
Paula::getInfo()
{
    PaulaInfo info;
    
    info.intreq = intreq;
    info.intena = intena;
    
    return info;
}

uint16_t
Paula::peekINTREQ()
{
       return intreq;
}

void
Paula::pokeINTREQ(uint16_t value)
{
    debug("pokeINTREQ(%X)\n", value);
    
    if (value & 0x8000) intreq |= (value & 0x7FFF); else intreq &= ~value;
}

uint16_t
Paula::peekINTENA()
{
    return intena;
}

void
Paula::pokeINTENA(uint16_t value)
{
    debug("pokeINTENA(%X)\n", value);
    
    if (value & 0x8000) intena |= (value & 0x7FFF); else intena &= ~value;
}

int
Paula::interruptLevel()
{
    uint16_t mask = intreq & intena;

    if (mask & 0b0110000000000000) return 6;
    if (mask & 0b0001100000000000) return 5;
    if (mask & 0b0000011110000000) return 4;
    if (mask & 0b0000000001110000) return 3;
    if (mask & 0b0000000000001000) return 2;
    if (mask & 0b0000000000000111) return 1;
    
    return 0;
}
