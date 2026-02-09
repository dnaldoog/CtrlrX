#!/usr/bin/env python3
import re
import sys
import html
from pathlib import Path
from collections import defaultdict

# ------------------------------------------------------------
# CONFIG
# ------------------------------------------------------------

DEFAULT_SRC = Path(".")
DEFAULT_OUT = Path("lua_api.xml")

# Classes that should ALWAYS behave as static utility tables
FORCE_STATIC_CLASSES = {
    "CtrlrLuaUtils",
}

# ------------------------------------------------------------
# PARSER
# ------------------------------------------------------------

class LuabindParserV3:
    def __init__(self):
        # name -> { cpp, methods, static, enums }
        self.classes = {}
        self.signatures = {}          # method -> args
        self.inheritance = {}         # cpp -> parent

    # --------------------------------------------------------
    # HEADER PASS (signatures, inheritance, enums)
    # --------------------------------------------------------

    def index_cpp_headers(self, root: Path):
        for path in root.rglob("*.[hH]*"):
            try:
                text = path.read_text(encoding="utf-8", errors="ignore")
            except Exception:
                continue

            # Inheritance
            for m in re.finditer(
                r"class\s+(\w+)\s*:\s*(?:public|protected|private)\s+([\w:]+)",
                text,
            ):
                self.inheritance[m.group(1)] = m.group(2).split("::")[-1]

            # Enums
            for enum in re.finditer(
                r"enum\s+(?:class\s+)?(\w+)\s*{([^}]+)}",
                text,
                re.S,
            ):
                name = enum.group(1)
                values = [
                    v.strip().split("=")[0]
                    for v in enum.group(2).split(",")
                    if v.strip()
                ]
                self.signatures[f"enum::{name}"] = {
                    "type": "enum",
                    "values": values,
                }

            # Function signatures
            for fn in re.finditer(
                r"(static\s+)?[\w:<>&*]+\s+(\w+)\s*\(([^)]*)\)",
                text,
            ):
                is_static, name, args = fn.groups()
                if name in ("if", "for", "while", "switch"):
                    continue

                clean = self._clean_args(args)
                self.signatures[name] = {
                    "args": clean,
                    "type": "static" if is_static else "instance",
                }

    # --------------------------------------------------------
    # CPP PASS (luabind / sol2 bindings)
    # --------------------------------------------------------

    def parse_cpp_files(self, root: Path):
        for path in root.rglob("*.cpp"):
            try:
                text = path.read_text(encoding="utf-8", errors="ignore")
            except Exception:
                continue

            for cls in re.finditer(
                r"class_<\s*([^,>]+).*?\(\s*\"([^\"]+)\"",
                text,
            ):
                cpp = cls.group(1).split("::")[-1]
                lua = cls.group(2)

                self.classes.setdefault(
                    lua,
                    {
                        "cpp": cpp,
                        "methods": defaultdict(list),
                        "static": defaultdict(list),
                        "enums": {},
                    },
                )

                block_end = text.find(";", cls.end())
                block = text[cls.end() : block_end]

                for d in re.finditer(
                    r"\.(def|def_static)\s*\(\s*\"([^\"]+)\"",
                    block,
                ):
                    kind, name = d.groups()
                    sig = self.signatures.get(
                        name,
                        {"args": "()", "type": "static" if kind == "def_static" else "instance"},
                    )

                    target = "static" if kind == "def_static" else "methods"
                    self.classes[lua][target][name].append(sig)

    # --------------------------------------------------------
    # STATIC PROMOTION
    # --------------------------------------------------------

    def promote_static(self):
        for cls in self.classes.values():
            for name, sigs in list(cls["methods"].items()):
                if all(s.get("type") == "static" for s in sigs):
                    cls["static"][name].extend(sigs)
                    del cls["methods"][name]

    def force_static_classes(self):
        for lua, cls in self.classes.items():
            if lua in FORCE_STATIC_CLASSES or cls["cpp"] in FORCE_STATIC_CLASSES:
                for name, sigs in cls["methods"].items():
                    cls["static"][name].extend(sigs)
                cls["methods"].clear()

    # --------------------------------------------------------
    # XML OUTPUT
    # --------------------------------------------------------

    def write_xml(self, out: Path):
        xml = ['<?xml version="1.0" encoding="UTF-8"?>', "<LuaAPI>"]

        for lua in sorted(self.classes):
            cls = self.classes[lua]
            parent = (
                f' inherits="{self.inheritance[cls["cpp"]]}"'
                if cls["cpp"] in self.inheritance
                else ""
            )

            xml.append(f'  <class name="{lua}" cpp_name="{cls["cpp"]}"{parent}>')

            if cls["methods"]:
                xml.append("    <methods>")
                for name, sigs in cls["methods"].items():
                    for sig in sigs:
                        xml.append(
                            f'      <method name="{name}" type="instance" args="{sig["args"]}"/>'
                        )
                xml.append("    </methods>")

            if cls["static"]:
                xml.append("    <static_methods>")
                for name, sigs in cls["static"].items():
                    for sig in sigs:
                        xml.append(
                            f'      <method name="{name}" type="static" args="{sig["args"]}"/>'
                        )
                xml.append("    </static_methods>")

            for k, v in self.signatures.items():
                if k.startswith("enum::"):
                    xml.append("    <enums>")
                    xml.append(f'      <enum name="{k.split("::")[1]}">')
                    for val in v["values"]:
                        xml.append(f'        <value name="{val}"/>')
                    xml.append("      </enum>")
                    xml.append("    </enums>")

            xml.append("  </class>")

        xml.append("</LuaAPI>")
        out.write_text("\n".join(xml), encoding="utf-8")

    # --------------------------------------------------------

    @staticmethod
    def _clean_args(args: str) -> str:
        if not args or args.strip() in ("void",):
            return "()"
        a = re.sub(r'=[^,)]+', '', args)
        a = re.sub(r'\b(const|juce::|std::|&|\*|override|final|noexcept)\b', '', a)
        return f"({html.escape(' '.join(a.split()).strip())})"


# ------------------------------------------------------------
# MAIN
# ------------------------------------------------------------

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python luabind_parser.py <source_dir> <output_xml>")
        print("Using defaults.")

    src = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_SRC
    out = Path(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_OUT

    parser = LuabindParserV3()
    parser.index_cpp_headers(src)
    parser.parse_cpp_files(src)
    parser.promote_static()
    parser.force_static_classes()
    parser.write_xml(out)

    print(f"[OK] Lua API XML generated → {out}")
