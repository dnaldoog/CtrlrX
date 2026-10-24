/*
  ==============================================================================

    Ctrlricongenerator.h
    Created: 18 Sep 2026 10:58:59am
    Author:  zan64

  ==============================================================================
*/

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Generates a multi-resolution Windows .ico file from an SVG source file.
//
// Each requested size is rasterized via JUCE's Drawable SVG support and
// encoded as a standalone PNG, then embedded directly as a PNG-format icon
// entry in the .ico — supported natively since Windows Vista, so this
// avoids hand-assembling raw BITMAPINFOHEADER + XOR/AND mask data, which is
// the fiddlier, easier-to-get-wrong part of the classic .ico format.
//
// The resulting file is a real, standalone .ico — not a PE resource. It's
// meant to be handed to rcedit (or any other icon-embedding tool) as input;
// this function has no knowledge of PE resources at all.
class CtrlrIconGenerator {
public:
	static juce::Result generateIcoFromSvg(const juce::File &svgFile, const juce::File &icoDestFile,
										   const juce::Array<int> &sizes = {16, 32, 48, 256}) {
		auto drawable = juce::Drawable::createFromSVGFile(svgFile);

		if (drawable == nullptr)
			return juce::Result::fail("CtrlrIconGenerator: failed to parse SVG: " + svgFile.getFullPathName());

		struct IconEntry {
				int size;
				juce::MemoryBlock pngData;
		};

		juce::Array<IconEntry> entries;
		juce::PNGImageFormat pngFormat;

		for (int size : sizes) {
			if (size <= 0 || size > 256) {
				return juce::Result::fail("CtrlrIconGenerator: invalid icon size " + juce::String(size) +
										  " (must be 1-256)");
			}

			//juce::Image image(juce::Image::ARGB, size, size, true);
			juce::Image image(juce::Image::ARGB, size, size, true, juce::SoftwareImageType());
			juce::Graphics g(image);

			drawable->drawWithin(g, juce::Rectangle<float>(0, 0, (float)size, (float)size),
								 juce::RectanglePlacement::centred, 1.0f);

			if (!image.isValid()) {
				return juce::Result::fail("CtrlrIconGenerator: failed to rasterize icon at size " +
										  juce::String(size));
			}

			juce::MemoryOutputStream pngStream;

			if (!pngFormat.writeImageToStream(image, pngStream)) {
				return juce::Result::fail("CtrlrIconGenerator: failed to encode PNG at size " + juce::String(size));
			}

			IconEntry entry;
			entry.size = size;
			entry.pngData.setSize(pngStream.getDataSize());
			entry.pngData.copyFrom(pngStream.getData(), 0, pngStream.getDataSize());
			entries.add(entry);
		}

		if (entries.isEmpty())
			return juce::Result::fail("CtrlrIconGenerator: no icon sizes were generated");

		// --- Assemble the .ico file ---
		//
		// ICONDIR header (6 bytes):
		//   WORD reserved   (0)
		//   WORD type       (1 = icon)
		//   WORD count      (number of entries)
		//
		// Then `count` ICONDIRENTRY structs (16 bytes each):
		//   BYTE  width      (0 means 256)
		//   BYTE  height     (0 means 256)
		//   BYTE  colorCount (0 for >=8bpp images)
		//   BYTE  reserved   (0)
		//   WORD  planes     (1)
		//   WORD  bitCount   (32)
		//   DWORD bytesInRes (size of this entry's image data)
		//   DWORD imageOffset (absolute file offset to this entry's image data)
		//
		// Then the raw image data for each entry, in the same order — here,
		// each one a complete, standalone PNG file's bytes.
		//
		// JUCE's OutputStream::writeShort/writeInt are documented as
		// little-endian, matching what this format requires — no manual
		// byte-swapping needed.

		juce::MemoryOutputStream out;

		out.writeShort((short)0);				  // reserved
		out.writeShort((short)1);				  // type = icon
		out.writeShort((short)entries.size());   // count

		uint32_t dataOffset = (uint32_t)(6 + entries.size() * 16);

		for (auto &entry : entries) {
			uint8_t dim = (entry.size >= 256) ? 0 : (uint8_t)entry.size;

			out.writeByte((char)dim);						  // width
			out.writeByte((char)dim);						  // height
			out.writeByte((char)0);						  // colour count
			out.writeByte((char)0);						  // reserved
			out.writeShort((short)1);						  // planes
			out.writeShort((short)32);						  // bit count
			out.writeInt((int)entry.pngData.getSize());	  // bytes in resource
			out.writeInt((int)dataOffset);					  // image offset

			dataOffset += (uint32_t)entry.pngData.getSize();
		}

		for (auto &entry : entries)
			out.write(entry.pngData.getData(), entry.pngData.getSize());

		if (!icoDestFile.replaceWithData(out.getData(), out.getDataSize())) {
			return juce::Result::fail("CtrlrIconGenerator: failed to write .ico file: " +
									  icoDestFile.getFullPathName());
		}

		return juce::Result::ok();
	}
};
