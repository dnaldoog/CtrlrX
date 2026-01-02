import re
import os
import sys
import html

class InheritanceParserV21:
    def __init__(self):
        self.signatures = {} 
        self.lua_api = {}    
        self.inheritance_map = {} # class_name -> parent_name

    def clean_args(self, args):
        if not args or args.strip().lower() in ["void", "nullptr"]: return "()"
        a = re.sub(r'=[^,)]+', '', args) 
        a = re.sub(r'juce::|std::|const|&|\*|override|final|virtual|inline|noexcept', '', a)
        clean = ' '.join(a.split()).strip().strip(',')
        return f"({html.escape(clean)})"

    def run(self, source_dir, output_xml):
        print(f"Scanning: {source_dir}")
        
        # PASS 1: Scrape Definitions & Inheritance from Headers
        for root, dirs, files in os.walk(source_dir):
            for file in files:
                if file.endswith(('.h', '.hpp')):
                    path = os.path.join(root, file)
                    try:
                        with open(path, 'r', encoding='utf-8', errors='ignore') as f:
                            content = f.read()
                            
                            # Detect Inheritance: class Name : public Parent
                            inherit_matches = re.finditer(r'class\s+([A-Za-z0-9_]+)\s*:\s*(?:public|protected|private)\s+([A-Za-z0-9_:]+)', content)
                            for im in inherit_matches:
                                child, parent = im.groups()
                                # Clean up namespace::Parent to just Parent
                                self.inheritance_map[child] = parent.split("::")[-1].strip()

                            # Scrape Methods
                            matches = re.finditer(r'(\w+)\s*\(([^)]*)\)\s*(?:const)?\s*[;{]', content)
                            for m in matches:
                                name, args = m.groups()
                                if len(name) > 3 and name not in ["if", "for", "while"]:
                                    cleaned = self.clean_args(args)
                                    if name not in self.signatures or len(cleaned) > len(self.signatures[name]):
                                        self.signatures[name] = cleaned
                    except: continue

        # PASS 2: Luabind Mapping (The v1 part)
        for root, dirs, files in os.walk(source_dir):
            for file in files:
                if file.endswith('.cpp'):
                    path = os.path.join(root, file)
                    try:
                        with open(path, 'r', encoding='utf-8', errors='ignore') as f:
                            content = f.read()
                            for c_match in re.finditer(r'class_<\s*([^,>]+)[^>]*>\s*(?:\(\s*"([^"]+)"\s*\))?', content):
                                cpp_name = c_match.group(1).split("::")[-1].strip()
                                lua_name = c_match.group(2) or cpp_name
                                if lua_name not in self.lua_api:
                                    self.lua_api[lua_name] = {"cpp": cpp_name, "methods": set()}
                                
                                block_end = content.find(';', c_match.end())
                                block = content[c_match.end():block_end]
                                for d_match in re.finditer(r'\.def\s*\(\s*"([^"]+)"', block):
                                    self.lua_api[lua_name]["methods"].add(d_match.group(1))
                    except: continue

        self.generate_xml(output_xml)

    def generate_xml(self, output_file):
        xml = ['<?xml version="1.0" encoding="UTF-8" ?>', '<LuaAPI>']
        for lua_name in sorted(self.lua_api.keys()):
            data = self.lua_api[lua_name]
            cpp_name = data["cpp"]
            
            # Look up the parent in our inheritance map
            parent_attr = ""
            if cpp_name in self.inheritance_map:
                parent_attr = f' inherits="{self.inheritance_map[cpp_name]}"'
            
            xml.append(f'  <class name="{lua_name}" cpp_name="{cpp_name}"{parent_attr}>')
            
            if data["methods"]:
                xml.append('    <methods>')
                for m_name in sorted(data["methods"]):
                    args = self.signatures.get(m_name, "()")
                    xml.append(f'      <method name="{m_name}" args="{args}" type="instance"/>')
                xml.append('    </methods>')
            xml.append('  </class>')
        
        xml.append('</LuaAPI>')
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write('\n'.join(xml))
        print(f"Success! Created {output_file} with inheritance tracking.")

if __name__ == "__main__":
    InheritanceParserV21().run(sys.argv[1], sys.argv[2])