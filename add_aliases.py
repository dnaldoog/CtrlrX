#!/usr/bin/env python3
"""
Enhanced patch script for LuaAPI.xml
- Add/fix args without re-parsing
- Support for overloaded methods
- Add constructors
- Add class aliases

Usage:
  python add_aliases.py
"""

from pathlib import Path
from xml.etree.ElementTree import parse, ElementTree, indent, SubElement

xml_file = Path("Source/Resources/XML/LuaAPI.xml")

# ================= PATCH DEFINITIONS =================

# CLASS ALIASES - creates duplicate class entries with alias name
CLASS_ALIASES = {
    "CtrlrPanel": ["panel"],
    "CtrlrModulator": ["mod", "modulator"],
    "MemoryBlock": ["mem"],
}

# METHOD ARGS - Format: "ClassName": {"methodName": "args" or ["overload1", "overload2"]}
METHOD_PATCHES = {
    "CtrlrPanel": {
        "getModulatorByName": "(String name)",
        "getModulatorByIndex": "(int index)",
        "getGlobalVariable": "(int index)",
        "setGlobalVariable": "(int index, int value)",
        "getModulatorWithProperty": [
            "(String propertyName, String propertyValue)",
            "(String propertyName, int propertyValue)"
        ],
    },
    "CtrlrModulator": {
        "setModulatorValue": "(int newValue, bool vst, bool midi, bool ui)",
        "setValue": [
            "(int newValue, bool force)",
            "(int newValue, bool force, bool mute)"
        ],
    },
    "MemoryBlock": {
        "loadFromHexString": "(String hex)",
        "toString": "()",
        "toHexString": "(int groupSize)",
    },
    "CtrlrMidiMessage": {
        "CtrlrMidiMessage": [
            "()",
            "({int} midiData)",
            "(String hexString)"
        ],
    },
}

# CONSTRUCTORS - Format: "ClassName": ["args1", "args2", ...]
CONSTRUCTORS = {
    "MemoryBlock": [
        "()",
        "(String hexString)",
        "({int} luaTable)"
    ],
    "CtrlrMidiMessage": [
        "()",
        "({int} midiData)",
        "(String hexString)"
    ],
}

# =====================================================


def add_class_aliases(root):
    """Create duplicate class entries with alias names."""
    print("\n=== Adding Class Aliases ===")
    
    for original_name, aliases in CLASS_ALIASES.items():
        original = root.find(f".//class[@name='{original_name}']")
        
        if original is None:
            print(f"  ✗ Class '{original_name}' not found")
            continue
        
        for alias in aliases:
            # Check if alias already exists
            if root.find(f".//class[@name='{alias}']") is not None:
                print(f"  ⊙ Alias '{alias}' already exists")
                continue
            
            # Create a shallow copy with new name
            alias_elem = SubElement(root, "class", {
                "name": alias,
                "cpp_name": original.get("cpp_name"),
                "alias_of": original_name
            })
            
            # Copy all child elements
            for child in original:
                alias_elem.append(child)
            
            print(f"  ✓ Created alias: {original_name} -> {alias}")


def patch_method_args(root):
    """Add or update args attribute for methods."""
    print("\n=== Patching Method Arguments ===")
    t
