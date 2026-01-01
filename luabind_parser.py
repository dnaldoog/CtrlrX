#!/usr/bin/env python3
"""
Ctrlr Luabind Parser - Optimized for Autocomplete
"""

import re
import sys
from pathlib import Path
from xml.etree.ElementTree import Element, SubElement, ElementTree, indent

# ================= CONFIG =================
VERBOSE = True
STRIP_NAMESPACES = True

# Map specifically known "Table" style globals to their friendly names
# and force them to be treated as static (dot access only)
CLASS_OVERRIDES = {
    "CtrlrLuaUtils": {"name": "utils", "force_static": True},
    "CtrlrPanel": {"name": "panel", "force_static": False},
}

# Common classes that should have their "L" prefix stripped
STRIP_L_PREFIX = ["MemoryBlock", "String", "File", "Rectangle", "Colour", "Graphics"]
# ==========================================

class LuabindParser:
    def __init__(self):
        self.classes = {}

    def parse_file(self, filepath: Path):
        content = filepath.read_text(encoding="utf-8", errors="ignore")
        wrap_pattern = r'wrapForLua\s*\([^)]*lua_State\s*\*[^)]*\)'
        wraps = list(re.finditer(wrap_pattern, content))

        if not wraps: return

        if VERBOSE:
            print(f"\nFILE: {filepath.name} ({len(wraps)} wrapForLua)")

        for wrap in wraps:
            self.parse_wrap_block(content, wrap.start())

    def extract_bracket_block(self, text: str, start: int):
        depth = 0
        in_string = False
        escape = False
        for i in range(start, len(text)):
            c = text[i]
            if escape: escape = False; continue
            if c == '\\': escape = True; continue
            if c == '"': in_string = not in_string; continue
            if in_string: continue
            if c == '[': depth += 1
            elif c == ']':
                depth -= 1
                if depth == 0: return text[start + 1:i]
        return None

    def parse_wrap_block(self, content: str, start_pos: int):
        module_match = re.search(r'(?:luabind::)?module\s*\(\s*L\s*\)', content[start_pos:])
        if not module_match: return
        
        module_start = start_pos + module_match.end()
        bracket_start = content.find('[', module_start)
        if bracket_start == -1: return

        module_body = self.extract_bracket_block(content, bracket_start)
        if module_body: self.parse_module(module_body)

    def parse_module(self, content: str):
        content = re.sub(r'//.*', '', content)
        content = re.sub(r'/\*.*?\*/', '', content, flags=re.S)
        class_iter = re.finditer(r'class_<\s*([^,>]+)', content)
        positions = [m.start() for m in class_iter]

        for idx, pos in enumerate(positions):
            end = positions[idx + 1] if idx + 1 < len(positions) else len(content)
            self.parse_class_block(content[pos:end])

    def parse_class_block(self, block: str):
        header = re.search(r'class_<\s*([^,>]+)[^>]*>\s*(\(\s*"([^"]+)"\s*\))?', block)
        if not header: return

        cpp_class = self.strip_namespace(header.group(1))
        lua_name = header.group(3) or cpp_class

        # --- Transform Names ---
        final_name = lua_name
        force_static = False

        if lua_name in CLASS_OVERRIDES:
            final_name = CLASS_OVERRIDES[lua_name]["name"]
            force_static = CLASS_OVERRIDES[lua_name]["force_static"]
        elif lua_name.startswith('L') and lua_name[1:] in STRIP_L_PREFIX:
            final_name = lua_name[1:]

        if VERBOSE and final_name != lua_name:
            print(f"   Mapping: {lua_name} -> {final_name}")

        cls = self.classes.setdefault(final_name, {
            "cpp_name": cpp_class,
            "methods": set(),
            "static": set(),
            "enums": {},
            "force_static": force_static
        })

        body = block[header.end():]
        for m in re.finditer(r'\.def\s*\(\s*"([^"]+)"', body):
            cls["methods"].add(m.group(1))

        for scope in re.finditer(r'\.scope\s*\[', body):
            scope_body = self.extract_bracket_block(body, scope.end() - 1)
            if scope_body:
                for m in re.finditer(r'def\s*\(\s*"([^"]+)"', scope_body):
                    cls["static"].add(m.group(1))

    def strip_namespace(self, name: str) -> str:
        return name.split("::")[-1].strip() if STRIP_NAMESPACES else name.strip()

    def generate_xml(self, output_file: str):
        root = Element("LuaAPI")
        for name in sorted(self.classes):
            cls = self.classes[name]
            ce = SubElement(root, "class", {"name": name, "cpp_name": cls["cpp_name"]})

            force_all_static = cls["force_static"]

            if cls["methods"]:
                # If force_static (like utils), move all .def methods to static_methods
                node_name = "static_methods" if force_all_static else "methods"
                m_type = "static" if force_all_static else "instance"
                me = SubElement(ce, node_name)
                for m in sorted(cls["methods"]):
                    SubElement(me, "method", {"name": m, "type": m_type})

            if cls["static"]:
                se = SubElement(ce, "static_methods")
                for m in sorted(cls["static"]):
                    SubElement(se, "method", {"name": m, "type": "static"})

        indent(root, space="  ")
        ElementTree(root).write(output_file, encoding="utf-8", xml_declaration=True)
        print(f"\nGenerated: {output_file}\nClasses: {len(self.classes)}")

def main():
    if len(sys.argv) != 3:
        print("Usage: python luabind_parser.py <source_dir> <output.xml>")
        sys.exit(1)
    parser = LuabindParser()
    cpp_files = sorted(Path(sys.argv[1]).glob("**/*.cpp"))
    print(f"Found {len(cpp_files)} .cpp files total.") # DEBUG LINE 1
    for f in cpp_files:
        if "wrapForLua" in f.read_text(encoding="utf-8", errors="ignore"):
            print(f"Parsing: {f.name}") # DEBUG LINE 2
            parser.parse_file(f)
    parser.generate_xml(sys.argv[2])

if __name__ == "__main__":
    main()