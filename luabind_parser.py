#!/usr/bin/env python3
import re
import sys
from pathlib import Path
from xml.etree.ElementTree import Element, SubElement, ElementTree, indent
from collections import defaultdict
import copy

# ================= DEFAULT PATHS =================
DEFAULT_SRC = Path(".")
DEFAULT_OUT = Path("Source/Resources/XML/LuaAPI.xml")

# ================= CONFIG & MANUAL PATCHES =================
VERBOSE = True
STRIP_NAMESPACES = True

FORCE_STATIC_CLASSES = ["CtrlrLuaUtils"]
CLASS_ALIASES = {}
MANUAL_OVERRIDES = {}

def strip_namespace(name: str) -> str:
    if not STRIP_NAMESPACES or not name: return name.strip() if name else ""
    return name.split("::")[-1].strip()

def clean_cpp_args(args_raw: str) -> str:
    if not args_raw or not args_raw.strip(): return "()"
    parts = []
    raw_parts = re.split(r',(?![^<]*>)', args_raw)
    for arg in raw_parts:
        arg = arg.strip().split('=')[0].strip()
        arg = re.sub(r'\b(const|volatile|inline|static)\b|[&*]', '', arg)
        arg = arg.replace("luabind::object", "table")
        type_only = arg.split()
        if type_only:
            parts.append(type_only[0].split('::')[-1])
    return f"({', '.join(parts)})"

class LuabindParser:
    def __init__(self):
        self.classes = {}
        self.cpp_definition_map = {}

    def index_source_files(self, source_dir: Path):
        print("[*] Building C++ type index... Please wait.")
        for f in source_dir.glob("**/*.[ch]*"):
            try:
                content = f.read_text(encoding="utf-8", errors="ignore")
                def_pattern = r'(?:\w+)?\s+([\w:]+)::(\w+)\s*\(([^)]*)\)'
                for match in re.finditer(def_pattern, content):
                    cls, meth, args = match.groups()
                    self.cpp_definition_map[f"{strip_namespace(cls)}::{meth}"] = args
            except: continue

    def extract_args(self, signature_text: str, current_cpp_class: str, method_name: str, lua_name: str) -> str:
        if lua_name in MANUAL_OVERRIDES and method_name in MANUAL_OVERRIDES[lua_name]:
            if "args" in MANUAL_OVERRIDES[lua_name][method_name]:
                return MANUAL_OVERRIDES[lua_name][method_name]["args"]
        cast_match = re.search(r'\([^)]*\)\s*\(([^)]*)\)', signature_text)
        if cast_match: return clean_cpp_args(cast_match.group(1))
        keys = [f"{current_cpp_class}::{method_name}", f"L{current_cpp_class}::{method_name}"]
        for k in keys:
            if k in self.cpp_definition_map: return clean_cpp_args(self.cpp_definition_map[k])
        return "()"

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

    def parse_file(self, filepath: Path):
        try:
            content = filepath.read_text(encoding="utf-8", errors="ignore")
        except: return
        clean_content = re.sub(r'//.*', '', content)
        clean_content = re.sub(r'/\*.*?\*/', lambda m: ' ' * len(m.group()), clean_content, flags=re.S)

        for m in re.finditer(r'(?:luabind::)?module\s*\(\s*\w+\s*\)\s*\[', clean_content):
            body = self.extract_bracket_block(clean_content, m.end() - 1)
            if body: self.parse_module(body)

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
            "cpp_name": cpp_class, "methods": defaultdict(list), "static": defaultdict(list), "enums": {}
        })

        # 1. Instance Methods (.def)
        # We stop at the first .scope we find to avoid mis-identifying static as instance
        scope_pos = block.find(".scope")
        instance_area = block[:scope_pos] if scope_pos != -1 else block
        
        for m in re.finditer(r'\.def\s*\(\s*"([^"]+)"\s*,\s*([^,)]+)', instance_area):
            name, sig = m.group(1), m.group(2)
            if name == "constructor": continue
            args = self.extract_args(sig, cpp_class, name, lua_name)
            cls["methods"][name].append(args)

        # 2. Static Methods (Standalone def() inside .scope)
        if scope_pos != -1:
            scope_body = self.extract_bracket_block(block, scope_pos + len(".scope") - 1)
            if scope_body:
                # Note the lack of a dot before def
                for m in re.finditer(r'(?<!\.)def\s*\(\s*"([^"]+)"\s*,\s*([^,)]+)', scope_body):
                    name, sig = m.group(1), m.group(2)
                    args = self.extract_args(sig, cpp_class, name, lua_name)
                    cls["static"][name].append(args)

        # 3. Enum Parsing
        for enm in re.finditer(r'\.enum_\s*\(\s*"([^"]+)"\s*\)\s*\[', block):
            e_name = enm.group(1)
            e_body = self.extract_bracket_block(block, enm.end() - 1)
            if e_body:
                vals = dict(re.findall(r'value\s*\(\s*"([^"]+)"\s*,\s*([^)]+)\)', e_body))
                cls["enums"][e_name] = {k: v.strip().split(',')[-1].strip().rstrip(')]') for k, v in vals.items()}

    def apply_final_patches(self):
        for target in FORCE_STATIC_CLASSES:
            if target in self.classes:
                c = self.classes[target]
                c["static"].update(c["methods"])
                c["methods"] = defaultdict(list)

    def generate_xml(self, output_file: str):
        root = Element("LuaAPI")
        for name in sorted(self.classes):
            c = self.classes[name]
            ce = SubElement(root, "class", {"name": name, "cpp_name": c["cpp_name"]})
            
            if c.get("enums"):
                ee = SubElement(ce, "enums")
                for e_name, vals in sorted(c["enums"].items()):
                    en = SubElement(ee, "enum", {"name": e_name})
                    for k, v in sorted(vals.items()):
                        SubElement(en, "value", {"name": k, "value": v})

            for sec_key, sec_tag, type_val in [("methods", "methods", "instance"), ("static", "static_methods", "static")]:
                if c[sec_key]:
                    node = SubElement(ce, sec_tag)
                    for m_name, sigs in sorted(c[sec_key].items()):
                        for i, args in enumerate(sigs, 1):
                            attr = {"name": m_name, "type": type_val, "args": args}
                            if len(sigs) > 1: attr["overload"] = str(i)
                            SubElement(node, "method", attr)
        
        indent(root, space="  ")
        Path(output_file).parent.mkdir(parents=True, exist_ok=True)
        ElementTree(root).write(output_file, encoding="utf-8", xml_declaration=True)

if __name__ == "__main__":
    src_dir = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_SRC
    out_file = Path(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_OUT

    print("\n[*] Luabind Parser for CtrlrX")
    print(f"[*] Scanning: {src_dir.absolute()}")
    print("[*] Patch LuaAPI.xml file by editing and running lua_api_patcher.py")

    parser = LuabindParser()
    parser.index_source_files(src_dir)
    for f in src_dir.glob("**/*.cpp"):
        parser.parse_file(f)
    parser.apply_final_patches()
    parser.generate_xml(str(out_file))
    print(f"[*] Success: Found {len(parser.classes)} classes.")