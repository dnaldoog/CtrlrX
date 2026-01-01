import re
import os
import sys

class LuaAPIGenerator:
    def __init__(self):
        # Detects C++ class definitions
        self.class_re = re.compile(r'^\s*class\s+([A-Za-z0-9_]+)\s*(?::\s*public\s+[^{]+)?\s*\{', re.MULTILINE)
        # Relaxed Method Regex to capture standard and virtual methods
        self.method_re = re.compile(r'(?:virtual\s+)?(?:[\w<>\d::]+\s+)+(\w+)\s*\(([^)]*)\)', re.MULTILINE)

    def clean_args(self, args):
        """Refines C++ arguments, normalizes types, and filters logic/comment leaks."""
        if not args or args.strip().lower() in ["void", "nullptr", "0"]:
            return "()"
        
        # 1. Strip default values (= 0), comments (//), and C++ syntax keys
        args = re.sub(r'=[^,)]+', '', args)
        args = re.sub(r'/\*.*?\*/|//.*', '', args)
        args = re.sub(r'juce::|std::|const|&|\*|override|final|virtual|inline|noexcept', '', args)
        
        # 2. Logic Leak Check: Filter out descriptive human text 
        # (e.g., "not GNOME Classic", "set to 0", "binary")
        noise_phrases = [
            "jump to", "updated with", "byte", "user just", "chained", 
            "gnome", "set to", "binary", "dunno", "internal", "check"
        ]
        if any(phrase in args.lower() for phrase in noise_phrases):
            return "()"

        # 3. Symbol Check: If it contains math or scope operators, it's code, not a param list
        if any(c in args for c in ['+', '"', '/', '{', '}', '.', '?', '!', '<', '>']):
            return "()"
            
        # 4. Clean up spacing and commas
        args = ' '.join(args.split()).strip().strip(',')

        # 5. Type Normalization for Lua
        replacements = {
            r'\bString\b': 'string',
            r'\bbool\b': 'boolean',
            r'\bint\b': 'number',
            r'\buint\d*\b': 'number',
            r'\bint\d*\b': 'number',
            r'\bdouble\b': 'number',
            r'\bfloat\b': 'number',
            r'\bvar\b': 'variable',
            r'\bsize_t\b': 'number',
            r'\bptrdiff_t\b': 'number'
        }
        for pattern, replacement in replacements.items():
            args = re.sub(pattern, replacement, args, flags=re.IGNORECASE)

        return f"({args})" if args else "()"

    def process_directory(self, input_path):
        all_classes = {}
        # Blacklist of C++ keywords and internal JUCE macros misidentified as methods
        blacklisted_names = [
            "if", "for", "while", "return", "switch", "static_assert",
            "JUCE_LEAK_DETECTOR", "JUCE_DECLARE", "JUCE_API",
            "JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR",
            "JUCE_DECLARE_NON_COPYABLE"
        ]

        file_count = 0
        class_count = 0

        for root, _, files in os.walk(input_path):
            for file in files:
                if file.endswith((".h", ".cpp")):
                    file_count += 1
                    try:
                        with open(os.path.join(root, file), 'r', encoding='utf-8', errors='ignore') as f:
                            content = f.read()
                            for match in self.class_re.finditer(content):
                                class_name = match.group(1).strip()
                                
                                # Skip blacklisted class names or noise
                                if class_name in blacklisted_names or len(class_name) < 2:
                                    continue

                                block_start = match.end()
                                next_class = self.class_re.search(content, block_start)
                                block_end = next_class.start() if next_class else len(content)
                                class_block = content[block_start:block_end]
                                
                                methods = []
                                for m_match in self.method_re.finditer(class_block):
                                    m_name = m_match.group(1)
                                    
                                    # Filter: Destructors, Blacklisted keywords, and Lua wrapper internal methods
                                    if (m_name.startswith('~') or 
                                        m_name in blacklisted_names or 
                                        m_name[0].isdigit() or # Methods can't start with numbers (e.g., 8bit)
                                        "wrapForLua" in m_name):
                                        continue
                                    
                                    m_args = self.clean_args(m_match.group(2))
                                    methods.append({'name': m_name, 'args': m_args})
                                
                                if methods:
                                    class_count += 1
                                    all_classes.setdefault(class_name, []).extend(methods)
                    except Exception as e:
                        print(f"Error processing {file}: {e}")
                        continue
        
        print(f"--- Statistics ---")
        print(f"Files scanned: {file_count}")
        print(f"Classes found: {class_count}")
        return all_classes

    def save_xml(self, classes, output_file):
        if not classes:
            print("No valid API data found. XML not saved.")
            return

        xml = ['<?xml version="1.0" ?>', '<LuaAPI>']
        for name in sorted(classes.keys()):
            cpp_name = name
            if name == "BigInteger": cpp_name = "LBigInteger"
            
            xml.append(f'  <class name="{name}" cpp_name="{cpp_name}">')
            xml.append('    <methods>')
            
            seen_signatures = set()
            for m in classes[name]:
                # Unique signature ensures we don't have literal duplicates
                sig = f"{m['name']}_{m['args']}"
                if sig not in seen_signatures:
                    # Strip brackets for the XML attribute
                    clean_args = m['args'].strip('()')
                    xml.append(f'      <method name="{m["name"]}" args="{clean_args}" type="instance"/>')
                    seen_signatures.add(sig)
                    
            xml.append('    </methods>')
            xml.append('  </class>')
        xml.append('</LuaAPI>')
        
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write('\n'.join(xml))
        print(f"Successfully saved API to: {output_file}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 script.py <input_dir> <output_xml>")
    else:
        gen = LuaAPIGenerator()
        print("Parsing Source...")
        data = gen.process_directory(sys.argv[1])
        gen.save_xml(data, sys.argv[2])