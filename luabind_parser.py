#!/usr/bin/env python3
"""
Ctrlr Luabind Parser
Extracts Lua API from Ctrlr / JUCE luabind C++ bindings

Usage:
  python luabind_parser.py <source_directory> <output_xml>
"""

import re
import sys
from pathlib import Path
from xml.etree.ElementTree import Element, SubElement, ElementTree, indent

# ================= CONFIG =================

VERBOSE = True
STRIP_NAMESPACES = True

# ==========================================


def strip_namespace(name: str) -> str:
    if not STRIP_NAMESPACES:
        return name.strip()
    return name.split("::")[-1].strip()


def file_contains_wrap(filepath: Path) -> bool:
    try:
        return "wrapForLua" in filepath.read_text(
            encoding="utf-8", errors="ignore"
        )
    except Exception:
        return False


class LuabindParser:
    def __init__(self):
        self.classes = {}

    # -------------------------------------------------------------

    def parse_file(self, filepath: Path):
        content = filepath.read_text(encoding="utf-8", errors="ignore")

        wrap_pattern = (
            r'wrapForLua\s*'
            r'\(\s*lua_State\s*\*\s*L\s*\)'
        )

        wraps = list(re.finditer(wrap_pattern, content))
        if not wraps:
            return

        if VERBOSE:
            print(f"\nFILE: {filepath.name} ({len(wraps)} wrapForLua)")

        for wrap in wraps:
            self.parse_wrap_block(content, wrap.start())

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
        if not module_body:
            return

        self.parse_module(module_body)

    # -------------------------------------------------------------

    def extract_bracket_block(self, text: str, start: int) -> str | None:
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
                    return text[start + 1 : i]

        return None

    # -------------------------------------------------------------

    def parse_module(self, content: str):
        # Remove comments
        content = re.sub(r'//.*', '', content)
        content = re.sub(r'/\*.*?\*/', '', content, flags=re.S)

        class_iter = re.finditer(r'class_<([^>]+)>', content)
        positions = [m.start() for m in class_iter]

        for idx, pos in enumerate(positions):
            end = positions[idx + 1] if idx + 1 < len(positions) else len(content)
            self.parse_class_block(content[pos:end])

    # -------------------------------------------------------------

    def parse_class_block(self, block: str):
        header = re.search(
            r'class_<([^>]+)>\s*(\(\s*"([^"]+)"\s*\))?',
            block
        )
        if not header:
            return

        template = header.group(1)
        lua_name = header.group(3)

        cpp_class = template.split(',')[0].strip()
        cpp_class = strip_namespace(cpp_class)
        lua_name = lua_name or cpp_class

        if VERBOSE:
            print(f"  Class: {lua_name} (C++: {cpp_class})")

        cls = self.classes.setdefault(lua_name, {
            "cpp_name": cpp_class,
            "methods": set(),
            "static": set(),
            "enums": {}
        })

        body = block[header.end():]


        # Instance methods
        for m in re.finditer(r'\.def\s*\(\s*"([^"]+)"', body):
            cls["methods"].add(m.group(1))

        # Static methods (.scope)
        for scope in re.finditer(r'\.scope\s*\[', body):
            scope_body = self.extract_bracket_block(body, scope.end() - 1)
            if not scope_body:
                continue
            for m in re.finditer(r'def\s*\(\s*"([^"]+)"', scope_body):
                cls["static"].add(m.group(1))

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

    def extract_until_top_comma(self, text: str) -> str:
        depth = 0
        in_string = False
        escape = False
        result = []

        for c in text:
            if escape:
                escape = False
                result.append(c)
                continue
            if c == '\\':
                escape = True
                result.append(c)
                continue
            if c == '"':
                in_string = not in_string
                result.append(c)
                continue

            if in_string:
                result.append(c)
                continue

            if c in '([<':
                depth += 1
            elif c in ')]>':
                depth -= 1
            elif c == ',' and depth == 0:
                break

            result.append(c)

        return ''.join(result)

    # -------------------------------------------------------------

    def generate_xml(self, output_file: str):
        root = Element("LuaAPI")

        for name in sorted(self.classes):
            cls = self.classes[name]
            ce = SubElement(root, "class", {
                "name": name,
                "cpp_name": cls["cpp_name"]
            })

            if cls["methods"]:
                me = SubElement(ce, "methods")
                for m in sorted(cls["methods"]):
                    SubElement(me, "method", {
                        "name": m,
                        "type": "instance"
                    })

            if cls["static"]:
                se = SubElement(ce, "static_methods")
                for m in sorted(cls["static"]):
                    SubElement(se, "method", {
                        "name": m,
                        "type": "static"
                    })

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

    cpp_files = list(source_dir.glob("**/*.cpp"))
    for f in sorted(cpp_files):
        if not file_contains_wrap(f):
            continue
        parser.parse_file(f)

    parser.generate_xml(output_xml)


if __name__ == "__main__":
    main()
