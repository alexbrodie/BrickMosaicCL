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
let ciImage = CIImage(contentsOf: URL(fileURLWithPath: imagePath))
if (ciImage == nil) {
    fputs("Couldn't load '\(imagePath)'\n", stderr)
    exit(1)
}

// Define the buffer size and type
let pixelFormat = CIFormat.ARGB8
let extent: CGRect = ciImage!.extent
let width: Int = Int(extent.width)
let height: Int  = Int(extent.height)
let stride: Int = width * 4
let bufferLength: Int = stride * height
let buffer = UnsafeMutableRawPointer.allocate(byteCount: bufferLength, alignment: 4)

// Render that
let context = CIContext(options: nil)
//let cgImage = context.createCGImage(ciImage!, from: extent, format: pixelFormat, colorSpace: nil, deferred: false)
context.render(ciImage!, toBitmap: buffer, rowBytes: stride, bounds: extent, format: pixelFormat, colorSpace: nil)

let engine = makeEngine()

process(engine, buffer, Int32(width), Int32(height), Int32(stride))

freeEngine(engine)

let colorSpace: CGColorSpace = CGColorSpace(name: CGColorSpace.sRGB)!

let ciImageAfter: CIImage = CIImage(bitmapData: Data(bytesNoCopy: buffer, count: bufferLength, deallocator: Data.Deallocator.none),
                                    bytesPerRow: stride, size: CGSize(width: width, height: height),
                                    format: pixelFormat, colorSpace: colorSpace)

try context.writePNGRepresentation(of: ciImageAfter, to: URL(fileURLWithPath: imagePath + "~output.png"),
                                   format: CIFormat.ARGB8, colorSpace: colorSpace)

puts("Fin!\n")
