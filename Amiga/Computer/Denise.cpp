// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Denise::Denise()
{
    setDescription("Denise");
}

Denise::~Denise()
{
    debug(2, "Destroying Denise[%p]\n", this);
}

void
Denise::_powerOn()
{
    clock = 0;
    frame = 0;
    frameBuffer = longFrame;
    pixelBuffer = frameBuffer;
}
void
Denise::_powerOff()
{
    
}
void
Denise::_reset()
{
    
}
void
Denise::_ping()
{
    
}
void
Denise::_dump()
{
    msg("Frame: %lld\n", frame);
}

void
Denise::executeUntil(uint64_t targetClock)
{
    clock = targetClock;
    uint64_t targetFrame = clock / masterCyclesPerFrame;
    
    if (frame != targetFrame) {
        
        // Next frame has been reached
        frame = targetFrame;
        endOfFrame();
    }
}


void
Denise::endOfFrame()
{
    // Switch the active frame buffer
    frameBuffer = (frameBuffer == longFrame) ? shortFrame : longFrame;
    pixelBuffer = frameBuffer;
    
    // Toggle the fake image from time to time
    if ((frame / 25) % 2) {
        memcpy((void *)longFrame, fakeImage1, BUFSIZE);
        memcpy((void *)shortFrame, fakeImage1, BUFSIZE);
    } else {
        memcpy((void *)longFrame, fakeImage2, BUFSIZE);
        memcpy((void *)shortFrame, fakeImage2, BUFSIZE);
    }
    
    // Take a snapshot once in a while
    if (amiga->getTakeAutoSnapshots() && amiga->getSnapshotInterval() > 0) {
        unsigned fps = 50;
        if (frame % (fps * amiga->getSnapshotInterval()) == 0) {
            amiga->takeAutoSnapshot();
        }
    }
    
    // Count some sheep (zzzzzz) ...
    if (!amiga->getWarp()) {
        amiga->synchronizeTiming();
    }
}
