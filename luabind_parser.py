#!/usr/bin/env python3
"""
Ctrlr Luabind Parser - With Function Signatures
Extracts argument types from explicit casts, constructors, and source file definitions
"""

import re
import sys
from pathlib import Path
from xml.etree.ElementTree import Element, SubElement, ElementTree, indent
from typing import Optional, Dict, Set

# ================= CONFIG =================
VERBOSE = True
STRIP_NAMESPACES = True

CLASS_OVERRIDES = {
    "CtrlrLuaUtils": {"name": "utils", "force_static": True},
    "CtrlrPanel": {"name": "panel", "force_static": False},
}

STRIP_L_PREFIX = ["MemoryBlock", "String", "File", "Rectangle", "Colour", "Graphics"]
# ==========================================

class LuabindParser:
    def __init__(self, source_dir: Path):
        self.classes = {}
        self.source_dir = source_dir
        self.cpp_cache = {}  # Cache for Source .cpp files
        self.juce_files = {}  # Cache for JUCE .cpp files (fallback)
        self.function_defs = {}  # Cache for parsed function definitions

    def load_cpp_files(self):
        """Pre-load all .cpp files for faster lookup"""
        if VERBOSE:
            print("[*] Pre-loading .cpp files for function lookup...")
        
        # Load Source files first (higher priority)
        cpp_files = list(self.source_dir.glob("**/*.cpp"))
        cpp_files_source = [f for f in cpp_files if "JUCE" not in str(f)]
        
        for f in cpp_files_source:
            try:
                self.cpp_cache[f.name] = f.read_text(encoding="utf-8", errors="ignore")
            except:
                pass
        
        if VERBOSE:
            print(f"[*] Loaded {len(self.cpp_cache)} Source .cpp files")
        
        # Store JUCE files separately for fallback lookup
        self.juce_files = {}
        cpp_files_juce = [f for f in cpp_files if "JUCE" in str(f)]
        
        for f in cpp_files_juce:
            try:
                self.juce_files[f.name] = f.read_text(encoding="utf-8", errors="ignore")
            except:
                pass
        
        if VERBOSE:
            print(f"[*] Loaded {len(self.juce_files)} JUCE .cpp files (fallback)")

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

    def extract_signature_from_cast(self, cast_str: str) -> Optional[str]:
        """
        Extract argument types from an explicit cast like:
        (bool (Range<int>::*)(const int) const) or
        (String (String::*)(int,int) const)
        Returns: "int" or "const int,String"
        """
        # Match: (returnType (ClassName::*)(args) modifiers)
        match = re.search(r'\(\s*\w+(?:<[^>]+>)?\s+\(\w+::\*\)\s*\(([^)]*)\)', cast_str)
        if match:
            args_str = match.group(1).strip()
            if args_str:
                # Clean up the argument types
                return self.simplify_types(args_str)
            return ""
        return None

    def extract_constructor_args(self, constructor_str: str) -> Optional[str]:
        """
        Extract argument types from constructor template like:
        constructor<int,int>() or constructor<const String &>()
        Returns: "int,int" or "string"
        """
        match = re.search(r'constructor\s*<\s*([^>]+)\s*>\s*\(\s*\)', constructor_str)
        if match:
            args_str = match.group(1).strip()
            if args_str:
                return self.simplify_types(args_str)
            return ""
        return None

    def simplify_types(self, type_str: str) -> str:
        """
        Convert C++ types to simpler names for Lua documentation
        const int -> int
        const String & -> string
        etc.
        """
        # Remove const, volatile, &, *
        simplified = re.sub(r'\b(const|volatile)\b\s*', '', type_str)
        simplified = re.sub(r'[&*\s]+$', '', simplified)  # Remove trailing &, *, spaces
        
        # Type mappings
        mappings = {
            'int': 'int',
            'bool': 'bool',
            'double': 'number',
            'float': 'number',
            'String': 'string',
            'StringRef': 'string',
            'CharPointer_UTF8': 'string',
            'int64': 'int',
            'uint8': 'int',
            'uint32': 'int',
            'var': 'variant',
            'MemoryBlock': 'MemoryBlock',
            'File': 'File',
            'Colour': 'Colour',
        }
        
        # Split by comma and process each type
        types = [t.strip() for t in simplified.split(',')]
        result = []
        for t in types:
            # Extract base type (handle templates)
            base = re.sub(r'<.*>', '', t).strip()
            result.append(mappings.get(base, base))
        
        return ','.join(result)

    def find_function_in_source(self, class_name: str, method_name: str) -> Optional[str]:
        """
        Search through source files for function definition and extract argument types
        Tries both the class name as-is and with/without 'L' prefix
        First searches Source files, then falls back to JUCE if not found
        """
        # Build list of class name variations to try
        class_names_to_try = [class_name]
        
        # If class starts with 'L', also try without it (e.g., LMemoryBlock -> MemoryBlock)
        if class_name.startswith('L') and len(class_name) > 1:
            class_names_to_try.append(class_name[1:])
        
        # If class doesn't start with 'L', also try with 'L' (e.g., MemoryBlock -> LMemoryBlock)
        if not class_name.startswith('L'):
            class_names_to_try.append('L' + class_name)
        
        # PASS 1: Try to find in Source files (higher priority)
        for class_to_search in class_names_to_try:
            for content in self.cpp_cache.values():
                pattern = rf'(?:^|\s+)\w+(?:<[^>]+>)?\s+{re.escape(class_to_search)}::{re.escape(method_name)}\s*\(\s*([^){{]*)\s*\)'
                match = re.search(pattern, content, re.MULTILINE)
                if match:
                    args_str = match.group(1).strip()
                    if args_str:
                        return self.simplify_types(args_str)
                    return ""
        
        # PASS 2: Fall back to JUCE files if not found in Source
        if hasattr(self, 'juce_files'):
            for class_to_search in class_names_to_try:
                for content in self.juce_files.values():
                    pattern = rf'(?:^|\s+)\w+(?:<[^>]+>)?\s+{re.escape(class_to_search)}::{re.escape(method_name)}\s*\(\s*([^){{]*)\s*\)'
                    match = re.search(pattern, content, re.MULTILINE)
                    if match:
                        args_str = match.group(1).strip()
                        if args_str:
                            return self.simplify_types(args_str)
                        return ""
        
        return None

    def extract_method_spec(self, text: str, start: int) -> tuple:
        """
        Extract method name and full spec from .def("name", spec)
        Handles nested parentheses correctly
        Returns: (method_name, method_spec, end_position)
        """
        # Find the opening "
        quote_start = text.find('"', start)
        if quote_start == -1:
            return (None, None, -1)
        
        # Find closing "
        quote_end = text.find('"', quote_start + 1)
        if quote_end == -1:
            return (None, None, -1)
        
        method_name = text[quote_start + 1:quote_end]
        
        # Find the comma after the method name
        comma_pos = text.find(',', quote_end)
        if comma_pos == -1:
            return (None, None, -1)
        
        # Now extract the spec with proper parenthesis matching
        spec_start = comma_pos + 1
        while spec_start < len(text) and text[spec_start] in ' \t':
            spec_start += 1
        
        # Match parentheses/brackets to find the complete spec
        paren_depth = 0
        angle_depth = 0
        spec_end = spec_start
        
        for i in range(spec_start, len(text)):
            c = text[i]
            
            if c == '<':
                angle_depth += 1
            elif c == '>':
                angle_depth -= 1
            elif c == '(' and angle_depth == 0:
                paren_depth += 1
            elif c == ')' and angle_depth == 0:
                paren_depth -= 1
                if paren_depth < 0:  # Found the closing paren of .def()
                    spec_end = i
                    break
        
        method_spec = text[spec_start:spec_end].strip()
        return (method_name, method_spec, spec_end)

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
            "methods": {},  # Changed to dict: name -> {type, args}
            "static": {},   # Changed to dict
            "enums": {},
            "force_static": force_static
        })

        body = block[header.end():]
        
        # Parse instance methods with proper paren matching
        for m in re.finditer(r'\.def\s*\(', body):
            method_name, method_spec, _ = self.extract_method_spec(body, m.start())
            if method_name and method_spec:
                args = self.extract_args_from_def(method_spec, cpp_class, method_name)
                cls["methods"][method_name] = {"type": "instance", "args": args}

        # Parse constructors
        for m in re.finditer(r'\.def\s*\(\s*constructor\s*<\s*([^>]*)\s*>\s*\(\s*\)', body):
            ctor_args = m.group(1).strip()
            if ctor_args:
                args = self.simplify_types(ctor_args)
            else:
                args = ""
            cls["methods"]["__init"] = {"type": "constructor", "args": args}

        # Parse static methods
        for scope in re.finditer(r'\.scope\s*\[', body):
            scope_body = self.extract_bracket_block(body, scope.end() - 1)
            if scope_body:
                for m in re.finditer(r'def\s*\(', scope_body):
                    method_name, method_spec, _ = self.extract_method_spec(scope_body, m.start())
                    if method_name and method_spec:
                        args = self.extract_args_from_def(method_spec, cpp_class, method_name)
                        cls["static"][method_name] = {"type": "static", "args": args}

    def extract_args_from_def(self, method_spec: str, cpp_class: str, method_name: str) -> str:
        """
        Extract argument types from a .def() specification.
        Handles:
          - Explicit casts: (bool (Class::*)(int, const String &) const)
          - Plain pointer: &ClassName::methodName
        """
        method_spec = method_spec.strip()
        
        # Try explicit cast first
        if '::*' in method_spec:  # Has explicit cast
            args = self.extract_signature_from_cast(method_spec)
            if args is not None:
                return args
        
        # Fall back to searching source files
        args = self.find_function_in_source(cpp_class, method_name)
        return args if args is not None else ""

    def strip_namespace(self, name: str) -> str:
        return name.split("::")[-1].strip() if STRIP_NAMESPACES else name.strip()

    def generate_xml(self, output_file: str):
        root = Element("LuaAPI")
        for name in sorted(self.classes):
            cls = self.classes[name]
            ce = SubElement(root, "class", {"name": name, "cpp_name": cls["cpp_name"]})

            force_all_static = cls["force_static"]

            if cls["methods"]:
                node_name = "static_methods" if force_all_static else "methods"
                me = SubElement(ce, node_name)
                for method_name in sorted(cls["methods"]):
                    m_info = cls["methods"][method_name]
                    attrs = {"name": method_name, "type": m_info["type"]}
                    if m_info["args"]:
                        attrs["args"] = f"({m_info['args']})"
                    SubElement(me, "method", attrs)

            if cls["static"]:
                se = SubElement(ce, "static_methods")
                for method_name in sorted(cls["static"]):
                    m_info = cls["static"][method_name]
                    attrs = {"name": method_name, "type": m_info["type"]}
                    if m_info["args"]:
                        attrs["args"] = f"({m_info['args']})"
                    SubElement(se, "method", attrs)

        indent(root, space="  ")
        ElementTree(root).write(output_file, encoding="utf-8", xml_declaration=True)
        print(f"\nGenerated: {output_file}\nClasses: {len(self.classes)}")

def main():
    if len(sys.argv) != 3:
        print("Usage: python luabind_parser.py <source_dir> <output.xml>")
        sys.exit(1)
    
    source_path = Path(sys.argv[1])
    parser = LuabindParser(source_path)
    parser.load_cpp_files()
    
    cpp_files = sorted(source_path.glob("**/*.cpp"))
    print(f"Found {len(cpp_files)} .cpp files total.")
    
    for f in cpp_files:
        if "wrapForLua" in f.read_text(encoding="utf-8", errors="ignore"):
            print(f"Parsing: {f.name}")
            parser.parse_file(f)
    
    parser.generate_xml(sys.argv[2])

if __name__ == "__main__":
    main()