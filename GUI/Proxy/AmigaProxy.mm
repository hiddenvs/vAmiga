// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#import "AmigaProxy.h"
#import "Amiga.h"
#import "vAmiga-Swift.h"

struct MemWrapper { AmigaMemory *mem; };
struct DMAControllerWrapper { DMAController *dmaController; };
struct DeniseWrapper { Denise *denise; };
struct AmigaWrapper { Amiga *amiga; };


//
// Memory proxy
//

@implementation MemProxy

- (instancetype) initWithMemory:(AmigaMemory *)mem
{
    if (self = [super init]) {
        wrapper = new MemWrapper();
        wrapper->mem = mem;
    }
    return self;
}
- (void) dump
{
    wrapper->mem->dump();
}
@end


//
// DMAController proxy
//

@implementation DMAControllerProxy

- (instancetype) initWithDMAController:(DMAController *)dmaController
{
    if (self = [super init]) {
        wrapper = new DMAControllerWrapper();
        wrapper->dmaController = dmaController;
    }
    return self;
}
- (void) dump
{
    wrapper->dmaController->dump();
}

@end


//
// Denise proxy
//

@implementation DeniseProxy

- (instancetype) initWithDenise:(Denise *)denise
{
    if (self = [super init]) {
        wrapper = new DeniseWrapper();
        wrapper->denise = denise;
    }
    return self;
}
- (void) dump
{
    wrapper->denise->dump();
}
- (void) initFakePictures:(void *)fake1 fake2:(void *)fake2
{
    wrapper->denise->initFakePictures((int *)fake1, (int *)fake2);
}
- (void *) screenBuffer
{
    return wrapper->denise->screenBuffer();
}
@end


//
// Amiga
//

@implementation AmigaProxy

@synthesize wrapper;
@synthesize mem;
@synthesize dma;
@synthesize denise;

- (instancetype) init
{
    NSLog(@"AmigaProxy::init");
    
    if (!(self = [super init]))
        return self;
    
    Amiga *amiga = new Amiga();
    wrapper = new AmigaWrapper();
    wrapper->amiga = amiga;
    
    // Create sub proxys
    mem = [[MemProxy alloc] initWithMemory:&amiga->mem];
    dma = [[DMAControllerProxy alloc] initWithDMAController:&amiga->dma];
    denise = [[DeniseProxy alloc] initWithDenise:&amiga->denise];
    
    return self;
}

- (void) kill
{
    assert(wrapper->amiga != NULL);
    NSLog(@"AmigaProxy::kill");
    
    // Kill the emulator
    delete wrapper->amiga;
    wrapper->amiga = NULL;
}

- (BOOL) releaseBuild
{
    return releaseBuild(); // see vastd.h
}

- (void) powerOn
{
    return wrapper->amiga->powerOn();
}
- (void) powerOff
{
    return wrapper->amiga->powerOff();
}
- (void) powerOnOrOff
{
    return wrapper->amiga->powerOnOrOff();
}
- (void) reset
{
    return wrapper->amiga->reset();
}
- (void) ping
{
    return wrapper->amiga->ping();
}
- (void) dump
{
    return wrapper->amiga->dump();
}

- (BOOL) readyToPowerUp
{
    return wrapper->amiga->readyToPowerUp();
}
- (BOOL) isRunning
{
    return wrapper->amiga->isRunning();
}
- (BOOL) isPaused
{
    return wrapper->amiga->isPaused();
}
- (void) run
{
    wrapper->amiga->run();
}
- (void) pause
{
    wrapper->amiga->pause();
}
- (void) runOrPause
{
    wrapper->amiga->runOrPause();
}
- (void) suspend
{
    return wrapper->amiga->suspend();
}
- (void) resume
{
    return wrapper->amiga->resume();
}

- (AmigaConfiguration) config
{
    return wrapper->amiga->getConfig();
}
- (BOOL) configureModel:(NSInteger)model
{
    return wrapper->amiga->configureModel((AmigaModel)model);
}
- (BOOL) configureChipMemory:(NSInteger)size
{
    return wrapper->amiga->configureChipMemory((unsigned)size);
}
- (BOOL) configureSlowMemory:(NSInteger)size
{
    return wrapper->amiga->configureSlowMemory((unsigned)size);
}
- (BOOL) configureFastMemory:(NSInteger)size
{
    return wrapper->amiga->configureFastMemory((unsigned)size);
}
- (BOOL) configureRealTimeClock:(BOOL)value
{
    return wrapper->amiga->configureRealTimeClock(value);
}
- (BOOL) configureDrive:(NSInteger)driveNr connected:(BOOL)value
{
    return wrapper->amiga->configureDrive((unsigned)driveNr, value);
}
- (BOOL) configureDrive:(NSInteger)driveNr type:(NSInteger)type
{
    return wrapper->amiga->configureDrive((unsigned)driveNr, (DriveType)type);
}

- (BOOL) hasBootRom
{
    return wrapper->amiga->hasBootRom();
}
- (void) deleteBootRom
{
    wrapper->amiga->deleteBootRom();
}
- (BOOL) isBootRom:(NSURL *)url
{
    return BootRom::isBootRomFile([[url path] UTF8String]);
}
- (BOOL) loadBootRomFromBuffer:(NSData *)data
{
    if (data == NULL) return NO;
    const uint8_t *bytes = (const uint8_t *)[data bytes];
    return wrapper->amiga->loadBootRomFromBuffer(bytes, [data length]);
}
- (BOOL) loadBootRomFromFile:(NSURL *)url
{
    return wrapper->amiga->loadBootRomFromFile([[url path] UTF8String]);
}
- (uint64_t) bootRomFingerprint
{
    return wrapper->amiga->bootRomFingerprint();
}
- (BOOL) hasKickRom
{
    return wrapper->amiga->hasKickRom();
}
- (void) deleteKickRom
{
    wrapper->amiga->deleteKickRom();
}
- (BOOL) isKickRom:(NSURL *)url
{
    return KickRom::isKickRomFile([[url path] UTF8String]);
}
- (BOOL) loadKickRomFromBuffer:(NSData *)data
{
    if (data == NULL) return NO;
    const uint8_t *bytes = (const uint8_t *)[data bytes];
    return wrapper->amiga->loadKickRomFromBuffer(bytes, [data length]);
}
- (BOOL) loadKickRomFromFile:(NSURL *)url
{
    return wrapper->amiga->loadKickRomFromFile([[url path] UTF8String]);
}
- (uint64_t) kickRomFingerprint
{
    return wrapper->amiga->kickRomFingerprint();
}





- (void) addListener:(const void *)sender function:(Callback *)func
{
    wrapper->amiga->addListener(sender, func);
}
- (void) removeListener:(const void *)sender
{
    wrapper->amiga->removeListener(sender);
}
- (Message)message
{
    return wrapper->amiga->getMessage();
}

- (BOOL) alwaysWarp
{
    return wrapper->amiga->getAlwaysWarp();
}
- (void) setAlwaysWarp:(BOOL)value
{
    wrapper->amiga->setAlwaysWarp(value);
}
- (BOOL) warpLoad
{
    return wrapper->amiga->getWarpLoad();
}
- (void) setWarpLoad:(BOOL)value
{
    wrapper->amiga->setWarpLoad(value);
}
@end
