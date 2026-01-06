#!/usr/bin/env python3
"""
Patch script to add/fix args in LuaAPI.xml without re-parsing
Useful for methods that didn't parse correctly or have complex signatures
"""

from pathlib import Path
from xml.etree.ElementTree import parse, ElementTree, indent

xml_file = Path("Source/Resources/XML/LuaAPI.xml")

# PATCH DEFINITIONS
# Format: "ClassName": {"methodName": "args_string"}
patches = {
    "CtrlrPanel": {  # Use the real class name, not the alias
        "getModulatorByName": "String name",
        "getModulatorByIndex": "int index",
    },
    "mod": {  # This works because "mod" exists as its own class too
        "getModulatorByName": "String name",
        "getModulatorByIndex": "int index",
    },
    "CtrlrModulator": {
        "setModulatorValue": "int newValue,bool vst,bool midi,bool ui",
    },
    "MemoryBlock": {
        "loadFromHexString": "String hex",
    },
        "CtrlrMidiMessage": {
        "CtrlrMidiMessage": "{int} <or> String hex",
    },
}

print("Loading XML...")
tree = parse(xml_file)
root = tree.getroot()

total_patched = 0

for class_name, methods_to_patch in patches.items():
    # Find the class
    class_elem = root.find(f".//class[@name='{class_name}']")
    
    if class_elem is None:
        print(f"  ✗ Class '{class_name}' not found")
        continue
    
    # Find methods and static_methods sections
    methods_elem = class_elem.find("methods")
    static_elem = class_elem.find("static_methods")
    
    for method_name, args_string in methods_to_patch.items():
        found = False
        
        # Search in methods
        if methods_elem is not None:
            method = methods_elem.find(f".//method[@name='{method_name}']")
            if method is not None:
                old_args = method.get("args", "")
                method.set("args", f"({args_string})")
                print(f"  ✓ Patched {class_name}::{method_name}")
                print(f"      OLD: {old_args}")
                print(f"      NEW: ({args_string})")
                total_patched += 1
                found = True
        
        # Search in static_methods
        if not found and static_elem is not None:
            method = static_elem.find(f".//method[@name='{method_name}']")
            if method is not None:
                old_args = method.get("args", "")
                method.set("args", f"({args_string})")
                print(f"  ✓ Patched {class_name}::{method_name} (static)")
                print(f"      OLD: {old_args}")
                print(f"      NEW: ({args_string})")
                total_patched += 1
                found = True
        
        if not found:
            print(f"  ✗ Method '{class_name}::{method_name}' not found")

indent(root, space="  ")
tree.write(xml_file, encoding="utf-8", xml_declaration=True)
print(f"\n✓ Updated: {xml_file}")
print(f"✓ Total patches applied: {total_patched}")
