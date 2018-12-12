//
//  main.swift
//  BrickMosaicCL
//
//  Created by Alexander Brodie on 11/25/18.
//  Copyright © 2018 Alexander Brodie. All rights reserved.
//

import Foundation
import CoreImage

if (CommandLine.argc != 2) {
    let app = URL(fileURLWithPath: CommandLine.arguments[0])
    fputs("Usage: \(app.lastPathComponent) filename\n", stderr)
    exit(1)
}

// Load CIImage
let imagePath = CommandLine.arguments[1]
var ciImage = CIImage(contentsOf: URL(fileURLWithPath: imagePath))
if (ciImage == nil) {
    fputs("Couldn't load '\(imagePath)'\n", stderr)
    exit(1)
}

var extent: CGRect = ciImage!.extent
var width: Int = Int(extent.width)
var height: Int  = Int(extent.height)

// Constrain to a max dimension
// 100x100 LEGO studs is roughly $60 and 2.6 x 2.6 ft.
let maxDim: Int = 100
let scale = CGFloat(maxDim) / CGFloat(max(width, height))
//ciImage = ciImage!.transformed(by: CGAffineTransform(scaleX: scale, y: scale))
let filter = CIFilter(name: "CILanczosScaleTransform")!
filter.setValue(ciImage, forKey: "inputImage")
filter.setValue(scale, forKey: "inputScale")
filter.setValue(1.0, forKey: "inputAspectRatio")
ciImage = filter.value(forKey: "outputImage") as? CIImage

extent = ciImage!.extent
width = Int(extent.width)
height  = Int(extent.height)

let colorSpace: CGColorSpace = CGColorSpace(name: CGColorSpace.sRGB)!
let context = CIContext(options: nil)

try context.writePNGRepresentation(of: ciImage!, to: URL(fileURLWithPath: imagePath + "~scaled.png"),
                                   format: CIFormat.ARGB8, colorSpace: colorSpace)

// Define the buffer size and type
let pixelFormat = CIFormat.ARGB8
let stride: Int = width * 4
let bufferLength: Int = stride * height
let buffer = UnsafeMutableRawPointer.allocate(byteCount: bufferLength, alignment: 4)

// Render that
//let cgImage = context.createCGImage(ciImage!, from: extent, format: pixelFormat, colorSpace: nil, deferred: false)
context.render(ciImage!, toBitmap: buffer, rowBytes: stride, bounds: extent, format: pixelFormat, colorSpace: nil)

let engine = makeEngine()

setDitherColorSpace(engine, "RGB")
setDitherType(engine, "FS")

process(engine, buffer, Int32(width), Int32(height), Int32(stride))

freeEngine(engine)


ciImage = CIImage(bitmapData: Data(bytesNoCopy: buffer, count: bufferLength, deallocator: Data.Deallocator.none),
                                    bytesPerRow: stride, size: CGSize(width: width, height: height),
                                    format: pixelFormat, colorSpace: colorSpace)

try context.writePNGRepresentation(of: ciImage!, to: URL(fileURLWithPath: imagePath + "~output.png"),
                                   format: CIFormat.ARGB8, colorSpace: colorSpace)

puts("Fin!\n")
