#!/usr/bin/env python3
"""
Debug script to find where CtrlrPanel is defined in Lua bindings
"""

import sys
from pathlib import Path
import re

def search_for_class(source_dir, class_name):
    """Search for class binding in C++ files."""
    
    print(f"Searching for '{class_name}' bindings in {source_dir}")
    print("=" * 60)
    
    # Patterns to search for
    patterns = [
        rf'class_<\s*{class_name}',  # Direct class binding
        rf'wrapForLua.*{class_name}',  # wrapForLua with class name
        rf'globals.*\[.*{class_name.lower()}.*\]',  # Global assignment
        rf'\.def\s*\(\s*".*{class_name}',  # Method definitions
    ]
    
    found_files = []
    
    for cpp_file in sorted(source_dir.glob("**/*.cpp")):
        try:
            content = cpp_file.read_text(encoding='utf-8', errors='ignore')
            
            for pattern in patterns:
                if re.search(pattern, content, re.IGNORECASE):
                    if cpp_file not in found_files:
                        found_files.append(cpp_file)
                        print(f"\n✓ Found in: {cpp_file.relative_to(source_dir)}")
                        
                        # Show matching lines
                        for line_num, line in enumerate(content.split('\n'), 1):
                            if re.search(pattern, line, re.IGNORECASE):
                                print(f"  Line {line_num}: {line.strip()[:80]}")
        except Exception as e:
            continue
    
    if not found_files:
        print(f"\n✗ No bindings found for '{class_name}'")
        print("\nPossible reasons:")
        print("1. Class is assigned as a global (not in class_<...>)")
        print("2. Class binding is in a header file (.h)")
        print("3. Class name is different (check wrapForLua methods)")
    else:
        print(f"\n{'='*60}")
        print(f"Found {len(found_files)} file(s) with '{class_name}' bindings")
    
    return found_files


def check_wrap_for_lua(source_dir):
    """List all files with wrapForLua."""
    print("\n" + "=" * 60)
    print("Files with wrapForLua:")
    print("=" * 60)
    
    wrap_files = []
    
    for cpp_file in sorted(source_dir.glob("**/*.cpp")):
        try:
            content = cpp_file.read_text(encoding='utf-8', errors='ignore')
            if 'wrapForLua' in content:
                wrap_files.append(cpp_file)
                
                # Extract class name from wrapForLua
                match = re.search(r'void\s+(\w+)::wrapForLua', content)
                if match:
                    class_name = match.group(1)
                    print(f"  {cpp_file.relative_to(source_dir):<50} ({class_name})")
                else:
                    print(f"  {cpp_file.relative_to(source_dir)}")
        except:
            continue
    
    print(f"\nTotal: {len(wrap_files)} files")
    return wrap_files


def main():
    if len(sys.argv) < 2:
        print("Usage: python debug_parser.py <source_directory> [class_name]")
        print("\nExamples:")
        print("  python debug_parser.py Source/")
        print("  python debug_parser.py Source/ CtrlrPanel")
        sys.exit(1)
    
    source_dir = Path(sys.argv[1])
    class_name = sys.argv[2] if len(sys.argv) > 2 else "CtrlrPanel"
    
    if not source_dir.exists():
        print(f"Error: Directory not found: {source_dir}")
        sys.exit(1)
    
    # Search for specific class
    search_for_class(source_dir, class_name)
    
    # List all wrapForLua files
    check_wrap_for_lua(source_dir)


if __name__ == "__main__":
    main()