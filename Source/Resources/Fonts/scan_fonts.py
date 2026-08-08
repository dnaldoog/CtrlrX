import os
from fontTools.ttLib import TTFont

FONT_DIR = "."  # Current directory, or set to your custom font folder path

print(f"Scanning fonts in '{FONT_DIR}'...\n")

for filename in os.listdir(FONT_DIR):
    if filename.lower().endswith((".ttf", ".otf")):
        filepath = os.path.join(FONT_DIR, filename)
        try:
            font = TTFont(filepath)
            
            # 1. Check Unicode Mapping for ASCII Hyphen (0x002D)
            cmap = font.getBestCmap()
            has_ascii_hyphen = (0x002D in cmap) if cmap else False
            has_unicode_hyphen = (0x2010 in cmap) if cmap else False

            # 2. Check for Duplicate Glyph Names
            glyph_names = font.getGlyphOrder()
            duplicate_names = set([name for name in glyph_names if glyph_names.count(name) > 1])

            # 3. Report Anomalies
            issues = []
            if not has_ascii_hyphen:
                issues.append("MISSING U+002D (ASCII Hyphen '-')")
            if has_unicode_hyphen and not has_ascii_hyphen:
                issues.append("HAS U+2010 BUT MISSING U+002D (Likely cause of CoreText/JUCE bugs!)")
            if duplicate_names:
                issues.append(f"DUPLICATE GLYPH NAMES: {list(duplicate_names)}")

            if issues:
                print(f"[!] {filename}:")
                for issue in issues:
                    print(f"    - {issue}")
            else:
                print(f"[OK] {filename}")

        except Exception as e:
            print(f"[ERROR] Could not parse {filename}: {e}")

print("\nScan complete!")