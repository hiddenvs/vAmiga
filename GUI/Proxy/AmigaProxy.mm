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

struct AmigaWrapper { Amiga *amiga; };
struct MemWrapper { AmigaMemory *mem; };
struct DMAControllerWrapper { DMAController *dmaController; };
struct DeniseWrapper { Denise *denise; };
struct PaulaWrapper { Paula *paula; };
struct AmigaKeyboardWrapper { AmigaKeyboard *keyboard; };
struct DiskControllerWrapper { DiskController *controller; };
struct AmigaDriveWrapper { AmigaDrive *drive; };
struct AmigaFileWrapper { AmigaFile *file; };
struct ADFFileWrapper { ADFFile *adf; };


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
// Paula proxy
//

@implementation PaulaProxy

- (instancetype) initWithPaula:(Paula *)paula
{
    if (self = [super init]) {
        wrapper = new PaulaWrapper();
        wrapper->paula = paula;
    }
    return self;
}
- (void) dump
{
    wrapper->paula->dump();
}
- (NSInteger) volume
{
    return wrapper->paula->getVolume();
}
- (NSInteger) bufferUnderflows
{
    return wrapper->paula->bufferUnderflows();
}
- (NSInteger) bufferOverflows
{
    return wrapper->paula->bufferOverflows();
}
- (double) fillLevel
{
    return wrapper->paula->fillLevel();
}

@end


//
// Keyboard proxy
//

@implementation AmigaKeyboardProxy

- (instancetype) initWithKeyboard:(AmigaKeyboard *)keyboard
{
    if (self = [super init]) {
        wrapper = new AmigaKeyboardWrapper();
        wrapper->keyboard = keyboard;
    }
    return self;
}
- (void) dump
{
    wrapper->keyboard->dump();
}
- (BOOL) keyIsPressed:(NSInteger)keycode
{
    return wrapper->keyboard->keyIsPressed(keycode);
}
- (void) pressKey:(NSInteger)keycode
{
    wrapper->keyboard->pressKey(keycode);
}
- (void) releaseKey:(NSInteger)keycode
{
    wrapper->keyboard->releaseKey(keycode);
}
- (void) releaseAllKeys
{
    wrapper->keyboard->releaseAllKeys();
}

@end


//
// DiskController proxy
//

@implementation DiskControllerProxy

- (instancetype) initWithDiskController:(DiskController *)controller
{
    if (self = [super init]) {
        wrapper = new DiskControllerWrapper();
        wrapper->controller = controller;
    }
    return self;
}
- (void) dump
{
    wrapper->controller->dump();
}
- (BOOL) doesDMA:(NSInteger)nr
{
    return wrapper->controller->doesDMA((unsigned)nr);
}

@end


//
// AmigaDrive proxy
//

@implementation AmigaDriveProxy

- (instancetype) initWithDrive:(AmigaDrive *)drive
{
    if (self = [super init]) {
        wrapper = new AmigaDriveWrapper();
        wrapper->drive = drive;
    }
    return self;
}
- (NSInteger) nr
{
    return wrapper->drive->getNr();
}
- (void) dump
{
    wrapper->drive->dump();
}
- (BOOL) isConnected
{
    return wrapper->drive->isConnected();
}
- (void) setConnected:(BOOL)value
{
    wrapper->drive->setConnected(value);
}
- (void) toggleConnected
{
    wrapper->drive->toggleConnected();
}
- (BOOL) hasDisk
{
    return wrapper->drive->hasDisk();
}
- (BOOL) hasWriteProtectedDisk
{
    return wrapper->drive->hasWriteProtectedDisk();
}
- (BOOL) hasModifiedDisk
{
    return wrapper->drive->hasModifiedDisk();
}
- (void) setModifiedDisk:(BOOL)value
{
    wrapper->drive->setModifiedDisk(value);
}
- (void) ejectDisk
{
    wrapper->drive->ejectDisk();
}
- (void) insertDisk:(ADFFileProxy *)fileProxy
{
    AmigaFileWrapper *fileWrapper = [fileProxy wrapper];
    wrapper->drive->insertDisk((ADFFile *)(fileWrapper->file));
}
- (void) toggleWriteProtection
{
    wrapper->drive->toggleWriteProtection();
}

@end


//
// AmigaFile proxy
//

@implementation AmigaFileProxy

- (instancetype) initWithFile:(AmigaFile *)file
{
    if (file == nil) {
        return nil;
    }
    if (self = [super init]) {
        wrapper = new AmigaFileWrapper();
        wrapper->file = file;
    }
    return self;
}
+ (AmigaFileProxy *) makeWithFile:(AmigaFile *)file
{
    if (file == nil) {
        return nil;
    }
    return [[self alloc] initWithFile:file];
}
- (void)setPath:(NSString *)path
{
    AmigaFile *file = (AmigaFile *)([self wrapper]->file);
    file->setPath([path UTF8String]);
}
- (AmigaFileWrapper *)wrapper
{
    return wrapper;
}
- (AmigaFileType)type
{
    return wrapper->file->type();
}
- (NSInteger) sizeOnDisk
{
    return wrapper->file->sizeOnDisk();
}
- (void) seek:(NSInteger)offset
{
    wrapper->file->seek(offset);
}
- (NSInteger)read
{
    return wrapper->file->read();
}
- (void) readFromBuffer:(const void *)buffer length:(NSInteger)length
{
    wrapper->file->readFromBuffer((const uint8_t *)buffer, length);
}
- (NSInteger) writeToBuffer:(void *)buffer
{
    return wrapper->file->writeToBuffer((uint8_t *)buffer);
}

- (void) dealloc
{
    // NSLog(@"AmigaFileProxy::dealloc");
    
    if (wrapper) {
        if (wrapper->file) delete wrapper->file;
        delete wrapper;
    }
}

@end


//
// Snapshot proxy
//

@implementation AmigaSnapshotProxy

+ (BOOL) isSupportedSnapshot:(const void *)buffer length:(NSInteger)length
{
    return AmigaSnapshot::isSupportedSnapshot((uint8_t *)buffer, length);
}
+ (BOOL) isUnsupportedSnapshot:(const void *)buffer length:(NSInteger)length
{
    return AmigaSnapshot::isUnsupportedSnapshot((uint8_t *)buffer, length);
}
+ (BOOL) isSupportedSnapshotFile:(NSString *)path
{
    return AmigaSnapshot::isSupportedSnapshotFile([path UTF8String]);
}
+ (BOOL) isUnsupportedSnapshotFile:(NSString *)path
{
    return AmigaSnapshot::isUnsupportedSnapshotFile([path UTF8String]);
}

+ (instancetype) make:(AmigaSnapshot *)snapshot
{
    if (snapshot == NULL) {
        return nil;
    }
    return [[self alloc] initWithFile:snapshot];
}
+ (instancetype) makeWithBuffer:(const void *)buffer length:(NSInteger)length
{
    AmigaSnapshot *snapshot = AmigaSnapshot::makeWithBuffer((uint8_t *)buffer, length);
    return [self make:snapshot];
}
+ (instancetype) makeWithFile:(NSString *)path
{
    AmigaSnapshot *snapshot = AmigaSnapshot::makeWithFile([path UTF8String]);
    return [self make:snapshot];
}
+ (instancetype) makeWithAmiga:(AmigaProxy *)proxy
{
    Amiga *amiga = [proxy wrapper]->amiga;
    amiga->suspend();
    AmigaSnapshot *snapshot = AmigaSnapshot::makeWithAmiga(amiga);
    amiga->resume();
    return [self make:snapshot];
}

@end


//
// ADFFile proxy
//

@implementation ADFFileProxy

+ (BOOL)isADFFile:(NSString *)path
{
    return ADFFile::isADFFile([path UTF8String]);
}
+ (instancetype) make:(ADFFile *)archive
{
    if (archive == NULL) return nil;
    return [[self alloc] initWithFile:archive];
}
+ (instancetype) makeWithBuffer:(const void *)buffer length:(NSInteger)length
{
    ADFFile *archive = ADFFile::makeWithBuffer((const uint8_t *)buffer, length);
    return [self make: archive];
}
+ (instancetype) makeWithFile:(NSString *)path
{
    ADFFile *archive = ADFFile::makeWithFile([path UTF8String]);
    return [self make: archive];
}
+ (instancetype) make
{
    ADFFile *archive = ADFFile::make();
    return [self make: archive];
}
- (void)seekTrack:(NSInteger)nr
{
    ((ADFFile *)wrapper->file)->seekTrack(nr);
}
- (void)seekSector:(NSInteger)nr
{
    ((ADFFile *)wrapper->file)->seekSector(nr);
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
@synthesize paula;
@synthesize keyboard;
@synthesize diskController;
@synthesize df0;
@synthesize df1;

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
    paula = [[PaulaProxy alloc] initWithPaula:&amiga->paula];
    keyboard = [[AmigaKeyboardProxy alloc] initWithKeyboard:&amiga->keyboard];
    diskController = [[DiskControllerProxy alloc] initWithDiskController:&amiga->diskController];
    df0 = [[AmigaDriveProxy alloc] initWithDrive:&amiga->df0];
    df1 = [[AmigaDriveProxy alloc] initWithDrive:&amiga->df1];
    
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

- (uint64_t) masterClock
{
    return wrapper->amiga->masterClock;
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
- (BOOL) configureLayout:(NSInteger)value
{
    return wrapper->amiga->configureLayout(value);
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
- (BOOL) takeAutoSnapshots
{
    return wrapper->amiga->getTakeAutoSnapshots();
}
- (void) setTakeAutoSnapshots:(BOOL)b
{
    wrapper->amiga->setTakeAutoSnapshots(b);
}
- (void) suspendAutoSnapshots
{
    wrapper->amiga->suspendAutoSnapshots();
}
- (void) resumeAutoSnapshots
{
    wrapper->amiga->resumeAutoSnapshots();
}
- (NSInteger) snapshotInterval
{
    return wrapper->amiga->getSnapshotInterval();
}
- (void) loadFromSnapshot:(AmigaSnapshotProxy *)proxy
{
    AmigaSnapshot *snapshot = (AmigaSnapshot *)([proxy wrapper]->file);
    wrapper->amiga->loadFromSnapshotSafe(snapshot);
}
- (void) setSnapshotInterval:(NSInteger)value
{
    wrapper->amiga->setSnapshotInterval(value);
}
- (BOOL)restoreAutoSnapshot:(NSInteger)nr
{
    return wrapper->amiga->restoreAutoSnapshot((unsigned)nr);
}
- (BOOL)restoreUserSnapshot:(NSInteger)nr
{
    return wrapper->amiga->restoreUserSnapshot((unsigned)nr);
}
- (BOOL)restoreLatestUserSnapshot
{
    return wrapper->amiga->restoreLatestUserSnapshot();
}
- (BOOL)restoreLatestAutoSnapshot
{
    return wrapper->amiga->restoreLatestAutoSnapshot();
}
- (NSInteger) numAutoSnapshots
{
    return wrapper->amiga->numAutoSnapshots();
}
- (NSInteger) numUserSnapshots
{
    return wrapper->amiga->numUserSnapshots();
}
- (NSData *)autoSnapshotData:(NSInteger)nr {
    AmigaSnapshot *snapshot = wrapper->amiga->autoSnapshot((unsigned)nr);
    return [NSData dataWithBytes: (void *)snapshot->getHeader()
                          length: snapshot->sizeOnDisk()];
}
- (NSData *)userSnapshotData:(NSInteger)nr {
    AmigaSnapshot *snapshot = wrapper->amiga->userSnapshot((unsigned)nr);
    return [NSData dataWithBytes: (void *)snapshot->getHeader()
                          length: snapshot->sizeOnDisk()];
}
- (unsigned char *)autoSnapshotImageData:(NSInteger)nr
{
    AmigaSnapshot *s = wrapper->amiga->autoSnapshot((int)nr);
    return s ? s->getImageData() : NULL;
}
- (unsigned char *)userSnapshotImageData:(NSInteger)nr
{
    AmigaSnapshot *s = wrapper->amiga->userSnapshot((int)nr);
    return s ? s->getImageData() : NULL;
}
- (NSSize) autoSnapshotImageSize:(NSInteger)nr {
    AmigaSnapshot *s = wrapper->amiga->autoSnapshot((int)nr);
    return s ? NSMakeSize(s->getImageWidth(), s->getImageHeight()) : NSMakeSize(0,0);
}
- (NSSize) userSnapshotImageSize:(NSInteger)nr {
    AmigaSnapshot *s = wrapper->amiga->userSnapshot((int)nr);
    return s ? NSMakeSize(s->getImageWidth(), s->getImageHeight()) : NSMakeSize(0,0);
}
- (time_t)autoSnapshotTimestamp:(NSInteger)nr {
    AmigaSnapshot *s = wrapper->amiga->autoSnapshot((int)nr);
    return s ? s->getTimestamp() : 0;
}
- (time_t)userSnapshotTimestamp:(NSInteger)nr {
    AmigaSnapshot *s = wrapper->amiga->userSnapshot((int)nr);
    return s ? s->getTimestamp() : 0;
}
- (void)takeUserSnapshot
{
    wrapper->amiga->takeUserSnapshotSafe();
}
- (void)deleteAutoSnapshot:(NSInteger)nr
{
    wrapper->amiga->deleteAutoSnapshot((unsigned)nr);
}
- (void)deleteUserSnapshot:(NSInteger)nr
{
    wrapper->amiga->deleteUserSnapshot((unsigned)nr);
}

@end

