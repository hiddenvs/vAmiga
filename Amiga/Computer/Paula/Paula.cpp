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
    HardwareComponent *subcomponents[] = {
        
        &audioUnit,
        NULL };
    
    registerSubcomponents(subcomponents, sizeof(subcomponents));
    
    // Register snapshot items
    SnapshotItem items[] = {
        
        { NULL,                0,                          0 }};
    
    registerSnapshotItems(items, sizeof(items));
    
}
