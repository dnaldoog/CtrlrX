/*
  ==============================================================================

    CtrlrWinIconPatcher.h
    Created: 14 Sep 2026 1:05:36pm
    Author:  zan64

  ==============================================================================
*/

#ifndef CTRLR_WIN_ICON_PATCHER_H
#define CTRLR_WIN_ICON_PATCHER_H

#pragma once

#include <JuceHeader.h>

#if JUCE_WINDOWS

#ifndef NOMINMAX
  #define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <vector>

// Enforce explicit 2-byte alignment for ICO file structures
#pragma pack(push, 2)
struct ICONDIR {
	uint16_t idReserved;
	uint16_t idType;
	uint16_t idCount;
};

struct ICONDIRENTRY {
	uint8_t  bWidth;
	uint8_t  bHeight;
	uint8_t  bColorCount;
	uint8_t  bReserved;
	uint16_t wPlanes;
	uint16_t wBitCount;
	uint32_t dwBytesInRes;
	uint32_t dwImageOffset;
};

struct GRPICONDIRENTRY {
	uint8_t  bWidth;
	uint8_t  bHeight;
	uint8_t  bColorCount;
	uint8_t  bReserved;
	uint16_t wPlanes;
	uint16_t wBitCount;
	uint32_t dwBytesInRes;
	uint16_t nID;
};

struct GRPICONDIR {
	uint16_t idReserved;
	uint16_t idType;
	uint16_t idCount;
	GRPICONDIRENTRY idEntries[1];
};
#pragma pack(pop)

namespace CtrlrWinIconPatcher {

/**
 * Converts a JUCE Drawable (from SVG) into PNG memory buffers suitable for Windows Vista+ high-res icons.
 */
static inline juce::MemoryBlock renderSvgToPngBuffer(juce::Drawable &svgDrawable, int size) {
	juce::Image img(juce::Image::ARGB, size, size, true);
	juce::Graphics g(img);

	svgDrawable.drawWithin(g, juce::Rectangle<float>(0, 0, (float)size, (float)size), juce::RectanglePlacement::centred, 1.0f);

	juce::MemoryOutputStream stream;
	juce::PNGImageFormat pngFormat;
	pngFormat.writeImageToStream(img, stream);

	return stream.getMemoryBlock();
}

/**
 * Updates the embedded RT_GROUP_ICON in an exported Windows .exe file with a custom SVG.
 *
 * @param targetExePath Path to the exported .exe file copy.
 * @param svgText SVG string from user's ValueTree property.
 * @return true on success.
 */
static inline bool patchExecutableIcon(const juce::File &targetExePath, const juce::String &svgText) {
	if (!targetExePath.existsAsFile() || svgText.isEmpty())
		return false;

	std::unique_ptr<juce::XmlElement> xmlTag(juce::XmlDocument::parse(svgText));
	if (xmlTag == nullptr)
		return false;

	std::unique_ptr<juce::Drawable> drawable(juce::Drawable::createFromSVG(*xmlTag));
	if (drawable == nullptr)
		return false;

	const std::vector<int> sizes = {16, 32, 48, 256};
	std::vector<juce::MemoryBlock> iconBuffers;

	for (int sz : sizes) {
		iconBuffers.push_back(renderSvgToPngBuffer(*drawable, sz));
	}

	HANDLE hUpdate = ::BeginUpdateResourceW(targetExePath.getFullPathName().toWideCharPointer(), FALSE);
	if (hUpdate == NULL)
		return false;

	for (size_t i = 0; i < sizes.size(); ++i) {
		WORD iconID = static_cast<WORD>(i + 1);

		BOOL ok = ::UpdateResourceW(hUpdate,
									MAKEINTRESOURCEW(3), // Type 3 = RT_ICON
									MAKEINTRESOURCEW(iconID), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
									(LPVOID)iconBuffers[i].getData(), (DWORD)iconBuffers[i].getSize());

		if (!ok) {
			::EndUpdateResourceW(hUpdate, TRUE);
			return false;
		}
	}

	size_t groupSize = sizeof(GRPICONDIR) + sizeof(GRPICONDIRENTRY) * (sizes.size() - 1);
	std::vector<uint8_t> groupData(groupSize, 0);

	GRPICONDIR *grpDir = reinterpret_cast<GRPICONDIR *>(groupData.data());
	grpDir->idReserved = 0;
	grpDir->idType = 1;
	grpDir->idCount = static_cast<uint16_t>(sizes.size());

	for (size_t i = 0; i < sizes.size(); ++i) {
		GRPICONDIRENTRY &entry = grpDir->idEntries[i];
		entry.bWidth = (sizes[i] >= 256) ? 0 : static_cast<uint8_t>(sizes[i]);
		entry.bHeight = (sizes[i] >= 256) ? 0 : static_cast<uint8_t>(sizes[i]);
		entry.bColorCount = 0;
		entry.bReserved = 0;
		entry.wPlanes = 1;
		entry.wBitCount = 32;
		entry.dwBytesInRes = static_cast<uint32_t>(iconBuffers[i].getSize());
		entry.nID = static_cast<uint16_t>(i + 1);
	}

	BOOL groupOk = ::UpdateResourceW(hUpdate,
									 MAKEINTRESOURCEW(14), // Type 14 = RT_GROUP_ICON
									 MAKEINTRESOURCEW(1),
									 MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), groupData.data(),
									 static_cast<DWORD>(groupData.size()));

	if (!groupOk) {
		::EndUpdateResourceW(hUpdate, TRUE);
		return false;
	}

	return ::EndUpdateResourceW(hUpdate, FALSE) != FALSE;
}

} // namespace CtrlrWinIconPatcher

#endif // JUCE_WINDOWS

#endif // CTRLR_WIN_ICON_PATCHER_H