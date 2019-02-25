// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _COLORIZER_INC
#define _COLORIZER_INC

#include "HardwareComponent.h"

class Colorizer : public HardwareComponent {

private:
    
    // The 32 Amiga color registers
    uint16_t colorReg[32];
    
    /* The 64 Amiga colors in RGBA format
     * The first 32 entries represent the colors that are stored in the color
     * registers in RGBA format. The other 32 colors contain the RGBA values
     * used in halfbright mode. They are computed out of the first 32 colors by
     * dividing each color component by 2. 
     */
    uint32_t colorRGBA[64];
    
    /* Color cache for all possible 4096 colors
     * When computeRGBA() is invoked, the function first checks if the color
     * is stored in the color cache. If not, it is computed on-the-fly and
     * stored in the cache. Whenever a color adjustment parameter changes,
     * the color cache is cleared.
     */
    uint32_t colorCache[4096];
    
    //
    // Color adjustment parameters
    //
    
    double brightness = 1.0;
    double saturation = 1.0;
    double contrast = 1.0;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    Colorizer();
    
    //
    // Configuring the color palette
    //
    
    void setBrightness(double value);
    void setSaturation(double value);
    void setContrast(double value);

    
    //
    // Accessing colors and color registers
    //

    // Peeks a value from one of the 32 color registers.
    uint16_t peekColorReg(int reg);

    // Pokes a value into one of the 32 color registers.
    void pokeColorReg(int reg, uint16_t value);
    
    // Reads a color value in RGBA format
    inline uint32_t getRGBA(int nr) { assert(nr < 64); return colorRGBA[nr]; }
    
private:
    
    // Clears the color cache
    void clearColorCache();
    
    // Computes the RGB value for a color stored in a color register
    void computeRGBA(int reg);

public:
    
    /* Updates the 64 colors stored in the colorRGBA lookup table
     * This functions is to be called whenever a color adjustment parameter
     * changes or a snapshot is restored.
     */
    void updateRGBAs();
};

#endif
