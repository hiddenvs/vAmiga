// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGACOMPONENT_INC
#define _AMIGACOMPONENT_INC

#include "AmigaObject.h"

// Forward declarations
class Amiga;


/* Base class for all hardware components
 * This class defines the base functionality of all hardware components.
 * it comprises functions for powering up and down, resetting, suspending and
 * resuming, as well as functions for loading and saving snapshots.
 */
class HardwareComponent : public AmigaObject {
    
protected:
    
    /* Type and behavior of a snapshot item
     * The format flags are important when big chunks of data are specified.
     * They are needed in functions loadBuffer and saveBuffer to correctly
     * convert little endian to big endian format.
     * If the PERSISTANT flag is set, the snapshot won't be zeroed out in
     * powerOn().
     */
    enum {
        BYTE_ARRAY       = 0x01, //! Data chunk is an array of bytes
        WORD_ARRAY       = 0x02, //! Data chunk is an array of words
        DWORD_ARRAY      = 0x04, //! Data chunk is an array of double words
        QWORD_ARRAY      = 0x08, //! Data chunk is an array of quad words
        
        PERSISTANT       = 0x10, //! Don't zero out in powerOn()

    };
    
    /* Fingerprint of a snapshot item
     */
    typedef struct {
        
        union {
            void *data;
            uint8_t *data8;
            uint16_t *data16;
            uint32_t *data32;
            uint64_t *data64;
        };
        size_t size;
        uint8_t flags;
        
    } SnapshotItem;
    
public:
    
    /* Reference to the Amiga top-level object.
     * Because nearly all hardware components need to interact very closely
     * with each other, we keep a reference to the top-level object in each
     * component. Hence, each component can access every other component every
     * time.
     */
    Amiga *amiga = NULL;
    
protected:
    
    // Sub components of this component
    vector<HardwareComponent *> subComponents;
    
    // List of snapshot items of this component
    vector<SnapshotItem> snapshotItems;
    
    // Snapshot size on disk (in bytes)
    unsigned snapshotSize = 0;
    
    /* State model:
     * The virtual hardware components can be in three different states,
     * called 'Off', 'Paused', and 'Running'. The current state is determined
     * by variables 'power' and 'running' according to the following table:
     *
     *     power     | running    | State
     *     --------------------------------------
     *     false     | false      | Off
     *     false     | true       | INVALID
     *     true      | false      | Paused
     *     true      | true       | Running
     */
    bool power = false;
    bool running = false;
    
    // Indicates if this component should run in warp mode
    bool warp = false;
    
public:
    
    virtual ~HardwareComponent();
    
    
    //
    // Initializing the component
    //
    
    /* Assign top-level C64 object.
     * The provided reference is propagated automatically to all sub components.
     * This functions is called in the constructor of the Amiga class and never
     * called again afterwards.
     */
    virtual void setAmiga(Amiga *amiga);
    
    /* There are several functions for querying and changing state:
     *
     *          -----------------------------------------------
     *         |                    powerOn()                  |
     *         |                                               V
     *     ---------   powerOn()   ---------     run()     ---------
     *    |   Off   |------------>| Paused  |------------>| Running |
     *    |         |<------------|         |<------------|         |
     *     ---------   powerOff()  ---------    pause()    ---------
     *         ^                                               |
     *         |                   powerOff()                  |
     *          -----------------------------------------------
     *
     *     isPoweredOff()                  isPoweredOn()
     * |-------------------||----------------------------------------|
     *                      |-------------------||-------------------|
     *                            isPaused()          isRunning()
     */
    
    bool isPoweredOn() { return power; }
    bool isPoweredOff() { return !power; }
    bool isPaused() { return power && !running; }
    bool isRunning() { return running; }
    
    /* powerOn() powers the component on.
     *
     * current   | next      | action
     * -------------------------------------------------------------------------
     * off       | paused    | _powerOn() on each sub component
     * paused    | paused    | none
     * running   | running   | none
     */
    void powerOn();
    virtual void _powerOn() { }
    
    /* powerOff() powers the component off.
     *
     * current   | next      | action
     * -------------------------------------------------------------------------
     * off       | off       | none
     * paused    | off       | _powerOff() on each sub component
     * running   | off       | pause(), _powerOff() on each sub component
     */
    void powerOff();
    virtual void _powerOff() { }
    
    /* run() puts the component in 'running' state.
     *
     * current   | next      | action
     * -------------------------------------------------------------------------
     * off       | running   | powerOn(), _run() on each sub component
     * paused    | running   | _run() on each sub component
     * running   | running   | none
     */
    void run();
    virtual void _run() { }
    
    /* pause() puts the component in 'paused' state.
     *
     * current   | next      | action
     * -------------------------------------------------------------------------
     * off       | off       | none
     * paused    | paused    | none
     * running   | paused    | _pause() on each sub component
     */
    virtual void pause();
    virtual void _pause() { };
    
    /* Emulates a reset event on the virtual Amiga.
     * By default, each component resets its sub components.
     */
    virtual void reset();
    virtual void _reset() { }
  

    /* Asks the component to inform the GUI about its current state.
     * The GUI invokes this function when it needs to update all of its visual
     * elements. This happens, e.g., when a snapshot file was loaded.
     */
    void ping();
    virtual void _ping() { }
    
    // Dumps some debug information about the internal state to the console.
    void dump();
    virtual void _dump() { }
    
    
    // Getter for warp mode
    bool getWarp() { return warp; }
    
    // Switches warp mode on or off
    void setWarp(bool value);
    virtual void _setWarp(bool value) { }
    
    
    //
    // Registering snapshot items and sub components
    //
    
    /* Registers the subcomponents of this component.
     * This function is called once (in the constructor).
     */
    void registerSubcomponents(vector<HardwareComponent *> components);
        
    /* Registers the snapshot items of this component.
     * This function is called once (in the constructor).
     */
    void registerSnapshotItems(vector<SnapshotItem> items);
    
    
public:
    
    //
    // Loading and saving snapshots
    //
    
    // Returns the size of the internal state in bytes.
    virtual size_t stateSize();
    
    /* Loads the internal state from a memory buffer.
     */
    void loadFromBuffer(uint8_t **buffer);
    
    /* Delegation methods called inside loadFromBuffer()
     * A component can override this method to add custom behavior if not all
     * elements can be processed by the default implementation.
     */
    virtual void  willLoadFromBuffer(uint8_t **buffer) { };
    virtual void  didLoadFromBuffer(uint8_t **buffer) { };
    
    /* Saves the internal state to a memory buffer.
     */
    void saveToBuffer(uint8_t **buffer);
    
    /* Delegation methods called inside saveToBuffer()
     * A component can override this method to add custom behavior if not all
     * elements can be processed by the default implementation.
     */
    virtual void  willSaveToBuffer(uint8_t **buffer) { };
    virtual void  didSaveToBuffer(uint8_t **buffer) { };
};

#endif