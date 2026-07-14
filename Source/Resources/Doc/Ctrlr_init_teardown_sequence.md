# CtrlrX Architecture: Load & Teardown Lifecycles

This document outlines how CtrlrX boots up (Load Phase), how it destroys itself during shutdown (Teardown Phase), and why order of operations is critical to preventing **Read Access Violations** (Null Pointer / Use-After-Free crashes) in C++ and JUCE.

---

## 1. The Core Architecture (MVC)

CtrlrX operates on a classic **Model-View-Controller (MVC)** structural pattern.
Because it is written in C++ and uses JUCE, memory is managed hierarchically. Parents own children, and parents are responsible for cleaning up their children when they die.

---

## 2. The Load Phase (Building the Tower)

When `CtrlrX.exe` is launched, the application builds its memory structure from the inside out, establishing a strict parent-child ownership chain:

```text
[CtrlrApplication] (The Main App)
       │
       └──> [CtrlrStandaloneWindow] (The OS Window Frame)
                   │
                   ├──> [CtrlrProcessor] (The MIDI Engine & Lua State)
                   │           │
                   │           └──> [CtrlrManager] (The Master XML Data)
                   │
                   └──> [CtrlrPanelEditor] (The UI Container)
                               │
                               ├──> [CtrlrPanelViewport]
                               │           │
                               │           └──> [CtrlrPanelCanvas] (The Background)
                               │
                               └──> [CtrlrSlider / UI Components] (The Knobs/Buttons)

```

### The Startup Steps:
<BR><BR>
1. **The Core Engine (`CtrlrProcessor`):** The engine is initialized first. It handles the MIDI routing, compiles the Lua virtual machine environment, and creates the **`CtrlrManager`**, which acts as the "Model" holding the master XML/`ValueTree` data.
2. **The Shell (`CtrlrStandaloneWindow`):** The application creates the physical OS desktop window, giving it a pointer to the newly created processor.
3. **The Face (`CtrlrPanelEditor`):** The window spawns the master editor component, which in turn populates itself with the viewport, the drawing canvas, and all individual modulator widgets (sliders, buttons, combos).
4. **The Wiring (ValueTree Listeners):** To ensure the UI updates when properties change, UI elements (like `CtrlrPanelCanvas` and `CtrlrPanelEditor`) register themselves as **listeners** to the underlying data trees.

---

## 3. The Teardown Phase (The "Crash Zone")

When closing the application, this entire tower must collapse safely. If a parent object (like the processor or manager) is deleted while a child component is still "wired" to it, the child will try to interact with a "ghost" address in memory, causing a crash.

### The Correct & Safe Teardown Sequence:

### Step 1: Mute the Wires (Detaching Listeners)

Before deleting any memory, we must tell all UI classes to stop listening to the data model. If we skip this, modifications during destruction will fire callbacks to dead objects.

```cpp
owner.getPanelTree().removeListener(this); // Editor stops listening
canvasTree.removeListener(this);           // Canvas stops listening

```

### Step 2: Save the App State

While the processor and its child data pools are still 100% intact, the window calls `saveStateNow()`. This serializes all slider positions and window coordinates to disk.

### Step 3: Delete the UI (The View)

The application executes `deleteAndZero(filterWindow)`.

* This invokes `~CtrlrStandaloneWindow()`.
* The window destructor tears down the `CtrlrPanelEditor`.
* The editor deletes the viewport, the canvas, and all active modulators.

### Step 4: Delete the Engine (The Model)

The window's destructor finishes by calling `deleteFilter()`.

* This deletes `CtrlrProcessor` and `CtrlrManager`.
* MIDI ports are safely closed.
* The Lua states are completely destroyed.

### Step 5: Clean Up static JUCE Resources

Finally, static styles, look-and-feels, singletons, and the background OS message loops are safely unallocated.

---

## 4. Why the Crashes Were Happening

Almost every shutdown crash was a result of **Step 1 (Detaching Listeners)** being executed *after* **Step 4 (Deleting the Engine)**.

When the destruction sequence was out of order:

1. The processor and manager were deleted first.
2. The UI components were still active in memory and still registered as listeners.
3. A component's destructor or a visibility change modified a property tree.
4. The remaining UI components received a "Hey, this property changed!" message.
5. They attempted to handle the change by calling:
`panel.getCtrlrLuaManager().getMethodManager()`
6. Because the panel and Lua managers were already deleted, the application dereferenced a `nullptr` or a dangling pointer, instantly throwing a **Read Access Violation**.

### The Golden Rule of JUCE/C++ UI Teardown:

> **Always unregister your ValueTree and ChangeBroadcaster listeners BEFORE you destroy the objects they listen to.**