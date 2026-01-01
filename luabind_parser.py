import re
import os
import sys
import html

class LuaAPIGenerator:
    def __init__(self):
        # 1. STRICT C++ Class detection
        self.class_re = re.compile(r'^\s*(?:class|struct)\s+([A-Za-z0-9_]+)\s*(?::\s*(?:public|protected|private)\s+[^{]+)?\s*\{', re.MULTILINE)
        
        # 2. STRICT Method detection (matches signatures and ignores common code blocks)
        self.method_re = re.compile(r'^\s*((?:static|virtual|inline)\s+)?(?:[\w<>\d::*&]+\s+)?(\w+)\s*\(([^)]*)\)\s*(?:const|override|final|noexcept)*\s*[;{]', re.MULTILINE)

        # Folders to completely ignore to keep the XML size down
        self.BLACKLIST_DIRS = {
            'Boost', 'boost', 'JuceLibraryCode', 'SDKs', 'VST3_SDK', 
            '.git', 'Builds', 'BinaryData', 'Documentation', 'ThirdParty', 'libs'
        }

        # C++ Keywords that the parser might mistake for methods
        self.BLACKLIST_METHODS = {
            'if', 'for', 'while', 'return', 'switch', 'catch', 'using', 
            'decltype', 'sizeof', 'static_assert', 'const_cast', 'dynamic_cast',
            'reinterpret_cast', 'template', 'typedef', 'break', 'continue'
        }

        self.STRIP_L_PREFIX = ["MemoryBlock", "String", "File", "Rectangle", "Colour", "Graphics", "BigInteger"]
        
        self.CLASS_OVERRIDES = {
            "CtrlrLuaUtils": {"name": "utils", "force_static": True},
            "CtrlrPanel":    {"name": "panel", "force_static": False},
            "CtrlrModulator":{"name": "modulator", "force_static": False},
        }

    def clean_args(self, args):
        if not args or args.strip().lower() in ["void", "nullptr", "0"]: 
            return ""
        # Remove C++ default values, comments, and noise
        a = re.sub(r'=[^,)]+', '', args)
        a = re.sub(r'/\*.*?\*/|//.*', '', a)
        a = re.sub(r'juce::|std::|const|&|\*|override|final|virtual|inline|noexcept', '', a)
        # Normalize spaces and escape for XML safety
        clean = ' '.join(a.split()).strip().strip(',')
        return html.escape(clean)

    def process_directory(self, input_path):
        all_classes = {}
        print(f"Scanning directory: {input_path}")
        
        for root, dirs, files in os.walk(input_path):
            # Skip blacklisted directories
            dirs[:] = [d for d in dirs if d not in self.BLACKLIST_DIRS]
            
            # Check if any part of the path is blacklisted
            path_parts = set(re.split(r'[\\/]', root))
            if any(b in path_parts for b in self.BLACKLIST_DIRS):
                continue

            for file in files:
                if file.endswith((".h", ".cpp", ".hpp")):
                    try:
                        with open(os.path.join(root, file), 'r', encoding='utf-8', errors='ignore') as f:
                            content = f.read()
                            for class_match in self.class_re.finditer(content):
                                c_name = class_match.group(1)
                                
                                # Skip macro-based classes or very short noise
                                if c_name.isupper() or len(c_name) < 3: continue
                                
                                start = class_match.end()
                                next_class = self.class_re.search(content, start)
                                end = next_class.start() if next_class else len(content)
                                class_block = content[start:end]
                                
                                methods = []
                                for m_match in self.method_re.finditer(class_block):
                                    m_name = m_match.group(2)
                                    
                                    # Skip destructors and C++ keywords
                                    if m_name.startswith('~') or m_name in self.BLACKLIST_METHODS:
                                        continue
                                        
                                    is_static = "static" in (m_match.group(1) or "")
                                    m_args = self.clean_args(m_match.group(3))
                                    methods.append({'name': m_name, 'args': m_args, 'is_static': is_static})
                                
                                if methods:
                                    if c_name not in all_classes: all_classes[c_name] = []
                                    all_classes[c_name].extend(methods)
                    except Exception: continue
        return all_classes

    def save_xml(self, classes, output_file):
        xml = ['<?xml version="1.0" encoding="UTF-8" ?>', '<LuaAPI>']
        for cpp_name in sorted(classes.keys()):
            methods = classes[cpp_name]
            xml_name = cpp_name
            force_static = False
            
            # 1. Apply L-Prefix Strip
            if xml_name.startswith('L') and xml_name[1:] in self.STRIP_L_PREFIX:
                xml_name = xml_name[1:]
            
            # 2. Apply Class Aliases/Overrides
            if cpp_name in self.CLASS_OVERRIDES:
                xml_name = self.CLASS_OVERRIDES[cpp_name]["name"]
                force_static = self.CLASS_OVERRIDES[cpp_name]["force_static"]

            xml.append(f'  <class name="{xml_name}" cpp_name="{cpp_name}">')
            inst, stat = [], []
            seen_sigs = set()

            for m in methods:
                # Deduplicate methods (handles .h and .cpp occurrences)
                sig = f"{m['name']}({m['args']})"
                if sig in seen_sigs: continue
                seen_sigs.add(sig)

                is_m_static = m["is_static"] or force_static
                type_str = "static" if is_m_static else "instance"
                line = f'      <method name="{m["name"]}" args="{m["args"]}" type="{type_str}"/>'
                
                if is_m_static: stat.append(line)
                else: inst.append(line)

            if inst:
                xml.append('    <methods>')
                xml.extend(inst)
                xml.append('    </methods>')
            if stat:
                xml.append('    <static_methods>')
                xml.extend(stat)
                xml.append('    </static_methods>')
            
            xml.append('  </class>')
        
        xml.append('</LuaAPI>')
        
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write('\n'.join(xml))
        
        print(f"--- SUCCESS ---")
        print(f"File: {output_file}")
        print(f"Size: {os.path.getsize(output_file) // 1024} KB")
        print(f"Classes found: {len(classes)}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 luabind_parser.py [Source_Directory] [Output_XML_Path]")
    else:
        gen = LuaAPIGenerator()
        found_data = gen.process_directory(sys.argv[1])
        gen.save_xml(found_data, sys.argv[2])