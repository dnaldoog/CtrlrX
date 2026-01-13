import xml.etree.ElementTree as ET
from pathlib import Path
import os

def get_classes_with_enums(xml_path=None):
    # If no path is provided, calculate the default relative to this script
    if xml_path is None:
        ROOT = Path(__file__).resolve().parent.parent
        xml_path = ROOT / "Source" / "Resources" / "XML" / "LuaAPI.xml"
    
    # Convert to string for os.path compatibility if needed
    xml_path = str(xml_path)

    if not os.path.exists(xml_path):
        print(f"Error: File {xml_path} not found.")
        return

    try:
        tree = ET.parse(xml_path)
        root = tree.getroot()
        
        classes_with_enums = []

        # Iterate through every <class> tag
        for class_node in root.findall('class'):
            class_name = class_node.get('name')
            
            # Check if this class has an <enums> child and if that child has <enum> tags
            enums_node = class_node.find('enums')
            if enums_node is not None and len(enums_node.findall('enum')) > 0:
                if class_name:
                    classes_with_enums.append(class_name)

        # Sort and format the output
        classes_with_enums.sort()
        
        print("\n--- Copy and Paste into lua_api_patcher ---")
        formatted_list = 'FLATTEN_ENUMS = [' + ', '.join(f'"{c}"' for c in classes_with_enums) + ']'
        print(formatted_list)
        print("-------------------------------------------\n")

    except ET.ParseError as e:
        print(f"Error parsing XML: {e}")

if __name__ == "__main__":
    # Update this path to where your XML is located
    xml_file_path = "Source/Resources/XML/LuaAPI.xml"
    get_classes_with_enums(xml_file_path)
