# CtrlrX Lua API Maintenance Guide

This document explains the automated pipeline used to generate, refine, and embed the Lua API documentation and autocomplete data into CtrlrX.

## The API Pipeline

To ensure the Lua environment stays synchronized with the C++ source code while remaining user-friendly, the API follows a three-stage build process.

### 1. Extraction (`extract_api.py`)

The **Scraper** scans the C++ source files for `luabind` module definitions to create the "ground truth" of the API.

* **Location:** `python Scripts/luabind_parser.py`
* **What it does:** * Parses `.cpp` files for `class_<T>` definitions and `.def()` method calls.
* Indexes C++ headers to extract method signatures and arguments.
* **Consolidation:** Automatically merges "L" wrappers (e.g., `LMemoryBlock`) into their base JUCE classes (e.g., `MemoryBlock`).
* **Pruning:** Removes empty "placeholder" classes (like empty `J` or `JUCE` tags) to keep the API clean.



### 2. Refinement (`lua_api_patcher.py`)

The **Patcher** takes the raw XML and applies "Quality of Life" improvements specifically for the Lua scripting environment.

* **Location:** `Scripts/lua_api_patcher.py`
* **What it does:**
* **Enum Flattening:** Automatically flattens nested enum structures so users can type `Justification.left` instead of `Justification.Flags.left`.
* **Aliasing:** Creates deep-copy aliases for common Lua shortcuts (e.g., mapping `CtrlrPanel` to `panel`).
* **Manual Patches:** Injects constructors or special method signatures that are handled dynamically by Lua and cannot be easily scraped from C++.



### 3. Embedding (Projucer & BinaryData)

To ensure the API Browser and Autocomplete work "out-of-the-box" on all platforms (especially Linux) without external file dependencies, the XML is baked into the executable.

* **Source File:** `Source/Resources/XML/LuaAPI.xml`
* **Mechanism:** When the project is saved in **Projucer**, it refreshes `BinaryData.cpp`, converting the XML file into a static byte array.
* **Runtime:** The `CtrlrLuaApiDatabase` and `CtrlrLuaMethodAutoCompleteManager` load the XML directly from memory using `BinaryData::LuaAPI_xml`.

---

## How to Update the API

Follow these steps if you have added new C++ bindings or want to update the Lua documentation:

### Step 1: Run the Automation Scripts

Open a terminal in the CtrlrX root folder and run:

```bash
# Generate the raw XML from source
python3 Scripts/luabind_parser.py

# Apply Lua-specific refinements and aliases
python3 Scripts/lua_api_patcher.py

```

### Step 2: Update BinaryData

1. Open `CtrlrX.jucer` in the **Projucer**.
2. Press `Cmd+S` (macOS) or `Ctrl+S` (Windows/Linux) to save the project.
3. This step is crucial as it updates the C++ source code containing the embedded XML data.

### Step 3: Recompile

Build the project through your IDE (Visual Studio, Xcode, etc.) or via the command line. The changes will now be visible in the **Lua Method Browser** and the **Code Editor** autocomplete.

---

## Troubleshooting

* **Missing Methods:** Check if the method is correctly registered in the `luabind::module` block in the C++ source.
* **Broken Autocomplete:** Ensure that `AUTO_FLATTEN_ALL_ENUMS` is enabled in the patcher script if enums are appearing with incorrect paths.
* **Linux API Browser Empty:** Ensure the build process included the `BinaryData.cpp` update and that the `CtrlrLuaApiDatabase` is successfully calling `loadFromMemory`.

---

