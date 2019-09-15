// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

//
// Static class variables
//

bool Amiga::debugMode = false;
EventID Amiga::inspectionTarget = INS_NONE;


//
// Emulator thread
//

void
threadTerminated(void* thisAmiga)
{
    assert(thisAmiga != NULL);
    
    // Inform the Amiga that the thread has been canceled
    Amiga *amiga = (Amiga *)thisAmiga;
    amiga->threadDidTerminate();
}

void
*threadMain(void *thisAmiga) {
    
    assert(thisAmiga != NULL);
    
    // Inform the Amiga that the thread is about to start
    Amiga *amiga = (Amiga *)thisAmiga;
    amiga->threadWillStart();
    
    // Configure the thread
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    pthread_cleanup_push(threadTerminated, thisAmiga);
    
    // Enter the run loop
    amiga->runLoop();
    
    // Clean up and exit
    pthread_cleanup_pop(1);
    pthread_exit(NULL);
}


//
// Amiga Class
//

Amiga::Amiga()
{
    setDescription("Amiga");

    /* The order of sub-components is important here, because some components
     * are dependend on others during initialization. I.e.,
     *
     * - The control ports, the serial Controller, the disk controller, and the
     *   disk drives must preceed the CIAs, because the CIA port values depend
     *   on these devices.
     *
     * - The CIAs must preceed memory, because they determine if the lower
     *   memory banks are overlayed by Rom.
     *
     * - Memory mus preceed the CPU, because it contains the CPU reset vector.
     */

    subComponents = vector<HardwareComponent *> {

        &agnus,
        &rtc,
        &denise,
        &paula,
        &zorro,
        &controlPort1,
        &controlPort2,
        &serialPort,
        &mouse,
        &joystick1,
        &joystick2,
        &keyboard,
        &df0,
        &df1,
        &df2,
        &df3,
        &ciaA,
        &ciaB,
        &mem,
        &cpu,
    };

    // Set up initial state
    initialize();
    reset();

    // Initialize the mach timer info
    mach_timebase_info(&tb);
}

Amiga::~Amiga()
{
    debug("Destroying Amiga[%p]\n", this);
    powerOff();
}

void
Amiga::setDebugMode(bool enable)
{
    if ((debugMode = enable)) {
        
        debug("Enabling debug mode\n");
        setControlFlags(RL_ENABLE_TRACING | RL_ENABLE_BREAKPOINTS);

    } else {

        debug("Disabling debug mode\n");
        clearControlFlags(RL_ENABLE_TRACING | RL_ENABLE_BREAKPOINTS);
    }
}

void
Amiga::setInspectionTarget(EventID id)
{
    suspend();
    inspectionTarget = id;
    agnus.scheduleRel<INS_SLOT>(0, inspectionTarget);
    resume();
}

void
Amiga::clearInspectionTarget()
{
    setInspectionTarget(INS_NONE);
}

AmigaInfo
Amiga::getInfo()
{
    AmigaInfo result;
    
    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);
    
    return result;
}

AmigaConfiguration
Amiga::getConfig()
{
    AmigaConfiguration config;
    
    config.model = model;
    config.realTimeClock = realTimeClock;
    config.layout = keyboard.layout;
    config.filterActivation = paula.audioUnit.getFilterActivation();
    config.filterType = paula.audioUnit.getFilterType();
    config.cpuEngine = CPU_MUSASHI;
    config.cpuSpeed = cpu.getSpeed();
    config.cpu = cpu.getConfig();
    config.mem = mem.getConfig();
    config.denise = denise.getConfig();
    config.serialDevice = serialPort.getDevice();
    config.blitter = agnus.blitter.getConfig(); 
    config.diskController = paula.diskController.getConfig();
    config.df0 = df0.getConfig();
    config.df1 = df1.getConfig();
    config.df2 = df2.getConfig();
    config.df3 = df3.getConfig();

    return config;
}

bool
Amiga::configure(ConfigOption option, long value)
{
    AmigaConfiguration current = getConfig();
    
    switch (option) {

        case VA_AMIGA_MODEL:
            
            if (!isAmigaModel(value)) {
                warn("Invalid Amiga model: %d\n", value);
                warn("       Valid values: %d, %d, %d\n", AMIGA_500, AMIGA_1000, AMIGA_2000);
                return false;
            }
            
            if (current.model == value) return true;
            model = (AmigaModel)value;
            
            // Apply model specific config changes
            if (model == AMIGA_2000) realTimeClock = true;
            mem.updateMemSrcTable();
            break;
        
        case VA_KB_LAYOUT:
            
            if (current.layout == value) return true;
            keyboard.layout = value;
            break;
            
        case VA_CHIP_RAM:
            
            if (value != 256 && value != 512) {
                warn("Invalid Chip Ram size: %d\n", value);
                warn("         Valid values: 256KB, 512KB\n");
                return false;
            }
            
            mem.allocateChipRam(KB(value));
            break;
    
        case VA_SLOW_RAM:
            
            if ((value % 256) != 0 || value > 512) {
                warn("Invalid Slow Ram size: %d\n", value);
                warn("         Valid values: 0KB, 256KB, 512KB\n");
                return false;
            }
            
            mem.allocateSlowRam(KB(value));
            break;
        
        case VA_FAST_RAM:
            
            if ((value % 64) != 0 || value > 8192) {
                warn("Invalid Fast Ram size: %d\n", value);
                warn("         Valid values: 0KB, 64KB, 128KB, ..., 8192KB (8MB)\n");
                return false;
            }
            
            mem.allocateFastRam(KB(value));
            break;
  
        case VA_RT_CLOCK:
            
            if (current.realTimeClock == value) return true;
            realTimeClock = value;
            mem.updateMemSrcTable();
            break;

        case VA_EMULATE_SPRITES:

            if (current.denise.emulateSprites == value) return true;
            denise.setEmulateSprites(value);
            break;

        case VA_CLX_SPR_SPR:

            if (current.denise.clxSprSpr == value) return true;
            denise.setClxSprSpr(value);
            break;

        case VA_CLX_SPR_PLF:

            if (current.denise.clxSprPlf == value) return true;
            denise.setClxSprPlf(value);
            break;

        case VA_CLX_PLF_PLF:

            if (current.denise.clxPlfPlf == value) return true;
            denise.setClxPlfPlf(value);
            break;

        case VA_FILTER_ACTIVATION:

            if (!isFilterActivation(value)) {
                warn("Invalid filter activation: %d\n", value);
                warn("       Valid values: 0 ... %d\n", FILTACT_COUNT - 1);
                return false;
            }

            if (current.filterActivation == value) return true;
            paula.audioUnit.setFilterActivation((FilterActivation)value);
            break;

        case VA_FILTER_TYPE:

            if (!isFilterType(value)) {
                warn("Invalid filter type: %d\n", value);
                warn("       Valid values: 0 ... %d\n", FILT_COUNT - 1);
                return false;
            }

            if (current.filterType == value) return true;
            paula.audioUnit.setFilterType((FilterType)value);
            break;

        case VA_CPU_ENGINE:

            if (!isCPUEngine(value)) {
                warn("Invalid CPU engine: %d\n", value);
                warn("      Valid values: %d\n", CPU_MUSASHI);
                return false;
            }

            if (current.cpuEngine == value) return true;
            // Nothing to do yet, only one engine is supported
            break;

        case VA_CPU_SPEED:

            if (value != 1 && value != 2 && value != 4) {
                warn("Unsupported CPU speed: %d\n", value);
                warn("         Valid values: 1, 2, 4\n");
                return false;
            }

            if (current.cpuSpeed == value) return true;
            cpu.setSpeed(value);
            break;

        case VA_BLITTER_ACCURACY:
            
            if (current.blitter.accuracy == value) return true;
            agnus.blitter.setAccuracy(value);
            break;
            
        case VA_FIFO_BUFFERING:

            if (current.diskController.useFifo == value) return true;
            paula.diskController.setUseFifo(value);
            break;

        case VA_SERIAL_DEVICE:

            if (!isSerialPortDevice(value)) {
                warn("Invalid serial port device: %d\n", value);
                return false;
            }

            if (current.serialDevice == value) return true;
            serialPort.connectDevice((SerialPortDevice)value);
            break;

        case VA_DRIVE_SPEED:

            if (!isValidDriveSpeed(value)) {
                warn("Invalid drive speed: %d\n", value);
                return false;
            }

            paula.diskController.setSpeed(value);
            break;

        default: assert(false);
    }
    
    putMessage(MSG_CONFIG);
    return true;
}

bool
Amiga::configureDrive(unsigned drive, ConfigOption option, long value)
{
    if (drive >= 4) {
        warn("Invalid drive number: %d\n");
        return false;
    }
    
    DriveConfig current =
    drive == 0 ? getConfig().df0 :
    drive == 1 ? getConfig().df1 :
    drive == 2 ? getConfig().df2 : getConfig().df3;
    
    switch (option) {
            
        case VA_DRIVE_CONNECT:
            
            if (drive == 0 && value == false) {
                warn("Df0 cannot be disconnected. Ignoring.\n");
                return false;
            }

            if (getConfig().diskController.connected[drive] == value) return true;
            paula.diskController.setConnected(drive, value);
            break;
            
        case VA_DRIVE_TYPE:
            
            if (!isDriveType(value)) {
                warn("Invalid drive type: %d\n", value);
                return false;
            }
            
            if (value != DRIVE_35_DD) {
                warn("Unsupported drive type: %s\n", driveTypeName((DriveType)value));
                return false;
            }
            
            if (current.type == value) return true;
            df[drive]->setType((DriveType)value);
            break;

        default: assert(false);
    }
    
    putMessage(MSG_CONFIG);
    return true;
}


bool
Amiga::configureModel(AmigaModel m)
{
    return configure(VA_AMIGA_MODEL, m);
}

bool
Amiga::configureLayout(long layout)
{
    return configure(VA_KB_LAYOUT, layout);
}

bool
Amiga::configureChipMemory(long size)
{
    return configure(VA_CHIP_RAM, size);
}

bool
Amiga::configureSlowMemory(long size)
{
    return configure(VA_SLOW_RAM, size);
}

bool
Amiga::configureFastMemory(long size)
{
    return configure(VA_FAST_RAM, size);
}

void
Amiga::configureRealTimeClock(bool value)
{
    configure(VA_RT_CLOCK, value);
}

void
Amiga::prefix() const
{
    fprintf(stderr, "[%lld] (%3d,%3d) ",
            agnus.frame, agnus.pos.v, agnus.pos.h);

    fprintf(stderr, " %06X ", cpu.getPC());

    uint16_t dmacon = agnus.dmacon;
    bool dmaen = dmacon & DMAEN;
    fprintf(stderr, "%c%c%c%c%c%c ",
            (dmacon & BPLEN) ? (dmaen ? 'B' : 'B') : '-',
            (dmacon & COPEN) ? (dmaen ? 'C' : 'c') : '-',
            (dmacon & BLTEN) ? (dmaen ? 'B' : 'b') : '-',
            (dmacon & SPREN) ? (dmaen ? 'S' : 's') : '-',
            (dmacon & DSKEN) ? (dmaen ? 'D' : 'd') : '-',
            (dmacon & AUDEN) ? (dmaen ? 'A' : 'a') : '-');

    fprintf(stderr, "%04X %04X ", paula.intena, paula.intreq);

    if (agnus.copper.servicing) {
        fprintf(stderr, "[%06X] ", agnus.copper.getCopPC());
    }

    /*
    if (getDescription())
        fprintf(stderr, "%s: ", getDescription());
    */
}

void
Amiga::reset()
{
    suspend();

    assert(!isRunning());
    
    // Execute the standard reset routine
    HardwareComponent::reset();

    // Inform the GUI
    putMessage(MSG_RESET);

    resume();
}

void
Amiga::_initialize()
{
 
}

void
Amiga::_powerOn()
{
    debug(1, "Power on\n");

#ifdef BOOT_DISK

    ADFFile *adf = ADFFile::makeWithFile(BOOT_DISK);
    if (adf) df0.insertDisk(adf);

#endif

    runLoopCtrl = 0;
    
    // For debugging, we start in debug mode and set a breakpoint
    debugMode = true;

    // cpu.bpManager.setBreakpointAt(0xFD7962); // AROS ERROR
    // cpu.bpManager.setBreakpointAt(0xF890C6); //
    // cpu.bpManager.setBreakpointAt(0xFD822E); // Jumps to error output routine

    // cpu.bpManager.setBreakpointAt(0xFD818C); // BEFORE AUD registers
    // cpu.bpManager.setBreakpointAt(0xFA93E6); // FIRST FMODE non-OCS access
    // cpu.bpManager.setBreakpointAt(0xFD7982); // WRITE TO BPLCON3
    // cpu.bpManager.setBreakpointAt(0xFD81C8); // Coming back from resetsprite()
    // cpu.bpManager.setBreakpointAt(0xF8AB64); // Jump to FindResident in TaggedOpenLibrary

    // Hunting Boot Rom bug
    // cpu.bpManager.setBreakpointAt(0xF8008C); // Boot Rom start
    // cpu.bpManager.setBreakpointAt(0xF800B4); // Setting bg color to grey
    // cpu.bpManager.setBreakpointAt(0xF8011E); //
    // cpu.bpManager.setBreakpointAt(0xF80082); // Setting bg color to cyan (error)
    // cpu.bpManager.setBreakpointAt(0xF802E0); // Tests if mem can be written to

    // cpu.bpManager.setBreakpointAt(0x034434); // Shadow of the beast, DSKLEN POKE
    // cpu.bpManager.setBreakpointAt(0xFC54FC);


    // Update the recorded debug information
    inspect();

    putMessage(MSG_POWER_ON);
}

void
Amiga::_powerOff()
{
    debug(1, "Power off\n");
    
    // Update the recorded debug information
    inspect();
    
    putMessage(MSG_POWER_OFF);
}

void
Amiga::_run()
{
    // Check for missing Roms
    if (!readyToPowerUp()) {
        putMessage(MSG_ROM_MISSING);
        return;
    }
    
    // REMOVE ASAP
    if (SNAP_DEBUG == 1) {
        debug("Creating snapshot\n");
        Snapshot *snap = Snapshot::makeWithAmiga(this);
        debug("Snap at %p created\n");
        delete snap;
    }

    debug(RUNLOOP_DEBUG, "Starting emulation thread (PC = %X, irq_handler = %p)\n", cpu.getPC(), cpu.getIrqHandler());


    // Start the emulator thread.
    pthread_create(&p, NULL, threadMain, (void *)this);
    
    // Inform the GUI.
    putMessage(MSG_RUN);
}

void
Amiga::_pause()
{
    // Cancel the emulator thread if it still running
    if (p) signalStop();
    
    // Wait until the thread has terminated
    pthread_join(p, NULL);
    
    // Update the recorded debug information
    inspect();

    // Inform the GUI
    putMessage(MSG_PAUSE);
}

void
Amiga::_ping()
{
    putMessage(warp ? MSG_WARP_ON : MSG_WARP_OFF);
}

void
Amiga::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);
    
    info.cpuClock = cpu.clock;
    info.dmaClock = agnus.clock;
    info.ciaAClock = ciaA.clock;
    info.ciaBClock = ciaB.clock;
    info.frame = agnus.frame;
    info.vpos = agnus.pos.v;
    info.hpos = agnus.pos.h;
    
    pthread_mutex_unlock(&lock);
}

void
Amiga::_dump()
{
    AmigaConfiguration config = getConfig();
    DiskControllerConfig dc = config.diskController;

    dumpClock();
    plainmsg("    poweredOn: %s\n", isPoweredOn() ? "yes" : "no");
    plainmsg("   poweredOff: %s\n", isPoweredOff() ? "yes" : "no");
    plainmsg("       paused: %s\n", isPaused() ? "yes" : "no");
    plainmsg("      running: %s\n", isRunning() ? "yes" : "no");
    plainmsg("\n");
    plainmsg("Current configuration:\n\n");
    plainmsg("   AmigaModel: %s\n", modelName(config.model));
    plainmsg("realTimeClock: %s\n", config.realTimeClock ? "yes" : "no");
    plainmsg("          df0: %s %s\n",
             dc.connected[0] ? "yes" : "no", driveTypeName(config.df0.type));
    plainmsg("          df1: %s %s\n",
             dc.connected[1] ? "yes" : "no", driveTypeName(config.df1.type));
    plainmsg("          df2: %s %s\n",
             dc.connected[2] ? "yes" : "no", driveTypeName(config.df2.type));
    plainmsg("          df3: %s %s\n",
             dc.connected[3] ? "yes" : "no", driveTypeName(config.df3.type));

    plainmsg("\n");
    plainmsg("         warp: %d", warp);
    plainmsg("\n");
}

void
Amiga::_warpOn()
{
putMessage(MSG_WARP_ON);
}

void
Amiga::_warpOff()
{
    restartTimer();
    putMessage(MSG_WARP_OFF);
}

void
Amiga::suspend()
{
    debug(2, "Suspending (%d)...\n", suspendCounter);
    
    if (suspendCounter == 0 && !isRunning())
    return;
    
    pause();
    suspendCounter++;
}

void
Amiga::resume()
{
    debug(2, "Resuming (%d)...\n", suspendCounter);
    
    if (suspendCounter == 0)
    return;
    
    if (--suspendCounter == 0)
    run();
}

bool
Amiga::readyToPowerUp()
{
    AmigaConfiguration config = getConfig();
    
    // Check for Chip Ram
    if (!mem.hasChipRam()) {
        msg("NOT READY YET: Chip Ram is missing.\n");
        return false;
    }
    
    // Check for a Boot Rom (A1000 only)
    if (config.model == AMIGA_1000 && !mem.hasBootRom()) {
        msg("NOT READY YET: Boot Rom is missing.\n");
        return false;
    }
    
    // Check for a Kickstart Rom (A500, A2000)
    if (config.model != AMIGA_1000 && !mem.hasKickRom()) {
        msg("NOT READY YET: Kickstart Rom is missing.\n");
        return false;
    }

    // msg("Ready to power up\n");
    return true;
}

void
Amiga::setControlFlags(uint32_t flags)
{
    pthread_mutex_lock(&lock);
    runLoopCtrl |= flags;
    pthread_mutex_unlock(&lock);
}

void
Amiga::clearControlFlags(uint32_t flags)
{
    pthread_mutex_lock(&lock);
    runLoopCtrl &= ~flags;
    pthread_mutex_unlock(&lock);
}

void
Amiga::restartTimer()
{
    timeBase = time_in_nanos();
    clockBase = agnus.clock;
    
    // uint64_t kernelNow = mach_absolute_time();
    // uint64_t nanoNow = abs_to_nanos(kernelNow);
}

void
Amiga::synchronizeTiming()
{
    uint64_t now        = time_in_nanos();
    Cycle clockDelta    = agnus.clock - clockBase;
    int64_t elapsedTime = (clockDelta * 1000) / masterClockFrequency;
    int64_t targetTime  = timeBase + elapsedTime;
    
    /*
     debug("now         = %lld\n", now);
     debug("clockDelta  = %lld\n", clockDelta);
     debug("elapsedTime = %lld\n", elapsedTime);
     debug("targetTime  = %lld\n", targetTime);
     debug("\n");
     */
    
    // Check if we're running too slow ...
    if (now > targetTime) {
        
        // Check if we're completely out of sync ...
        if (now - targetTime > 200000000) {
            
            // warn("The emulator is way too slow (%lld).\n", now - targetTime);
            restartTimer();
            return;
        }
    }
    
    // Check if we're running too fast ...
    if (now < targetTime) {
        
        // Check if we're completely out of sync ...
        if (targetTime - now > 200000000) {
            
            warn("The emulator is way too fast (%lld).\n", targetTime - now);
            restartTimer();
            return;
        }
        
        // See you soon...
        mach_wait_until(targetTime);
        /*
         int64_t jitter = sleepUntil(targetTime, 1500000); // 1.5 usec early wakeup
         if (jitter > 1000000000) { // 1 sec
         warn("Jitter is too high (%lld).\n", jitter);
         // restartTimer();
         }
         */
    }
}

bool
Amiga::snapshotIsDue()
{
    unsigned fps = 50; // PAL frames per second
    
    if (!getTakeAutoSnapshots() || getSnapshotInterval() <= 0)
    return false;
    
    return agnus.frame % (fps * getSnapshotInterval()) == 0;
}

void
Amiga::loadFromSnapshotUnsafe(Snapshot *snapshot)
{
    uint8_t *ptr;
    
    if (snapshot && (ptr = snapshot->getData())) {
        load(ptr);
        // deserializeFromBuffer(*this, ptr);
        ping();
    }
}

void
Amiga::loadFromSnapshotSafe(Snapshot *snapshot)
{
    debug(2, "Amiga::loadFromSnapshotSafe\n");
    
    suspend();
    loadFromSnapshotUnsafe(snapshot);
    resume();
}

bool
Amiga::restoreSnapshot(vector<Snapshot *> &storage, unsigned nr)
{
    Snapshot *snapshot = getSnapshot(storage, nr);
    
    if (snapshot) {
        loadFromSnapshotSafe(snapshot);
        return true;
    }
    
    return false;
}

size_t
Amiga::numSnapshots(vector<Snapshot *> &storage)
{
    return storage.size();
}

Snapshot *
Amiga::getSnapshot(vector<Snapshot *> &storage, unsigned nr)
{
    return nr < storage.size() ? storage.at(nr) : NULL;
    
}

void
Amiga::takeSnapshot(vector<Snapshot *> &storage)
{
    // Delete oldest snapshot if capacity limit has been reached
    if (storage.size() >= MAX_SNAPSHOTS) {
        deleteSnapshot(storage, MAX_SNAPSHOTS - 1);
    }
    
    Snapshot *snapshot = Snapshot::makeWithAmiga(this);
    storage.insert(storage.begin(), snapshot);
    putMessage(MSG_SNAPSHOT_TAKEN);
}

void
Amiga::deleteSnapshot(vector<Snapshot *> &storage, unsigned index)
{
    Snapshot *snapshot = getSnapshot(storage, index);
    
    if (snapshot) {
        delete snapshot;
        storage.erase(storage.begin() + index);
    }
}


//
// The run loop
//

void
Amiga::threadWillStart()
{
    debug(2, "Emulator thread started\n");
}

void
Amiga::threadDidTerminate()
{
    debug(2, "Emulator thread terminated\n");
    p = NULL;
    
    /* Put emulator into pause mode. If we got here by a call to pause(), the
     * following (reentrant) call to pause() has no effect. If we got here
     * because a breakpoint was reached, the following call will perform the
     * state transition.
     */
    pause();
}

void
Amiga::stepInto()
{
    if (isRunning())
    return;
    
    cpu.bpManager.setSoftBreakpointAt(UINT32_MAX);
    run();
}

void
Amiga::stepOver()
{
    if (isRunning())
    return;
    
    debug("Setting bp at %X\n", cpu.getNextPC());
    cpu.bpManager.setSoftBreakpointAt(cpu.getNextPC());
    run();
}

void
Amiga::runLoop()
{
    debug(RUNLOOP_DEBUG, "runLoop()\n");

    // Prepare to run
    restartTimer();
    
    // Enable or disable debugging features
    debugMode ? setControlFlags(RL_DEBUG) : clearControlFlags(RL_DEBUG);
    agnus.scheduleRel<INS_SLOT>(0, inspectionTarget);
    
    // Enter the loop
    do {
        
        // Emulate the next CPU instruction
        Cycle newClock = cpu.executeInstruction();

        // Emulate Agnus up to the same cycle
        agnus.executeUntil(newClock);
        
        // Check if special action needs to be taken ...
        if (runLoopCtrl) {
            
            // Are we requested to take a snapshot?
            if (runLoopCtrl & RL_SNAPSHOT) {
                takeAutoSnapshot();
                clearControlFlags(RL_SNAPSHOT);
            }
            
            // Are we requested to update the debugger info structs?
            if (runLoopCtrl & RL_INSPECT) {
                inspect();
                clearControlFlags(RL_INSPECT);
            }
            
            // Are we requested to record the execution?
            if (runLoopCtrl & RL_ENABLE_TRACING) {
                cpu.recordInstruction();
            }
            
            // Are we requested to check for breakpoints?
            if (runLoopCtrl & RL_ENABLE_BREAKPOINTS) {
                if (cpu.bpManager.shouldStop()) {
                    inspect();
                    putMessage(MSG_BREAKPOINT_REACHED);
                    debug(RUNLOOP_DEBUG, "BREAKPOINT_REACHED\n");
                    break;
                }
            }
            
            // Are we requests to terminate the run loop?
            if (runLoopCtrl & RL_STOP) {
                clearControlFlags(RL_STOP);
                debug(RUNLOOP_DEBUG, "RL_STOP\n");
                break;
            }
        }
        
    } while (1);
}

void
Amiga::dumpClock()
{
    plainmsg("               Master cycles     CPU cycles    DMA cycles    CIA cycles\n");
    plainmsg("    CPU clock: %13lld  %13lld %13lld %13lld\n",
             cpu.clock,
             AS_CPU_CYCLES(cpu.clock),
             AS_DMA_CYCLES(cpu.clock),
             AS_CIA_CYCLES(cpu.clock));
    plainmsg("    DMA clock: %13lld  %13lld %13lld %13lld\n",
             agnus.clock,
             AS_CPU_CYCLES(agnus.clock),
             AS_DMA_CYCLES(agnus.clock),
             AS_CIA_CYCLES(agnus.clock));
    plainmsg("  CIA A clock: %13lld  %13lld %13lld %13lld\n",
             ciaA.clock,
             AS_CPU_CYCLES(ciaA.clock),
             AS_DMA_CYCLES(ciaA.clock),
             AS_CIA_CYCLES(ciaA.clock));
    plainmsg("  CIA B clock: %13lld  %13lld %13lld %13lld\n",
             ciaB.clock,
             AS_CPU_CYCLES(ciaB.clock),
             AS_DMA_CYCLES(ciaB.clock),
             AS_CIA_CYCLES(ciaB.clock));
    plainmsg("  Color clock: (%d,%d) hex: ($%X,$%X) Frame: %lld\n",
             agnus.pos.v, agnus.pos.h, agnus.pos.v, agnus.pos.h, agnus.frame);
    plainmsg("\n");
}
