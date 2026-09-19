#pragma once
#include "stdafx.h"
#include <juce_gui_basics/juce_gui_basics.h>

// Generates a macOS .icns icon file from an SVG source file.
//
// Each requested size is rasterized via JUCE's Drawable SVG support and
// encoded as a standalone PNG, then embedded directly as a PNG-format icon
// element — supported by macOS since roughly 10.7, well within anything
// this project targets.
//
// Unlike the Windows path (which needs rcedit to inject an already-built
// .ico into a compiled .exe's PE resources), an .icns file needs no
// external tool at all — it's written straight into an already-writable
// location, Contents/Resources/, and referenced by name from Info.plist's
// CFBundleIconFile key. exportWithDefaultPanel already rewrites Info.plist
// (setBundleInfo/setBundleInfoCarbon) and already writes files into
// Contents/Resources/ for other purposes, so wiring this in reuses both
// mechanisms rather than adding a new one.
//
// TWO FORMAT DIFFERENCES FROM THE WINDOWS .ico GENERATOR, WORTH NOTING
// EXPLICITLY SINCE THEY'RE EASY TO LOSE IF THIS CODE IS EVER REUSED:
//   1. .icns is BIG-ENDIAN throughout. .ico is little-endian.
//   2. .icns identifies each size by a 4-character OSType tag (e.g. "ic08"
//      for 256x256), not a byte-encoded width/height pair — only a fixed
//      set of sizes have a defined tag at all (see tagForSize() below).
class CtrlrIcnsGenerator {
public:
	static juce::Result generateIcnsFromSvg(const juce::File &svgFile, const juce::File &icnsDestFile,
											const juce::Array<int> &sizes = {16, 32, 64, 128, 256, 512}) {
		auto drawable = juce::Drawable::createFromSVGFile(svgFile);

		if (drawable == nullptr)
			return juce::Result::fail("CtrlrIcnsGenerator: failed to parse SVG: " + svgFile.getFullPathName());

		struct IconEntry {
				juce::String tag;
				juce::MemoryBlock pngData;
		};

		juce::Array<IconEntry> entries;
		juce::PNGImageFormat pngFormat;

		for (int size : sizes) {
			juce::String tag = tagForSize(size);

			if (tag.isEmpty()) {
				return juce::Result::fail("CtrlrIcnsGenerator: unsupported icon size " + juce::String(size) +
										  " (supported: 16, 32, 64, 128, 256, 512, 1024)");
			}

			// SoftwareImageType() forced here from the start — the Windows
			// .ico generator needed this added after hitting repeated
			// Direct2D assertions on off-screen (never-shown-on-a-window)
			// image creation. Not confirmed necessary on macOS, but there's
			// no reason to risk the same class of bug here when it costs
			// nothing to avoid up front.
			juce::Image image(juce::Image::ARGB, size, size, true, juce::SoftwareImageType());
			juce::Graphics g(image);

			drawable->drawWithin(g, juce::Rectangle<float>(0, 0, (float)size, (float)size),
								 juce::RectanglePlacement::centred, 1.0f);

			if (!image.isValid()) {
				return juce::Result::fail("CtrlrIcnsGenerator: failed to rasterize icon at size " +
										  juce::String(size));
			}

			juce::MemoryOutputStream pngStream;

			if (!pngFormat.writeImageToStream(image, pngStream)) {
				return juce::Result::fail("CtrlrIcnsGenerator: failed to encode PNG at size " + juce::String(size));
			}

			IconEntry entry;
			entry.tag = tag;
			entry.pngData.setSize(pngStream.getDataSize());
			entry.pngData.copyFrom(pngStream.getData(), 0, pngStream.getDataSize());
			entries.add(entry);
		}

		if (entries.isEmpty())
			return juce::Result::fail("CtrlrIcnsGenerator: no icon sizes were generated");

		// --- Assemble the .icns file ---
		//
		// File header (8 bytes):
		//   4 bytes  magic "icns"
		//   4 bytes  uint32 (big-endian) total file length, including this header
		//
		// Then, for each icon element:
		//   4 bytes  OSType tag (e.g. "ic08"), ASCII, not null-terminated
		//   4 bytes  uint32 (big-endian) element length, INCLUDING this 8-byte header
		//   N bytes  the raw PNG file's bytes

		uint32_t totalLength = 8; // file header
		for (auto &entry : entries)
			totalLength += 8 + (uint32_t)entry.pngData.getSize();

		juce::MemoryOutputStream out;

		writeFourCC(out, "icns");
		writeUInt32BE(out, totalLength);

		for (auto &entry : entries) {
			uint32_t elementLength = 8 + (uint32_t)entry.pngData.getSize();

			writeFourCC(out, entry.tag);
			writeUInt32BE(out, elementLength);
			out.write(entry.pngData.getData(), entry.pngData.getSize());
		}

		if (!icnsDestFile.replaceWithData(out.getData(), out.getDataSize())) {
			return juce::Result::fail("CtrlrIcnsGenerator: failed to write .icns file: " +
									  icnsDestFile.getFullPathName());
		}

		return juce::Result::ok();
	}

private:
	// Only sizes with a defined OSType tag are supported — .icns doesn't
	// accept arbitrary dimensions the way .ico's byte-encoded width/height
	// does. This covers the standard 1x set; the @2x Retina tags (ic11-
	// ic14) exist too but aren't included here to keep the default output
	// simple — easy to extend later if crisper Retina rendering matters.
	static juce::String tagForSize(int size) {
		switch (size) {
		case 16:
			return "icp4";
		case 32:
			return "icp5";
		case 64:
			return "icp6";
		case 128:
			return "ic07";
		case 256:
			return "ic08";
		case 512:
			return "ic09";
		case 1024:
			return "ic10";
		default:
			return {};
		}
	}

	static void writeFourCC(juce::MemoryOutputStream &out, const juce::String &fourCC) {
		jassert(fourCC.length() == 4);
		out.write(fourCC.toRawUTF8(), 4);
	}

	static void writeUInt32BE(juce::MemoryOutputStream &out, uint32_t value) {
		out.writeByte((char)((value >> 24) & 0xFF));
		out.writeByte((char)((value >> 16) & 0xFF));
		out.writeByte((char)((value >> 8) & 0xFF));
		out.writeByte((char)(value & 0xFF));
	}
};