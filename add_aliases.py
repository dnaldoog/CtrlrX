#!/usr/bin/env python3
"""
Enhanced patch script for LuaAPI.xml
- Add/fix args without re-parsing
- Support for overloaded methods
- Add constructors
- Add class aliases

Usage:
  python add_aliases.py                    # Apply patches
  python add_aliases.py --list-classes     # List all class names in XML

IMPORTANT: Use the actual class names from the XML file!
Run with --list-classes to see what they are.
"""

from pathlib import Path
from xml.etree.ElementTree import parse, ElementTree, indent, Element, SubElement
import copy
import sys

xml_file = Path("Source/Resources/XML/LuaAPI.xml")

# ================= PATCH DEFINITIONS =================
#
# HOW IT WORKS:
# 1. METHOD_PATCHES applies to XML class names (CtrlrPanel, LMemoryBlock, etc.)
# 2. CLASS_ALIASES creates user-friendly names (panel, MemoryBlock, etc.)
# 3. Aliases automatically inherit the patches from their parent class!
#
# Example:
#   - You patch "CtrlrPanel" with args
#   - You create alias "panel" -> CtrlrPanel
#   - Users search for "panel" and see all the patched methods with args!
#
# ==========================================================

# CLASS ALIASES - Use actual XML class names -> user-friendly aliases
# The patches apply to the XML class, and aliases inherit them automatically
CLASS_ALIASES = {
    "CtrlrPanel": ["panel"],                  # Users use: panel (never CtrlrPanel)
    "CtrlrModulator": ["mod", "modulator"],   # Users use: mod or modulator
    "LMemoryBlock": ["MemoryBlock"],          # Users use: MemoryBlock (never LMemoryBlock)
}

# METHOD ARGS - Use XML class names (patches auto-apply to aliases)
# The aliases (panel, MemoryBlock, etc.) will inherit these patches
METHOD_PATCHES = {
    "CtrlrPanel": {  # This patches both CtrlrPanel AND its alias "panel"
        "getModulatorByName": "(String name)",
        "getModulatorByIndex": "(int index)",
        "getGlobalVariable": "(int index)",
        "setGlobalVariable": "(int index, int value)",
        "getModulatorWithProperty": [
            "(String propertyName, String propertyValue)",
            "(String propertyName, int propertyValue)"
        ],
    },
    "CtrlrModulator": {  # This patches CtrlrModulator, mod, and modulator
        "setModulatorValue": "(int newValue, bool vst, bool midi, bool ui)",
        "setValue": [
            "(int newValue, bool force)",
            "(int newValue, bool force, bool mute)"
        ],
    },
    "LMemoryBlock": {  # This patches LMemoryBlock AND its alias "MemoryBlock"
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

# CONSTRUCTORS - Use XML class names (aliases inherit these too)
CONSTRUCTORS = {
    "LMemoryBlock": [  # Users will see this as "MemoryBlock(...)"
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
    added_count = 0
    
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
            
            # Deep copy the original class element
            alias_elem = copy.deepcopy(original)
            
            # Update the name attribute
            alias_elem.set("name", alias)
            alias_elem.set("alias_of", original_name)
            
            # Append to root
            root.append(alias_elem)
            
            print(f"  ✓ Created alias: {original_name} -> {alias}")
            added_count += 1
    
    return added_count


def patch_method_args(root):
    """Add or update args attribute for methods."""
    print("\n=== Patching Method Arguments ===")
    total_patched = 0
    
    for class_name, methods_to_patch in METHOD_PATCHES.items():
        # Find all classes with this name (including aliases)
        class_elems = root.findall(f".//class[@name='{class_name}']")
        
        if not class_elems:
            print(f"  ✗ Class '{class_name}' not found")
            continue
        
        for class_elem in class_elems:
            display_name = class_elem.get("name")
            
            methods_elem = class_elem.find("methods")
            static_elem = class_elem.find("static_methods")
            
            for method_name, args_data in methods_to_patch.items():
                # Handle overloads
                if isinstance(args_data, list):
                    # Multiple overloads
                    found_methods = []
                    parent_elem = None
                    
                    if methods_elem is not None:
                        found_methods.extend(methods_elem.findall(f".//method[@name='{method_name}']"))
                        parent_elem = methods_elem
                    if static_elem is not None:
                        static_methods = static_elem.findall(f".//method[@name='{method_name}']")
                        if static_methods:
                            found_methods.extend(static_methods)
                            parent_elem = static_elem
                    
                    if not found_methods:
                        print(f"  ✗ Method '{display_name}::{method_name}' not found for overloads")
                        continue
                    
                    # Update existing methods
                    for idx, args in enumerate(args_data):
                        if idx < len(found_methods):
                            method = found_methods[idx]
                            old_args = method.get("args", "")
                            method.set("args", args)
                            method.set("overload", str(idx + 1))
                            print(f"  ✓ Patched {display_name}::{method_name} [overload {idx + 1}]")
                            if old_args != args:
                                print(f"      {old_args} -> {args}")
                            total_patched += 1
                        else:
                            # Add missing overload
                            if parent_elem is not None:
                                new_method = SubElement(parent_elem, "method", {
                                    "name": method_name,
                                    "type": "instance" if parent_elem.tag == "methods" else "static",
                                    "args": args,
                                    "overload": str(idx + 1)
                                })
                                print(f"  + Added {display_name}::{method_name} [overload {idx + 1}]: {args}")
                                total_patched += 1
                
                else:
                    # Single method
                    found = False
                    
                    if methods_elem is not None:
                        method = methods_elem.find(f".//method[@name='{method_name}']")
                        if method is not None:
                            old_args = method.get("args", "")
                            method.set("args", args_data)
                            print(f"  ✓ Patched {display_name}::{method_name}")
                            if old_args != args_data:
                                print(f"      {old_args} -> {args_data}")
                            total_patched += 1
                            found = True
                    
                    if not found and static_elem is not None:
                        method = static_elem.find(f".//method[@name='{method_name}']")
                        if method is not None:
                            old_args = method.get("args", "")
                            method.set("args", args_data)
                            print(f"  ✓ Patched {display_name}::{method_name} (static)")
                            if old_args != args_data:
                                print(f"      {old_args} -> {args_data}")
                            total_patched += 1
                            found = True
                    
                    if not found:
                        print(f"  ✗ Method '{display_name}::{method_name}' not found")
    
    return total_patched


def add_constructors(root):
    """Add constructor entries to classes."""
    print("\n=== Adding Constructors ===")
    total_added = 0
    
    for class_name, ctor_list in CONSTRUCTORS.items():
        class_elems = root.findall(f".//class[@name='{class_name}']")
        
        if not class_elems:
            print(f"  ✗ Class '{class_name}' not found")
            continue
        
        for class_elem in class_elems:
            display_name = class_elem.get("name")
            
            # Check if constructors already exist
            existing = class_elem.findall("constructor")
            if existing:
                print(f"  ⊙ {display_name} already has {len(existing)} constructor(s)")
                continue
            
            # Add constructors at the beginning
            for idx, args in enumerate(ctor_list):
                ctor = Element("constructor", {"args": args})
                class_elem.insert(idx, ctor)
                
                print(f"  ✓ Added {display_name}{args}")
                total_added += 1
    
    return total_added


def list_classes():
    """List all classes in the XML for easy reference."""
    if not xml_file.exists():
        print(f"Error: {xml_file} not found!")
        return
    
    tree = parse(xml_file)
    root = tree.getroot()
    
    classes = root.findall(".//class")
    
    print(f"\n{'='*60}")
    print(f"Classes in {xml_file}")
    print(f"{'='*60}\n")
    
    print(f"{'XML Name':<25} {'C++ Name':<30} {'Alias Of':<15}")
    print("-" * 60)
    
    for cls in sorted(classes, key=lambda c: c.get('name', '')):
        name = cls.get('name', '?')
        cpp_name = cls.get('cpp_name', '?')
        alias_of = cls.get('alias_of', '')
        
        print(f"{name:<25} {cpp_name:<30} {alias_of:<15}")
    
    print(f"\nTotal classes: {len(classes)}")
    print(f"\nUse the 'XML Name' column in your patch definitions!\n")


def main():
    # Check for --list-classes flag
    if len(sys.argv) > 1 and sys.argv[1] in ['--list-classes', '-l', 'list']:
        list_classes()
        return
    
    if not xml_file.exists():
        print(f"Error: {xml_file} not found!")
        print(f"Current directory: {Path.cwd()}")
        print(f"\nRun with --list-classes to see available classes")
        return
    
    print(f"Loading XML: {xml_file}")
    tree = parse(xml_file)
    root = tree.getroot()
    
    # Apply patches
    aliases_added = add_class_aliases(root)
    patched = patch_method_args(root)
    added = add_constructors(root)
    
    # Save with formatting
    indent(root, space="  ")
    tree.write(xml_file, encoding="utf-8", xml_declaration=True)
    
    print(f"\n{'='*50}")
    print(f"✓ Updated: {xml_file}")
    print(f"✓ Class aliases added: {aliases_added}")
    print(f"✓ Method patches applied: {patched}")
    print(f"✓ Constructors added: {added}")
    print(f"{'='*50}")


if __name__ == "__main__":
    main()