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

func loadImage(path: String) -> CIImage {
    let image = CIImage(contentsOf: URL(fileURLWithPath: path))
    if (image == nil) {
        fputs("Couldn't load '\(path)'\n", stderr)
        exit(1)
    }
    
    return image!
}

func saveImage(image: CIImage, path: String) throws {
    let colorSpace: CGColorSpace = CGColorSpace(name: CGColorSpace.sRGB)!
    let context = CIContext(options: nil)
    
    try context.writePNGRepresentation(
        of: image,
        to: URL(fileURLWithPath: path),
        format: CIFormat.ARGB8,
        colorSpace: colorSpace)
}

// Load CIImage
let imagePath = CommandLine.arguments[1]
var ciImage = loadImage(path: imagePath)

func scaleTo(image: CIImage, maxDim: Int) -> CIImage {
    let extent: CGRect = image.extent
    let width: Int = Int(extent.size.width)
    let height: Int = Int(extent.size.height)
    
    let scale = CGFloat(maxDim) / CGFloat(max(width, height))

    //ciImage = ciImage!.transformed(by: CGAffineTransform(scaleX: scale, y: scale))

    let filter = CIFilter(name: "CILanczosScaleTransform")!
    filter.setValue(ciImage, forKey: "inputImage")
    filter.setValue(scale, forKey: "inputScale")
    filter.setValue(1.0, forKey: "inputAspectRatio")
    return filter.value(forKey: "outputImage") as! CIImage
}

// 100x100 LEGO studs is roughly $60 and 2.6 x 2.6 ft.
ciImage = scaleTo(image: ciImage, maxDim: 100)
try saveImage(image: ciImage, path: imagePath + "~1-scaled.png")

// Sharpen
func sharpen(image: CIImage) -> CIImage {
    let filter = CIFilter(name: "CIUnsharpMask")!
    filter.setValue(ciImage, forKey: "inputImage")
    filter.setValue(1.0, forKey: "inputRadius")
    filter.setValue(0.5, forKey: "inputIntensity")
    return filter.value(forKey: "outputImage") as! CIImage
}

ciImage = sharpen(image: ciImage)
try saveImage(image: ciImage, path: imagePath + "~2-sharpened.png")

let colorSpace: CGColorSpace = CGColorSpace(name: CGColorSpace.sRGB)!
let context = CIContext(options: nil)

// Define the buffer size and type
let extent = ciImage.extent
let width: Int = Int(extent.width)
let height: Int = Int(extent.height)
let pixelFormat = CIFormat.ARGB8
let stride: Int = width * 4
let bufferLength: Int = stride * height
let buffer = UnsafeMutableRawPointer.allocate(byteCount: bufferLength, alignment: 4)

let engine = makeEngine()

// For each permutation...
//let ditherType = "FS"
for ditherColorSpace in ["RGB", "LinearRGB", "YIQ", "XYZ", "LAB", "YCrCb"] {
for ditherType in ["", "FS", "JJN", "Stuki"] {
        
// Render the target
//let cgImage = context.createCGImage(ciImage!, from: extent, format: pixelFormat, colorSpace: nil, deferred: false)
context.render(ciImage, toBitmap: buffer, rowBytes: stride, bounds: extent, format: pixelFormat, colorSpace: nil)

// Dither-process it
setDitherType(engine, ditherType)
setDitherColorSpace(engine, ditherColorSpace)
process(engine, buffer, Int32(width), Int32(height), Int32(stride))

// Create result image
let finalImage = CIImage(bitmapData: Data(bytesNoCopy: buffer, count: bufferLength, deallocator: Data.Deallocator.none),
                                    bytesPerRow: stride, size: CGSize(width: width, height: height),
                                    format: pixelFormat, colorSpace: colorSpace)

// Save that to disk
try context.writePNGRepresentation(of: finalImage, to: URL(fileURLWithPath: imagePath + "~output-\(ditherType)-\(ditherColorSpace).png"),
                                   format: CIFormat.ARGB8, colorSpace: colorSpace)


}
}

freeEngine(engine)

puts("Fin!\n")
