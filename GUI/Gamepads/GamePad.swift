// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import IOKit.hid

//
// A GamePad object is created for each connected USB device
// Creation and destruction is done by the GamePadManager
//

class GamePad {

    // Keymap of the managed device (only used for keyboard emulated devices)
    var keyMap: [MacKey: UInt32]?
    
    // Indicates if a joystick emulation key is currently pressed
    var keyUp = false, keyDown = false, keyLeft = false, keyRight = false
    
    // Name of the connected controller
    var name: String?

    // Vendor ID of the managed device (only used for HID devices)
    var vendorID: Int

    // Product ID of the managed device (only used for HID devices)
    var productID: Int

    // Location ID of the managed device (only used for HID devices)
    var locationID: Int

    // Minimum value of analog axis event
    var min: Int?
    
    // Maximum value of analog axis event
    var max: Int?
    
    /* Rescued information from the last invocation of the action function
     * Used to determine if a joystick event needs to be triggered.
     */
    var oldEvents: [Int: [GamePadAction]] = [:]
    
    // Cotroller dependent usage IDs for left and right gamepad joysticks
    var lThumbXUsageID = kHIDUsage_GD_X
    var lThumbYUsageID = kHIDUsage_GD_Y
    var rThumbXUsageID = kHIDUsage_GD_Rz
    var rThumbYUsageID = kHIDUsage_GD_Z

    // Reference to the GamePadManager
    var manager: GamePadManager
    
    init(manager: GamePadManager,
         vendorID: Int, productID: Int, locationID: Int) {
        
        track()
        
        self.manager = manager
        self.vendorID = vendorID
        self.productID = productID
        self.locationID = locationID
    
        // Check for known devices
        if vendorID == 0x40B && productID == 0x6533 {
            
            name = "Competition Pro SL-6602"
        
        } else if vendorID == 0xF0D && productID == 0xC1 {

            name = "iNNEXT Retro (N64)"

        } else if vendorID == 0x79 && productID == 0x11 {

            name = "iNNEXT Retro (SNES)"

        } else if vendorID == 0x54C && productID == 0x268 {

            name = "Sony DualShock 3"
            rThumbXUsageID = kHIDUsage_GD_Z
            rThumbYUsageID = kHIDUsage_GD_Rz
        
        } else if vendorID == 0x54C && productID == 0x5C4 {
            
            name = "Sony DualShock 4"
            rThumbXUsageID = kHIDUsage_GD_Z
            rThumbYUsageID = kHIDUsage_GD_Rz

        } else if vendorID == 0x54C && productID == 0x9CC {
            
            name = "Sony Dualshock 4 (2nd Gen)"
            rThumbXUsageID = kHIDUsage_GD_Z
            rThumbYUsageID = kHIDUsage_GD_Rz
        
        } else if vendorID == 0x483 && productID == 0x9005 {
            
            name = "RetroFun! Joystick Adapter"

        } else if vendorID == 0x004 && productID == 0x0001 {
            
            name = "aJoy Retro Adapter"
            
        } else {
        
            // name = "Generic Gamepad"
        }
    }
    
    convenience init(manager: GamePadManager) {
        self.init(manager: manager, vendorID: 0, productID: 0, locationID: 0)
    }
    
    let actionCallback: IOHIDValueCallback = { inContext, inResult, inSender, value in
        let this: GamePad = unsafeBitCast(inContext, to: GamePad.self)
        this.hidDeviceAction(context: inContext, result: inResult, sender: inSender, value: value)
    }
}

//
// Keyboard emulation
//

extension GamePad {

    // Binds a key to a gamepad action
    func bind(key: MacKey, action: GamePadAction) {

        // Avoid double mappings
        unbind(action: action)

        keyMap![key] = action.rawValue
    }

    // Removes any existing key binding to the specified gampad action
     func unbind(action: GamePadAction) {

         for (k, dir) in keyMap! where dir == action.rawValue {
             keyMap![k] = nil
         }
     }

    /* Handles a keyboard down event
     * Checks if the provided keycode matches a joystick emulation key and
     * triggeres an event if a match has been found.
     * Returns true if a joystick event has been triggered.
     */
    func keyDown(_ macKey: MacKey) -> Bool {
        
        if let direction = keyMap?[macKey] {

            var events: [GamePadAction]
            
            switch GamePadAction(direction) {
                
            case PULL_UP:
                keyUp = true
                events = [PULL_UP]
                
            case PULL_DOWN:
                keyDown = true
                events = [PULL_DOWN]
                
            case PULL_LEFT:
                keyLeft = true
                events = [PULL_LEFT]
                
            case PULL_RIGHT:
                keyRight = true
                events = [PULL_RIGHT]
                
            case PRESS_FIRE:
                events = [PRESS_FIRE]

            case PRESS_LEFT:
                events = [PRESS_LEFT]

            case PRESS_RIGHT:
                events = [PRESS_RIGHT]

            default:
                fatalError()
            }
            
            return manager.joystickAction(self, events: events)
        }
        
        return false
    }
    
    /* Handles a keyboard up event
     * Checks if the provided keycode matches a joystick emulation key
     * and triggeres an event if a match has been found.
     * Returns true if a joystick event has been triggered.
     */
    func keyUp(_ macKey: MacKey) -> Bool {

        if let direction = keyMap?[macKey] {
            
            var events: [GamePadAction]
            
            switch GamePadAction(direction) {
            
            case PULL_UP:
                keyUp = false
                events = keyDown ? [PULL_DOWN] : [RELEASE_Y]
                
            case PULL_DOWN:
                keyDown = false
                events = keyUp ? [PULL_UP] : [RELEASE_Y]
                
            case PULL_LEFT:
                keyLeft = false
                events = keyRight ? [PULL_RIGHT] : [RELEASE_X]
                
            case PULL_RIGHT:
                keyRight = false
                events = keyLeft ? [PULL_LEFT] : [RELEASE_X]
                
            case PRESS_FIRE:
                events = [RELEASE_FIRE]

            case PRESS_LEFT:
                events = [RELEASE_LEFT]

            case PRESS_RIGHT:
                events = [RELEASE_RIGHT]

            default:
                fatalError()
            }
            
            return manager.joystickAction(self, events: events)
        }
    
        return false
    }
}

//
// Event handling
//

extension GamePad {

    // Based on http://docs.ros.org/hydro/api/oculus_sdk/html/OSX__Gamepad_8cpp_source.html#l00170
    func mapAnalogAxis(value: IOHIDValue, element: IOHIDElement) -> Int? {
        
        if min == nil {
            min = IOHIDElementGetLogicalMin(element)
            track("Minumum axis value = \(min!)")
        }
        if max == nil {
            max = IOHIDElementGetLogicalMax(element)
            track("Maximum axis value = \(max!)")
        }
        let val = IOHIDValueGetIntegerValue(value)
        
        var v = (Double) (val - min!) / (Double) (max! - min!)
        v = v * 2.0 - 1.0
        if v < -0.45 { return -2 }
        if v < -0.1 { return nil }  // dead zone
        if v <= 0.1 { return 0 }
        if v <= 0.45 { return nil } // dead zone
        return 2
    }

    func hidDeviceAction(context: UnsafeMutableRawPointer?,
                         result: IOReturn,
                         sender: UnsafeMutableRawPointer?,
                         value: IOHIDValue) {
    
        let element   = IOHIDValueGetElement(value)
        let intValue  = Int(IOHIDValueGetIntegerValue(value))
        let usagePage = Int(IOHIDElementGetUsagePage(element))
        let usage     = Int(IOHIDElementGetUsage(element))
        
        // Buttons
        if usagePage == kHIDPage_Button {
            track("BUTTON")
            manager.joystickAction(self, events: (intValue != 0) ? [PRESS_FIRE] : [RELEASE_FIRE])
            return
        }
        
        // Stick
        if usagePage == kHIDPage_GenericDesktop {
            
            var events: [GamePadAction]?
            
            switch usage {
                
            case lThumbXUsageID, rThumbXUsageID:
                
                track("lThumbXUsageID, rThumbXUsageID: \(intValue)")
                if let v = mapAnalogAxis(value: value, element: element) {
                    events = (v == 2) ? [PULL_RIGHT] : (v == -2) ? [PULL_LEFT] : [RELEASE_X]
                }
   
            case lThumbYUsageID, rThumbYUsageID:
                
                track("lThumbYUsageID, rThumbYUsageID: \(intValue)")
                if let v = mapAnalogAxis(value: value, element: element) {
                    events = (v == 2) ? [PULL_DOWN] : (v == -2) ? [PULL_UP] : [RELEASE_Y]
                }
                
            case kHIDUsage_GD_Hatswitch:
                
                track("kHIDUsage_GD_Hatswitch \(intValue)")
                switch intValue {
                case 0: events = [PULL_UP, RELEASE_X]
                case 1: events = [PULL_UP, PULL_RIGHT]
                case 2: events = [PULL_RIGHT, RELEASE_Y]
                case 3: events = [PULL_RIGHT, PULL_DOWN]
                case 4: events = [PULL_DOWN, RELEASE_X]
                case 5: events = [PULL_DOWN, PULL_LEFT]
                case 6: events = [PULL_LEFT, RELEASE_Y]
                case 7: events = [PULL_LEFT, PULL_UP]
                default: events = [RELEASE_XY]
                // default: break
                }
                
            default:
                // track("Unknown HID usage: \(usage)")")
                break
            }
            
            // Only proceed if the event is different than the previous one
            if events == nil || oldEvents[usage] == events {
                return
            } else {
                oldEvents[usage] = events!
            }
            
            // Trigger event
            manager.joystickAction(self, events: events!)
        }
    }
}
