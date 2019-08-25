// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _DMA_DEBUGGER_INC
#define _DMA_DEBUGGER_INC

#include "HardwareComponent.h"
#include "Colors.h"

class DmaDebugger : public HardwareComponent {

private:

    // Indicates if DMA debugging is turned on or off
    bool enabled = false;

    // Indicates if a certain DMA channel should be visualized
    bool visualize[BUS_OWNER_COUNT]; 

    // DMA debugging colors
    RgbColor debugColor[BUS_OWNER_COUNT][5];

    // Opacity of DMA pixels
    double opacity = 0.5;

    // Selects how DMA pixels are draw (overlayed on top or mixed)
    bool overlay = true;


    //
    // Constructing and destructing
    //

public:

    DmaDebugger();

    // Returns the current settings
    DMADebuggerInfo getInfo();


    //
    // Methods from HardwareComponent
    //

private:

    void _reset() override { }
    size_t _size() override { return 0; }
    size_t _load(uint8_t *buffer) override {return 0; }
    size_t _save(uint8_t *buffer) override { return 0; }
    

    //
    // Configuring the device
    //

public:

    // Turns DMA debugging on or off
    bool isEnabled() { return enabled; }
    void setEnabled(bool value);

    // Enables or disables the visual effects for a certain DMA source
    bool isVisualized(BusOwner owner);
    void setVisualized(BusOwner owner, bool value);

    // Gets or sets a debug color
    RgbColor getColor(BusOwner owner);
    void setColor(BusOwner owner, RgbColor color);
    void setColor(BusOwner owner, double r, double g, double b);

    // Gets or sets the opacity of the superimposed visual effect
    double getOpacity();
    void setOpacity(double value);

    // Gets or sets the overlay flag
    bool getOverlay() { return overlay; }
    void setOverlay(bool value) { overlay = value; }


    //
    // Running the debugger
    //

    // Superimposes the debug output onto the current rasterline
    void computeOverlay();

    // Cleans up some texture data at the end of each frame
    void vSyncHandler();
};

#endif

