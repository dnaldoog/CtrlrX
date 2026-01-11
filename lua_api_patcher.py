#!/usr/bin/env python3
import sys
import copy
from pathlib import Path
from xml.etree.ElementTree import parse, ElementTree, indent, Element, SubElement

# ==================== CONFIGURATION ====================
XML_PATH = Path("Source/Resources/XML/LuaAPI.xml")

# ALIASES: Original XML Class -> [New Friendly Names]
CLASS_ALIASES = {
    "CtrlrPanel": ["panel"],
    "CtrlrModulator": ["mod", "modulator"],
    "LMemoryBlock": ["MemoryBlock"],
    "CtrlrLuaUtils": ["utils"],
}

# METHOD PATCHES: Class Name -> { MethodName: Args or [Overloads] }
# TIP: If a method should be static but is in the wrong block, 
# add "type": "static" in the dictionary format below.
METHOD_PATCHES = {
    "LMemoryBlock": {
        "loadFromHexString": "(String hex)",
        "copyFrom": {"args": "(MemoryBlock source, int start, int size)", "type": "static"},
        "copyTo": {"args": "(MemoryBlock dest, int start, int size)", "type": "static"},
        "toHexString": "(int groupSize)",
        "toString": "()",
    },
    "CtrlrPanel": {
        "getModulatorByName": "(String name)",
        "getModulatorWithProperty": [
            "(String propName, String propValue)",
            "(String propName, int propValue)"
        ],
    },
    "CtrlrLuaUtils": {
        "unpackDsiData": "(MemoryBlock data)",
        "packDsiData": "(MemoryBlock data)",
    }
}

# CONSTRUCTORS: Class Name -> [Arg Strings]
CONSTRUCTORS = {
    "LMemoryBlock": ["()", "(String hexString)", "({int} luaTable)"],
    "CtrlrMidiMessage": ["()", "({int} midiData)", "(String hexString)"],
}
# =======================================================

class LuaAPIPatcher:
    def __init__(self, xml_path):
        self.xml_path = xml_path
        self.tree = parse(xml_path)
        self.root = self.tree.getroot()

    def find_method(self, class_elem, method_name):
        """Finds a method in either instance or static blocks."""
        for tag in ["methods", "static_methods"]:
            block = class_elem.find(tag)
            if block is not None:
                match = block.find(f"./method[@name='{method_name}']")
                if match is not None:
                    return match, block
        return None, None

    def apply_patches(self):
        print(f"[*] Patching {self.xml_path.name}...")

        # 1. Apply Method Patches to Original Classes
        for cls_name, patches in METHOD_PATCHES.items():
            cls_elems = self.root.findall(f".//class[@name='{cls_name}']")
            for cls_elem in cls_elems:
                for m_name, data in patches.items():
                    # Standardize data to a dict
                    m_args = data if isinstance(data, (str, list)) else data.get("args")
                    m_type = data.get("type") if isinstance(data, dict) else None
                    
                    self._patch_single_method(cls_elem, m_name, m_args, m_type)

        # 2. Add Constructors
        for cls_name, ctors in CONSTRUCTORS.items():
            for cls_elem in self.root.findall(f".//class[@name='{cls_name}']"):
                # Clear existing if any, then add
                for existing in cls_elem.findall("constructor"):
                    cls_elem.remove(existing)
                for args in ctors:
                    SubElement(cls_elem, "constructor", {"args": args, "type": "constructor"})

        # 3. Create Aliases (Inherits all patches applied above)
        for original, aliases in CLASS_ALIASES.items():
            orig_elem = self.root.find(f".//class[@name='{original}']")
            if orig_elem is not None:
                for alias in aliases:
                    # Remove existing alias if it exists to avoid duplicates
                    existing = self.root.find(f".//class[@name='{alias}']")
                    if existing is not None: self.root.remove(existing)
                    
                    new_alias = copy.deepcopy(orig_elem)
                    new_alias.set("name", alias)
                    new_alias.set("alias_of", original)
                    self.root.append(new_alias)

    def _patch_single_method(self, cls_elem, m_name, args, force_type=None):
        method_elem, current_parent = self.find_method(cls_elem, m_name)

        # Handle Overloads (List of arg strings)
        if isinstance(args, list):
            if method_elem is not None: current_parent.remove(method_elem) # Clear old
            target_parent = current_parent if current_parent is not None else cls_elem.find("methods")
            for i, arg_str in enumerate(args, 1):
                SubElement(target_parent, "method", {
                    "name": m_name, "args": arg_str, "overload": str(i), 
                    "type": "instance" if target_parent.tag == "methods" else "static"
                })
        # Handle Single String
        else:
            if method_elem is not None:
                method_elem.set("args", args)
                # If we need to move it from instance to static (or vice versa)
                if force_type == "static" and current_parent.tag == "methods":
                    current_parent.remove(method_elem)
                    static_block = cls_elem.find("static_methods")
                    if static_block is None: static_block = SubElement(cls_elem, "static_methods")
                    method_elem.set("type", "static")
                    static_block.append(method_elem)

    def save(self):
        indent(self.root, space="  ")
        self.tree.write(self.xml_path, encoding="utf-8", xml_declaration=True)
        print("[+] Patching complete and saved.")

if __name__ == "__main__":
    if not XML_PATH.exists():
        print(f"Error: Could not find {XML_PATH}")
        sys.exit(1)
    patcher = LuaAPIPatcher(XML_PATH)
    patcher.apply_patches()
    patcher.save()