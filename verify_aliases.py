#!/usr/bin/env python3
"""
Quick verification script to check if aliases exist in LuaAPI.xml
"""

from pathlib import Path
from xml.etree.ElementTree import parse

xml_file = Path("Source/Resources/XML/LuaAPI.xml")

if not xml_file.exists():
    print(f"ERROR: {xml_file} not found!")
    exit(1)

print("=" * 60)
print(f"Checking aliases in {xml_file}")
print("=" * 60)
print()

tree = parse(xml_file)
root = tree.getroot()

# Check specific classes
check_classes = [
    ("CtrlrPanel", "original"),
    ("panel", "alias"),
    ("LMemoryBlock", "original"),
    ("MemoryBlock", "alias"),
    ("CtrlrModulator", "original"),
    ("mod", "alias"),
    ("modulator", "alias"),
]

for class_name, label in check_classes:
    cls = root.find(f".//class[@name='{class_name}']")
    if cls:
        alias_of = cls.get("alias_of", "")
        if alias_of:
            print(f"✓ {class_name:<20} ({label}) -> alias of {alias_of}")
        else:
            print(f"✓ {class_name:<20} ({label})")
    else:
        print(f"✗ {class_name:<20} ({label}) NOT FOUND!")

print()
print("=" * 60)
print("All classes in XML:")
print("=" * 60)

all_classes = root.findall(".//class")
for cls in sorted(all_classes, key=lambda c: c.get('name', '')):
    name = cls.get('name', '?')
    alias_of = cls.get('alias_of', '')
    
    if alias_of:
        print(f"  {name:<25} (alias of {alias_of})")
    else:
        print(f"  {name}")

print()
print(f"Total classes: {len(all_classes)}")