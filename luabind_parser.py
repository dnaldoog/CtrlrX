#!/usr/bin/env python3
"""
Ctrlr Luabind Parser - Enhanced with Overload Detection
Extracts Lua API from Ctrlr / JUCE luabind C++ bindings

Usage:
  python luabind_parser.py <source_directory> <output_xml>
"""

import re
import sys
from pathlib import Path
from xml.etree.ElementTree import Element, SubElement, ElementTree, indent
from collections import defaultdict

# ================= CONFIG =================

VERBOSE = True
STRIP_NAMESPACES = True
DEBUG = False  # Set to True to see detailed parsing info

# ==========================================


def strip_namespace(name: str) -> str:
    if not STRIP_NAMESPACES:
        return name.strip()
    return name.split("::")[-1].strip()


def extract_args_from_signature(signature: str) -> str:
    """
    Extract arguments from C++ function signature.
    Examples:
      void(CtrlrModulator::*)(int, bool) -> "(int, bool)"
      void(CtrlrPanel::*)(const String&) -> "(String)"
    """
    # Match function pointer signature
    match = re.search(r'\([^)]*\)\s*\(([^)]*)\)', signature)
    if match:
        args = match.group(1).strip()
        if not args:
            return "()"
        
        # Clean up the args
        clean_args = []
        for arg in args.split(','):
            arg = arg.strip()
            # Remove const, &, *, etc.
            arg = re.sub(r'\bconst\b', '', arg)
            arg = re.sub(r'[&*]', '', arg)
            # Get type (remove parameter names)
            parts = arg.split()
            if parts:
                clean_args.append(parts[0])
        
        return f"({', '.join(clean_args)})"
    
    return ""


def file_contains_lua_bindings(filepath: Path) -> bool:
    """Quick check without full file read."""
    try:
        # Read only first 2KB for quick check
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            chunk = f.read(2048)
            # Check for multiple indicators of Lua bindings
            return ("wrapForLua" in chunk or 
                    "module(L)" in chunk or 
                    "class_<" in chunk or
                    "luabind::" in chunk)
    except Exception:
        return False


class LuabindParser:
    def __init__(self):
        # Changed to store list of methods to support overloads
        self.classes = {}

    # -------------------------------------------------------------

    def parse_file(self, filepath: Path):
        content = filepath.read_text(encoding="utf-8", errors="ignore")

        # Look for wrapForLua functions
        wrap_pattern = r'wrapForLua\s*\([^)]*lua_State\s*\*[^)]*\)'
        wraps = list(re.finditer(wrap_pattern, content))

        # Look for top-level module(L) blocks
        module_pattern = r'(?:luabind::)?module\s*\(\s*L\s*\)\s*\['
        modules = list(re.finditer(module_pattern, content))

        if not wraps and not modules:
            return

        if VERBOSE:
            if wraps:
                print(f"\nFILE: {filepath.name} ({len(wraps)} wrapForLua)")
            if modules and not wraps:
                print(f"\nFILE: {filepath.name} ({len(modules)} direct module blocks)")

        # Parse wrapForLua functions
        for wrap in wraps:
            self.parse_wrap_block(content, wrap.start())
        
        # Parse direct module(L) blocks (not inside wrapForLua)
        if modules and not wraps:
            for module_match in modules:
                bracket_start = module_match.end() - 1
                module_body = self.extract_bracket_block(content, bracket_start)
                if module_body:
                    if DEBUG:
                        print(f"  Parsing module block, length: {len(module_body)}")
                    self.parse_module(module_body)
                elif DEBUG:
                    print(f"  WARNING: Could not extract module block at position {bracket_start}")

    # -------------------------------------------------------------

    def parse_wrap_block(self, content: str, start_pos: int):
        module_match = re.search(
            r'(?:luabind::)?module\s*\(\s*L\s*\)',
            content[start_pos:]
        )

        if not module_match:
            return

        module_start = start_pos + module_match.end()
        bracket_start = content.find('[', module_start)

        if bracket_start == -1:
            return

        module_body = self.extract_bracket_block(content, bracket_start)

        if module_body:
            self.parse_module(module_body)

    # -------------------------------------------------------------

    def extract_bracket_block(self, text: str, start: int):
        depth = 0
        in_string = False
        escape = False

        for i in range(start, len(text)):
            c = text[i]

            if escape:
                escape = False
                continue

            if c == '\\':
                escape = True
                continue

            if c == '"':
                in_string = not in_string
                continue

            if in_string:
                continue

            if c == '[':
                depth += 1
            elif c == ']':
                depth -= 1
                if depth == 0:
                    return text[start + 1:i]

        return None

    # -------------------------------------------------------------

    def parse_module(self, content: str):
        content = re.sub(r'//.*', '', content)
        content = re.sub(r'/\*.*?\*/', '', content, flags=re.S)

        class_iter = re.finditer(r'class_<\s*([^,>]+)', content)

        positions = [m.start() for m in class_iter]

        for idx, pos in enumerate(positions):
            end = positions[idx + 1] if idx + 1 < len(positions) else len(content)
            self.parse_class_block(content[pos:end])

    # -------------------------------------------------------------

    def parse_class_block(self, block: str):
        header = re.search(
            r'class_<\s*([^,>]+)[^>]*>\s*(\(\s*"([^"]+)"\s*\))?',
            block
        )

        if not header:
            return

        cpp_class = strip_namespace(header.group(1))
        lua_name = header.group(3) or cpp_class

        if VERBOSE:
            print(f"  Class: {lua_name} (C++: {cpp_class})")

        cls = self.classes.setdefault(lua_name, {
            "cpp_name": cpp_class,
            "methods": defaultdict(list),  # name -> list of signatures
            "static": defaultdict(list),
            "constructors": [],
            "enums": {}
        })

        body = block[header.end():]

        # Constructors
        for ctor in re.finditer(r'\.def\s*\(\s*constructor\s*<([^>]*)>', body):
            args = ctor.group(1).strip()
            if args:
                cls["constructors"].append(f"({args})")
            else:
                cls["constructors"].append("()")

        # Instance methods - capture full .def() call
        for m in re.finditer(r'\.def\s*\(\s*"([^"]+)"\s*,\s*([^)]+)\)', body):
            method_name = m.group(1)
            signature = m.group(2)
            
            args = extract_args_from_signature(signature)
            cls["methods"][method_name].append(args)

        # Static methods (.scope)
        for scope in re.finditer(r'\.scope\s*\[', body):
            scope_body = self.extract_bracket_block(body, scope.end() - 1)
            if not scope_body:
                continue

            for m in re.finditer(r'def\s*\(\s*"([^"]+)"\s*,\s*([^)]+)\)', scope_body):
                method_name = m.group(1)
                signature = m.group(2)
                
                args = extract_args_from_signature(signature)
                cls["static"][method_name].append(args)

        # Enums
        for enum in re.finditer(r'\.enum_\s*\(\s*"([^"]+)"\s*\)\s*\[', body):
            enum_name = enum.group(1)
            enum_body = self.extract_bracket_block(body, enum.end() - 1)

            if not enum_body:
                continue

            values = {}
            for v in re.finditer(
                r'value\s*\(\s*"([^"]+)"\s*,\s*([^)]+)\)',
                enum_body
            ):
                values[v.group(1)] = v.group(2).strip()

            if values:
                cls["enums"][enum_name] = values

    # -------------------------------------------------------------

    def generate_xml(self, output_file: str):
        root = Element("LuaAPI")

        for name in sorted(self.classes):
            cls = self.classes[name]

            ce = SubElement(root, "class", {
                "name": name,
                "cpp_name": cls["cpp_name"]
            })

            # Constructors
            if cls["constructors"]:
                for ctor_args in cls["constructors"]:
                    SubElement(ce, "constructor", {
                        "args": ctor_args
                    })

            # Instance methods
            if cls["methods"]:
                me = SubElement(ce, "methods")
                for method_name in sorted(cls["methods"]):
                    signatures = cls["methods"][method_name]
                    
                    if len(signatures) == 1:
                        # Single overload
                        SubElement(me, "method", {
                            "name": method_name,
                            "type": "instance",
                            "args": signatures[0] or ""
                        })
                    else:
                        # Multiple overloads
                        for idx, args in enumerate(signatures, 1):
                            SubElement(me, "method", {
                                "name": method_name,
                                "type": "instance",
                                "args": args or "",
                                "overload": str(idx)
                            })

            # Static methods
            if cls["static"]:
                se = SubElement(ce, "static_methods")
                for method_name in sorted(cls["static"]):
                    signatures = cls["static"][method_name]
                    
                    if len(signatures) == 1:
                        SubElement(se, "method", {
                            "name": method_name,
                            "type": "static",
                            "args": signatures[0] or ""
                        })
                    else:
                        for idx, args in enumerate(signatures, 1):
                            SubElement(se, "method", {
                                "name": method_name,
                                "type": "static",
                                "args": args or "",
                                "overload": str(idx)
                            })

            # Enums
            if cls["enums"]:
                ee = SubElement(ce, "enums")
                for ename, values in cls["enums"].items():
                    e = SubElement(ee, "enum", {"name": ename})
                    for vname, vval in values.items():
                        SubElement(e, "value", {
                            "name": vname,
                            "value": vval
                        })

        indent(root, space="  ")
        ElementTree(root).write(
            output_file,
            encoding="utf-8",
            xml_declaration=True
        )

        print("\nGenerated:", output_file)
        print("Classes:", len(self.classes))


# ================================================================

def main():
    if len(sys.argv) != 3:
        print("Usage: python luabind_parser.py <source_dir> <output.xml>")
        sys.exit(1)

    source_dir = Path(sys.argv[1])
    output_xml = sys.argv[2]

    parser = LuabindParser()

    # Optimized: filter files first
    cpp_files = [f for f in sorted(source_dir.glob("**/*.cpp")) 
                 if file_contains_lua_bindings(f)]

    print(f"Found {len(cpp_files)} files with Lua bindings\n")

    for f in cpp_files:
        parser.parse_file(f)

    parser.generate_xml(output_xml)


if __name__ == "__main__":
    main()