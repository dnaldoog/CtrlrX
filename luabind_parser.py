import re
import os
import sys
import html

class LuabindHybridParser:
    def __init__(self):
        self.signatures = {} 
        self.lua_api = {}    
        self.inheritance_map = {}

    def clean_args(self, args):
        if not args or args.strip().lower() in ["void", "nullptr"]: return "()"
        # Remove defaults and JUCE namespaces for cleaner autocomplete
        a = re.sub(r'=[^,)]+', '', args) 
        a = re.sub(r'juce::|std::|const|&|\*|override|final|virtual|inline|noexcept', '', a)
        clean = ' '.join(a.split()).strip().strip(',')
        return f"({html.escape(clean)})"

    def run(self, source_dir, output_xml):
        print(f"[*] Starting scan in: {source_dir}")
        
        # PASS 1: Headers
        for root, _, files in os.walk(source_dir):
            for file in files:
                if file.endswith(('.h', '.hpp')):
                    try:
                        with open(os.path.join(root, file), 'r', encoding='utf-8', errors='ignore') as f:
                            content = f.read()
                            # Inheritance
                            for im in re.finditer(r'class\s+([A-Za-z0-9_]+)\s*:\s*(?:public|protected|private)\s+([A-Za-z0-9_:]+)', content):
                                child, parent = im.groups()
                                self.inheritance_map[child] = parent.split("::")[-1].strip()
                            # Methods
                            for m in re.finditer(r'(\w+)\s*\(([^)]*)\)\s*(?:const)?\s*[;{]', content):
                                name, args = m.groups()
                                if len(name) > 3:
                                    self.signatures[name] = self.clean_args(args)
                    except: continue

        # PASS 2: CPP Files
        found_classes = 0
        for root, _, files in os.walk(source_dir):
            for file in files:
                if file.endswith('.cpp'):
                    try:
                        with open(os.path.join(root, file), 'r', encoding='utf-8', errors='ignore') as f:
                            content = f.read()
                            # Find luabind class definitions
                            for c_match in re.finditer(r'class_<\s*([^,>]+)[^>]*>\s*(?:\(\s*"([^"]+)"\s*\))?', content):
                                cpp_name = c_match.group(1).split("::")[-1].strip()
                                lua_name = c_match.group(2) or cpp_name
                                
                                if lua_name not in self.lua_api:
                                    self.lua_api[lua_name] = {"cpp": cpp_name, "methods": set(), "static": set(), "enums": {}}
                                    found_classes += 1

                                # Find the end of the luabind block (the semicolon)
                                block_end = content.find(';', c_match.end())
                                block = content[c_match.end():block_end]

                                # 1. Extract Enums
                                for enm_match in re.finditer(r'\.enum_\s*\(\s*"([^"]+)"\s*\)\s*\[([^\]]+)\]', block, re.DOTALL):
                                    e_name, e_body = enm_match.groups()
                                    vals = re.findall(r'value\s*\(\s*"([^"]+)"', e_body)
                                    self.lua_api[lua_name]["enums"][e_name] = vals

                                # 2. Identify Static vs Instance
                                # We look for .scope section
                                scope_match = re.search(r'\.scope\s*\[(.*?)\]', block, re.DOTALL)
                                if scope_match:
                                    scope_content = scope_match.group(1)
                                    for m in re.finditer(r'(?<!\.)def\s*\(\s*"([^"]+)"', scope_content):
                                        self.lua_api[lua_name]["static"].add(m.group(1))
                                
                                # Everything else is usually an instance method
                                for m in re.finditer(r'\.def\s*\(\s*"([^"]+)"', block):
                                    m_name = m.group(1)
                                    if m_name not in self.lua_api[lua_name]["static"]:
                                        self.lua_api[lua_name]["methods"].add(m_name)
                    except: continue

        print(f"[*] Found {found_classes} classes. Generating XML...")
        self.generate_xml(output_xml)

    def generate_xml(self, output_file):
        xml = ['<?xml version="1.0" encoding="UTF-8" ?>', '<LuaAPI>']
        
        # Consolidation Logic (L-prefix merging)
        final_api = {}
        for lua_name, data in self.lua_api.items():
            # If it's LClassName, merge it into ClassName if possible
            clean_name = lua_name[1:] if lua_name.startswith('L') and len(lua_name) > 1 and lua_name[1].isupper() else lua_name
            if clean_name not in final_api:
                final_api[clean_name] = data
            else:
                final_api[clean_name]["methods"].update(data["methods"])
                final_api[clean_name]["static"].update(data["static"])
                final_api[clean_name]["enums"].update(data["enums"])

        for name in sorted(final_api.keys()):
            d = final_api[name]
            parent = f' inherits="{self.inheritance_map[d["cpp"]]}"' if d["cpp"] in self.inheritance_map else ""
            xml.append(f'  <class name="{name}" cpp_name="{d["cpp"]}"{parent}>')
            
            if d["enums"]:
                xml.append('    <enums>')
                for e_name, values in d["enums"].items():
                    xml.append(f'      <enum name="{e_name}">')
                    for v in values:
                        xml.append(f'        <value name="{v}" />')
                    xml.append('      </enum>')
                xml.append('    </enums>')

            xml.append('    <methods>')
            for m in sorted(d["methods"]):
                xml.append(f'      <method name="{m}" args="{self.signatures.get(m, "()")}" type="instance"/>')
            for m in sorted(d["static"]):
                xml.append(f'      <method name="{m}" args="{self.signatures.get(m, "()")}" type="static"/>')
            xml.append('    </methods>')
            xml.append('  </class>')
        
        xml.append('</LuaAPI>')
        
        try:
            with open(output_file, 'w', encoding='utf-8') as f:
                f.write('\n'.join(xml))
            print(f"[+] Success! Wrote to {output_file}")
        except Exception as e:
            print(f"[!] Error writing file: {e}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python luabind_parser.py <source_dir> <output_xml>")
    else:
        LuabindHybridParser().run(sys.argv[1], sys.argv[2])