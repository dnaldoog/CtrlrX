#!/usr/bin/env python3
"""
Quick script to add shorthand aliases to LuaAPI.xml without re-parsing
Ensures both original class name and alias exist for proper chaining
"""

from pathlib import Path
from xml.etree.ElementTree import Element, SubElement, ElementTree, indent, parse

xml_file = Path("Source/Resources/XML/LuaAPI.xml")
tree = parse(xml_file)
root = tree.getroot()

# Map of alias -> original cpp class name
aliases = {
    "mod": "CtrlrModulator",
    "comp": "Component",
    "g": "Graphics",
    "event": "MouseEvent",
    "panel": "CtrlrPanel"
}

added_count = 0

for alias_name, original_cpp_name in aliases.items():
    
    # Check if alias already exists
    if root.find(f".//class[@name='{alias_name}']") is not None:
        print(f"  ✓ Alias '{alias_name}' already exists, skipping")
        continue
    
    # Find the original class (by cpp_name attribute OR by name)
    original = root.find(f".//class[@cpp_name='{original_cpp_name}']")
    if original is None:
        original = root.find(f".//class[@name='{original_cpp_name}']")
    
    if original is not None:
        original_name = original.get("name")
        
        # Step 1: Ensure the original class exists with cpp_name
        # If it doesn't have cpp_name set, add it
        if original.get("cpp_name") is None or original.get("cpp_name") == "":
            original.set("cpp_name", original_cpp_name)
        
        # Step 2: Create the alias pointing to the original
        alias_elem = SubElement(root, "class", {
            "name": alias_name,
            "cpp_name": original_cpp_name,
            "alias": original_name  # Reference back to the original
        })
        
        # Copy methods element if it exists
        methods = original.find("methods")
        if methods is not None:
            methods_copy = SubElement(alias_elem, "methods")
            for method in methods:
                new_method = SubElement(methods_copy, "method")
                new_method.attrib.update(method.attrib)
        
        # Copy static_methods if it exists
        static = original.find("static_methods")
        if static is not None:
            static_copy = SubElement(alias_elem, "static_methods")
            for method in static:
                new_method = SubElement(static_copy, "method")
                new_method.attrib.update(method.attrib)
        
        print(f"  ✓ Added alias: '{alias_name}' -> '{original_name}' (cpp: {original_cpp_name})")
        added_count += 1
    else:
        print(f"  ✗ Warning: Original class '{original_cpp_name}' not found")

# Ensure we also have the full cpp names as classes (for resolveReturnType to work)
# For example, make sure "CtrlrModulator" exists alongside "mod"
for alias_name, cpp_name in aliases.items():
    if root.find(f".//class[@name='{cpp_name}']") is None and cpp_name != alias_name:
        # Find by cpp_name
        original = root.find(f".//class[@cpp_name='{cpp_name}']")
        if original is not None:
            original_name = original.get("name")
            if original_name != cpp_name:
                # Create an entry for the cpp_name as well
                cpp_class = SubElement(root, "class", {
                    "name": cpp_name,
                    "cpp_name": cpp_name,
                    "alias": original_name
                })
                
                # Copy methods
                methods = original.find("methods")
                if methods is not None:
                    methods_copy = SubElement(cpp_class, "methods")
                    for method in methods:
                        new_method = SubElement(methods_copy, "method")
                        new_method.attrib.update(method.attrib)
                
                # Copy static_methods
                static = original.find("static_methods")
                if static is not None:
                    static_copy = SubElement(cpp_class, "static_methods")
                    for method in static:
                        new_method = SubElement(static_copy, "method")
                        new_method.attrib.update(method.attrib)
                
                print(f"  ✓ Added cpp_name alias: '{cpp_name}' (for chaining)")
                added_count += 1

indent(root, space="  ")
tree.write(xml_file, encoding="utf-8", xml_declaration=True)
print(f"\n✓ Updated: {xml_file}")
print(f"✓ Added {added_count} new classes/aliases")