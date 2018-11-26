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
let buffer = UnsafeMutableRawPointer.allocate(byteCount: stride * height, alignment: 4)

// Render that
let context = CIContext(options: nil)
//let cgImage = context.createCGImage(ciImage!, from: extent, format: pixelFormat, colorSpace: nil, deferred: false)
context.render(ciImage!, toBitmap: buffer, rowBytes: stride, bounds: extent, format: pixelFormat, colorSpace: nil)



//let context = CIContext(options: [workingColorSpace])
puts("Fin!\n")

buffer
