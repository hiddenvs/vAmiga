// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/*
import Metal
import MetalKit
import MetalPerformanceShaders
*/

public class MetalView: MTKView {
    
    @IBOutlet weak var controller: MyController!

    /* Tracking area for trapping the mouse
     * We use a tracking area to determine when the mouse is moved into or
     * out of the emulator window.
     */
    var trackingArea: NSTrackingArea?
    
    // Indicates whether the mouse is currently within the tracking area.
    var insideTrackingArea = false
    
    // Shows whether the Amiga is currently in control of the mouse.
    var gotMouse = false

    // The selected method for retaining and releasing the mouse
    var retainMouseKeyComb = 0
    var retainMouseWithKeys = true
    var retainMouseByClick = true
    var retainMouseByEntering = false
    var releaseMouseKeyComb = 0
    var releaseMouseWithKeys = true
    var releaseMouseByShaking = true
    
    // Variables needed to detect a mouse detection
    var dxsign = CGFloat(1)
    var dxsum = CGFloat(0)
    var dxturns = 0
    var lastTurn = DispatchTime.init(uptimeNanoseconds: 0)
    var lastShake = DispatchTime.init(uptimeNanoseconds: 0)

    required public init(coder: NSCoder) {
    
        super.init(coder: coder)
    }
    
    required public override init(frame frameRect: CGRect, device: MTLDevice?) {
        
        super.init(frame: frameRect, device: device)
    }
    
    override open func awakeFromNib() {

        track()
        setupDragAndDrop()
    }
    
    override public var acceptsFirstResponder: Bool { return true }
    
    // Adjusts view height by a certain number of pixels
    func adjustHeight(_ height: CGFloat) {
    
        var newFrame = frame
        newFrame.origin.y -= height
        newFrame.size.height += height
        frame = newFrame
    }
    
    // Shrinks view vertically by the height of the status bar
    public func shrink() { adjustHeight(-24.0) }
    
    // Expand view vertically by the height of the status bar
    public func expand() { adjustHeight(24.0) }
}
