#pragma once
#include <juce_core/juce_core.h>

class CtrlrBinaryPatcher
{
public:
    static void hexStringToBytes(const juce::String &hexString, juce::MemoryBlock &result)
    {
        result.reset();
        juce::String cleanedHex = hexString.removeCharacters(" \t\r\n");

        for (int i = 0; i < cleanedHex.length(); i += 2)
        {
            if (i + 1 < cleanedHex.length())
            {
                juce::String byteStr = cleanedHex.substring(i, i + 2);
                uint8 byte = static_cast<uint8>(byteStr.getHexValue32());
                result.append(&byte, 1);
            }
        }
    }

    static void hexStringToFixedBytes(const juce::String &str, int fixedSize, juce::MemoryBlock &result)
    {
        result.setSize(fixedSize, true); // Zero-filled memory
        const char *chars = str.toUTF8();
        int copySize = juce::jmin(fixedSize, (int)strlen(chars));
        memcpy(result.getData(), chars, copySize);
    }

    static int replaceAllOccurrences(juce::MemoryBlock &targetData, const juce::MemoryBlock &searchData, const juce::MemoryBlock &replaceData)
    {
        if (searchData.getSize() != replaceData.getSize() || searchData.getSize() == 0)
            return 0;

        int count = 0;
        const uint8 *rawData = static_cast<const uint8 *>(targetData.getData());
        size_t dataSize = targetData.getSize();
        size_t searchSize = searchData.getSize();

        for (size_t i = 0; i <= dataSize - searchSize; ++i)
        {
            if (memcmp(rawData + i, searchData.getData(), searchSize) == 0)
            {
                targetData.copyFrom(replaceData.getData(), (int)i, replaceData.getSize());
                rawData = static_cast<const uint8 *>(targetData.getData());
                count++;
            }
        }
        return count;
    }

    // JUCE 8 Wide-Char UTF-16 Buffer Generator
    static juce::MemoryBlock makeUtf16Buffer(const juce::String &text, const juce::String &templateText)
    {
        int targetCharCount = templateText.length();
        juce::MemoryBlock block(targetCharCount * 2, true);

        juce::String paddedText = text.length() > targetCharCount
                                ? text.substring(0, targetCharCount)
                                : text.paddedRight(' ', targetCharCount);

        const juce::CharPointer_UTF16 utf16Ptr = paddedText.toUTF16();
        block.copyFrom(utf16Ptr.getAddress(), 0, targetCharCount * 2);
        return block;
    }

    // Combined JUCE 8 ASCII + UTF-16 Patching Pass
    static int patchPluginBinary(juce::MemoryBlock &binaryData,
                                 const juce::String &pluginName,
                                 const juce::String &pluginCode,
                                 const juce::String &manufacturerName,
                                 const juce::String &manufacturerCode,
                                 const juce::String &plugType)
    {
        int totalReplacements = 0;

        // --- 1. ASCII Pass ---
        juce::MemoryBlock pluginNameBytes, pluginCodeBytes, manufacturerNameBytes, manufacturerCodeBytes, plugTypeBytes;
        hexStringToFixedBytes(pluginName, 32, pluginNameBytes);
        hexStringToFixedBytes(pluginCode, 4, pluginCodeBytes);
        hexStringToFixedBytes(manufacturerName, 16, manufacturerNameBytes);
        hexStringToFixedBytes(manufacturerCode, 4, manufacturerCodeBytes);
        hexStringToFixedBytes(plugType, 16, plugTypeBytes);

        juce::MemoryBlock searchPluginName, searchPluginCode, searchManufacturerName, searchManufacturerCode, searchPlugType;
        hexStringToBytes("43 74 72 6C 72 58 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20", searchPluginName);
        hexStringToBytes("63 54 78 58", searchPluginCode);
        hexStringToBytes("43 74 72 6C 72 58 20 50 72 6F 6A 65 63 74 20 20", searchManufacturerName);
        hexStringToBytes("63 54 72 6C", searchManufacturerCode);
        hexStringToBytes("49 6E 73 74 72 75 6D 65 6E 74 7C 54 6F 6F 6C 73", searchPlugType);

        totalReplacements += replaceAllOccurrences(binaryData, searchPluginName, pluginNameBytes);
        totalReplacements += replaceAllOccurrences(binaryData, searchPluginCode, pluginCodeBytes);
        totalReplacements += replaceAllOccurrences(binaryData, searchManufacturerName, manufacturerNameBytes);
        totalReplacements += replaceAllOccurrences(binaryData, searchManufacturerCode, manufacturerCodeBytes);
        totalReplacements += replaceAllOccurrences(binaryData, searchPlugType, plugTypeBytes);

        // --- 2. JUCE 8 UTF-16 Wide-String Pass ---
        juce::MemoryBlock searchUtf16ManufName, replaceUtf16ManufName;
        hexStringToBytes("43 00 74 00 72 00 6C 00 72 00 58 00 20 00 50 00 72 00 6F 00 6A 00 65 00 63 00 74 00", searchUtf16ManufName);
        replaceUtf16ManufName = makeUtf16Buffer(manufacturerName, "CtrlrX Project");

        juce::MemoryBlock searchUtf16PluginName, replaceUtf16PluginName;
        hexStringToBytes("43 00 74 00 72 00 6C 00 72 00 58 00", searchUtf16PluginName);
        replaceUtf16PluginName = makeUtf16Buffer(pluginName, "CtrlrX");

        // Execute UTF-16 wide-string patch
        totalReplacements += replaceAllOccurrences(binaryData, searchUtf16ManufName, replaceUtf16ManufName);
        totalReplacements += replaceAllOccurrences(binaryData, searchUtf16PluginName, replaceUtf16PluginName);

        return totalReplacements;
    }
};
