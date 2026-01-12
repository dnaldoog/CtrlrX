#!/usr/bin/env python3
import sys
import copy
from pathlib import Path
from xml.etree.ElementTree import parse, ElementTree, indent, Element, SubElement

# ==================== CONFIGURATION ====================
XML_PATH = Path("Source/Resources/XML/LuaAPI.xml")

# If True, any class containing <enums> will have them flattened automatically
# so Justification.Flags.left becomes Justification.left
AUTO_FLATTEN_ALL_ENUMS = True 

# Manual overrides or specific classes to target if AUTO_FLATTEN is False
FLATTEN_ENUMS = ["AttributedString", "AudioPlayHead", "ComponentPeer", "CtrlrMIDIDeviceManager", "CtrlrMidiMessage", "CtrlrModulator", "CtrlrPanel", "File", "Font", "Graphics", "Image", "Justification", "KeyPress", "LAlertWindow", "Label", "MidiMessage", "ModifierKeys", "MouseCursor", "MouseInputSource", "PathStrokeType", "RectanglePlacement", "Slider"]
# The classes you want to consolidate
CLASS_ALIASES = {
    "LMemoryBlock": ["MemoryBlock"],
    "LAlertWindow": ["AlertWindow"],
    "LBubbleMessageComponent": ["BubbleMessageComponent"],
    "LAudioFormatManager": ["AudioFormatManager"],
    "LPopupMenu": ["PopupMenu"],
    "LRelativeCoordinate": ["RelativeCoordinate"],
    "LThreadWithProgressWindow": ["ThreadWithProgressWindow"],
    "CtrlrPanel": ["panel"],
    "CtrlrModulator": ["mod", "modulator"],
}

METHOD_PATCHES = {
    "MemoryBlock": {
        "loadFromHexString": "(String hex)",
        "copyFrom": {"args": "(MemoryBlock source, int start, int size)", "type": "static"},
        "copyTo": {"args": "(MemoryBlock dest, int start, int size)", "type": "static"},
        "toHexString": "(int groupSize)",
        "toString": "()",
    },
    "CtrlrPanel": {
        "getModulatorWithProperty": [
            "(String propName, String propValue)",
            "(String propName, int propValue)"
        ],
    }
}

CONSTRUCTORS = {
    "MemoryBlock": ["()", "(String hexString)", "({int} luaTable)"],
    "CtrlrMidiMessage": ["()", "({int} midiData)", "(String hexString)"],
}

L_WRAPPED_METHODS = {
    "panel": ["getName"],
    "modulator": ["getName"],
    "CtrlrPanel": ["getName"],
    "CtrlrModulator": ["getName"],
}
# =======================================================

class LuaAPIPatcher:
    def __init__(self, xml_path):
        self.xml_path = xml_path
        self.tree = parse(xml_path)
        self.root = self.tree.getroot()

    def find_method(self, class_elem, method_name):
        for tag in ["methods", "static_methods"]:
            block = class_elem.find(tag)
            if block is not None:
                match = block.find(f"./method[@name='{method_name}']")
                if match is not None:
                    return match, block
        return None, None

    def apply_patches(self):
        print(f"[*] Patching {self.xml_path.name}...")

        # 1. Flatten Enums (Automation)
        print("[*] Flattening enum structures...")
        for cls_elem in self.root.findall("class"):
            cls_name = cls_elem.get("name")
            enums_container = cls_elem.find("enums")
            
            if enums_container is not None:
                if AUTO_FLATTEN_ALL_ENUMS or cls_name in FLATTEN_ENUMS:
                    all_values = enums_container.findall(".//value")
                    # Clear nested <enum> tags
                    for old_enum in list(enums_container.findall("enum")):
                        enums_container.remove(old_enum)
                    
                    # Create one flat group (empty name for direct access)
                    new_group = SubElement(enums_container, "enum", {"name": ""})
                    for val in all_values:
                        new_group.append(val)

        # 2. Apply Method Patches
        for cls_name, patches in METHOD_PATCHES.items():
            for cls_elem in self.root.findall(f"./class[@name='{cls_name}']"):
                for m_name, data in patches.items():
                    m_args = data if isinstance(data, (str, list)) else data.get("args")
                    m_type = data.get("type") if isinstance(data, dict) else None
                    self._patch_single_method(cls_elem, m_name, m_args, m_type)

        # 3. Apply L() Wrapper Attribute
        for cls_name, methods in L_WRAPPED_METHODS.items():
            for cls_elem in self.root.findall(f"./class[@name='{cls_name}']"):
                for m_name in methods:
                    method_elem, _ = self.find_method(cls_elem, m_name)
                    if method_elem is not None:
                        method_elem.set("lua_wrap", "L")

        # 4. Add Constructors
        for cls_name, ctors in CONSTRUCTORS.items():
            for cls_elem in self.root.findall(f"./class[@name='{cls_name}']"):
                for existing in cls_elem.findall("constructor"):
                    cls_elem.remove(existing)
                for args in ctors:
                    SubElement(cls_elem, "constructor", {"args": args, "type": "constructor"})

# 5. Consolidate Wrappers and Create Aliases
        print("[*] Consolidating wrappers and creating aliases...")
        for original, aliases in CLASS_ALIASES.items():
            orig_elem = self.root.find(f"./class[@name='{original}']")
            
            if orig_elem is not None:
                for alias in aliases:
                    target_elem = self.root.find(f"./class[@name='{alias}']")
                    
                    if target_elem is not None:
                        # MERGE LOGIC: Move methods/enums from L-class to Base-class
                        print(f"    [+] Merging {original} into {alias}")
                        
                        for tag in ["methods", "static_methods", "enums", "constructor"]:
                            source_parts = orig_elem.findall(tag)
                            if source_parts:
                                dest_container = target_elem.find(tag)
                                if dest_container is None:
                                    dest_container = SubElement(target_elem, tag)
                                
                                for item in source_parts:
                                    # Avoid duplicates if the method exists in both
                                    if item.tag == "method":
                                        m_name = item.get("name")
                                        if dest_container.find(f"./method[@name='{m_name}']") is None:
                                            dest_container.append(copy.deepcopy(item))
                                    else:
                                        dest_container.append(copy.deepcopy(item))
                    else:
                        # ALIAS LOGIC: Base class didn't exist, create it as a copy
                        print(f"    [+] Aliasing {original} as {alias}")
                        new_alias = copy.deepcopy(orig_elem)
                        new_alias.set("name", alias)
                        self.root.append(new_alias)

                # Optional: Remove the original "L" class to keep the API clean
                if original.startswith("L") and original != "L": 
                    print(f"    [-] Removing wrapper class: {original}")
                    self.root.remove(orig_elem)

    def _patch_single_method(self, cls_elem, m_name, args, force_type=None):
        method_elem, current_parent = self.find_method(cls_elem, m_name)
        
        # Handle Overloads (List of arg strings)
        if isinstance(args, list):
            if method_elem is not None: current_parent.remove(method_elem)
            target_parent = current_parent if current_parent is not None else cls_elem.find("methods")
            if target_parent is None: target_parent = SubElement(cls_elem, "methods")
            for i, arg_str in enumerate(args, 1):
                SubElement(target_parent, "method", {
                    "name": m_name, "args": arg_str, "overload": str(i), 
                    "type": "instance" if target_parent.tag == "methods" else "static"
                })
        # Handle Single Patch
        else:
            if method_elem is not None:
                method_elem.set("args", args)
                if force_type == "static" and current_parent.tag == "methods":
                    current_parent.remove(method_elem)
                    static_block = cls_elem.find("static_methods")
                    if static_block is None: static_block = SubElement(cls_elem, "static_methods")
                    method_elem.set("type", "static")
                    static_block.append(method_elem)

    def save(self):
        indent(self.root, space="  ")
        self.tree.write(self.xml_path, encoding="utf-8", xml_declaration=True)
        print(f"[+] Patching complete. {len(self.root.findall('class'))} classes in final API.")

if __name__ == "__main__":
    if not XML_PATH.exists():
        print(f"Error: {XML_PATH} not found.")
        sys.exit(1)
    patcher = LuaAPIPatcher(XML_PATH)
    patcher.apply_patches()
    patcher.save()
