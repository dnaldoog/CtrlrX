#!/usr/bin/env python3
import re
import sys
from pathlib import Path
from xml.etree.ElementTree import Element, SubElement, ElementTree, indent
from collections import defaultdict
import copy

# ================= CONFIG & MANUAL PATCHES =================
VERBOSE = True
STRIP_NAMESPACES = True

FORCE_STATIC_CLASSES = ["CtrlrLuaUtils"]
CLASS_ALIASES = {
    "CtrlrPanel": ["panel"],
    "CtrlrModulator": ["mod", "modulator"],
    "LMemoryBlock": ["MemoryBlock"],
    "CtrlrLuaUtils": ["utils"],
}

# Use this to fix methods the indexer misses or misidentifies
MANUAL_OVERRIDES = {
    "MemoryBlock": {
        "copyFrom": {"type": "static", "args": "(MemoryBlock, int, int)"},
        "copyTo": {"type": "static", "args": "(MemoryBlock, int, int)"},
        "loadFromHexString": {"args": "(String)"},
        "fromBase64Encoding": {"args": "(String)"},
        "fillWith": {"args": "(int)"},
        "ensureSize": {"args": "(int)"},
        "getData": {"args": "()"},
    }
}
# ==========================================================

def strip_namespace(name: str) -> str:
    if not STRIP_NAMESPACES: return name.strip()
    return name.split("::")[-1].strip()

def clean_cpp_args(args_raw: str) -> str:
    if not args_raw or not args_raw.strip(): return "()"
    parts = []
    raw_parts = re.split(r',(?![^<]*>)', args_raw)
    for arg in raw_parts:
        arg = arg.strip().split('=')[0].strip()
        arg = re.sub(r'\b(const|volatile)\b|[&*]', '', arg)
        arg = arg.replace("luabind::object", "table")
        type_only = arg.split()
        if type_only:
            # Handle names like 'juce::String' -> 'String'
            parts.append(type_only[0].split('::')[-1])
    return f"({', '.join(parts)})"

def file_contains_lua_bindings(filepath: Path) -> bool:
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            chunk = f.read(8192)
            return any(x in chunk for x in ["wrapForLua", "module(L)", "class_<", "luabind::"])
    except: return False

class LuabindParser:
    def __init__(self):
        self.classes = {}
        self.cpp_definition_map = {}

    def index_source_files(self, source_dir: Path):
        if VERBOSE: print("[*] Building C++ type index...")
        for f in source_dir.glob("**/*.[ch]*"):
            try:
                content = f.read_text(encoding="utf-8", errors="ignore")
                # Updated regex to be more aggressive in finding signatures
                def_pattern = r'(?:\w+)?\s+([\w:]+)::(\w+)\s*\(([^)]*)\)'
                for match in re.finditer(def_pattern, content):
                    cls, meth, args = match.groups()
                    self.cpp_definition_map[f"{strip_namespace(cls)}::{meth}"] = args
            except: continue

    def extract_args(self, signature_text: str, current_cpp_class: str, method_name: str, lua_name: str) -> str:
        # Check Manual Overrides first
        if lua_name in MANUAL_OVERRIDES and method_name in MANUAL_OVERRIDES[lua_name]:
            if "args" in MANUAL_OVERRIDES[lua_name][method_name]:
                return MANUAL_OVERRIDES[lua_name][method_name]["args"]

        # Check for Luabind explicit cast
        cast_match = re.search(r'\([^)]*\)\s*\(([^)]*)\)', signature_text)
        if cast_match:
            return clean_cpp_args(cast_match.group(1))
        
        # Check C++ Index
        keys = [f"{current_cpp_class}::{method_name}", f"L{current_cpp_class}::{method_name}"]
        for k in keys:
            if k in self.cpp_definition_map:
                return clean_cpp_args(self.cpp_definition_map[k])
            
        return "()"

    def parse_file(self, filepath: Path):
        content = filepath.read_text(encoding="utf-8", errors="ignore")
        clean_content = re.sub(r'//.*', '', content)
        clean_content = re.sub(r'/\*.*?\*/', '', clean_content, flags=re.S)

        for m in re.finditer(r'(?:luabind::)?module\s*\(\s*L\s*\)\s*\[', clean_content):
            body = self.extract_bracket_block(clean_content, m.end() - 1)
            if body: self.parse_module(body)

    def extract_bracket_block(self, text: str, start: int):
        depth, in_string, escape = 0, False, False
        for i in range(start, len(text)):
            c = text[i]
            if escape: { escape := False }; continue
            if c == '\\': escape = True; continue
            if c == '"': in_string = not in_string; continue
            if in_string: continue
            if c == '[': depth += 1
            elif c == ']':
                depth -= 1
                if depth == 0: return text[start + 1:i]
        return None

    def parse_module(self, content: str):
        class_chunks = re.split(r'(class_<\s*[\w:]+)', content)
        for i in range(1, len(class_chunks), 2):
            block = class_chunks[i] + class_chunks[i+1]
            self.parse_class_block(block)

    def parse_class_block(self, block: str):
        header = re.search(r'class_<\s*([\w:]+).*?>\s*(?:\(\s*"([^"]+)"\s*\))?', block, re.DOTALL)
        if not header: return

        cpp_class = strip_namespace(header.group(1))
        lua_name = header.group(2) or cpp_class

        cls = self.classes.setdefault(lua_name, {
            "cpp_name": cpp_class, "methods": defaultdict(list), "static": defaultdict(list), "constructors": [],
        })

        for m in re.finditer(r'\.def\s*\(\s*"([^"]+)"\s*,\s*([^,)]+)', block):
            name, sig = m.group(1), m.group(2)
            if name == "constructor": continue
            
            args = self.extract_args(sig, cpp_class, name, lua_name)
            
            # Forced Static Check
            is_static = False
            if lua_name in MANUAL_OVERRIDES and name in MANUAL_OVERRIDES[lua_name]:
                if MANUAL_OVERRIDES[lua_name][name].get("type") == "static":
                    is_static = True
            
            if not is_static:
                is_static = ".scope" in block and block.find(name) > block.find(".scope")
            
            if is_static:
                cls["static"][name].append(args)
            else:
                cls["methods"][name].append(args)

    def apply_final_patches(self):
        for target in FORCE_STATIC_CLASSES:
            if target in self.classes:
                c = self.classes[target]
                c["static"].update(c["methods"])
                c["methods"] = defaultdict(list)

        new_classes = {}
        for original, aliases in CLASS_ALIASES.items():
            if original in self.classes:
                for a in aliases:
                    new_classes[a] = copy.deepcopy(self.classes[original])
        self.classes.update(new_classes)

    def generate_xml(self, output_file: str):
        root = Element("LuaAPI")
        for name in sorted(self.classes):
            c = self.classes[name]
            ce = SubElement(root, "class", {"name": name, "cpp_name": c["cpp_name"]})
            for sec_key, sec_tag, type_val in [("methods", "methods", "instance"), ("static", "static_methods", "static")]:
                if c[sec_key]:
                    node = SubElement(ce, sec_tag)
                    for m_name, sigs in sorted(c[sec_key].items()):
                        for i, args in enumerate(sigs, 1):
                            attr = {"name": m_name, "type": type_val, "args": args}
                            if len(sigs) > 1: attr["overload"] = str(i)
                            SubElement(node, "method", attr)
        indent(root, space="  ")
        ElementTree(root).write(output_file, encoding="utf-8", xml_declaration=True)

if __name__ == "__main__":
    if len(sys.argv) != 3: sys.exit(1)
    parser = LuabindParser()
    path = Path(sys.argv[1])
    parser.index_source_files(path)
    cpp_files = [f for f in sorted(path.glob("**/*.cpp")) if file_contains_lua_bindings(f)]
    for f in cpp_files: parser.parse_file(f)
    parser.apply_final_patches()
    parser.generate_xml(sys.argv[2])
