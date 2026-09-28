# Viewport Options

The **Viewport options** property in the Panel property editor controls how an exported instance of your panel behaves on load or when the user resizes its window. There are three choices. New panels default to **Scrollable**.

- **Scrollable**: window resizable, scrollbars when the panel is larger than the window.
- **Fixed**: window locked to the panel size, no scrollbars.
- **Scaled**: window resizable with the aspect ratio locked, the panel scales to fit, no scrollbars.

## Scrollable

Use this for large panels that you want to frame in a smaller window. The user can scroll to reach parts of the panel that are outside the visible area.

The window's starting size comes from two properties:

- `uiViewPortWidth`
- `uiViewPortHeight`

For example, a panel that is 2000 x 1600 with the ViewPort set to 600 x 400 opens as a 600 x 400 window that the user can scroll around.

- A value of `0` means "use the panel size".
- A value larger than the panel size is treated as the panel size.
- The user can resize the window, but the next launch opens at your size again. The user's window size is not remembered between sessions.

## Fixed

Use this when the panel should always appear at its designated size. The window cannot be resized and has no scrollbars. `uiViewPortWidth` and `uiViewPortHeight` are ignored.

## Scaled

Use this when the panel should grow and shrink with the window. Dragging the window corner keeps the panel's aspect ratio and scales the whole panel to fit. There are no scrollbars. `uiViewPortWidth` and `uiViewPortHeight` are ignored.

### `uiViewPortWidth` and `uiViewPortHeight`

These two properties only have an effect when **Viewport options** is set to **Scrollable**. In **Fixed** and **Scaled** modes they are ignored. They do not change the size of the panel itself, which comes from the panel's canvas rectangle.

### Working in the editor

The viewport options apply to the running panel (for example an exported instance), not to the design view. While you are editing, the panel is shown normally so you can lay it out at its real size.

## Using the mode from Lua

The chosen mode can be read from a script as text:

```lua
local mode = panel:getPanelEditor():getProperty("uiViewPortMode")
-- "Scrollable", "Fixed" or "Scaled"
```

## Upgrading existing panels
<br>

Earlier versions had several separate viewport properties. They are no longer used:

- `uiViewPortResizable`
- `uiViewPortShowScrollBars`
- `uiViewPortEnableFixedAspectRatio`
- `uiViewPortFixedAspectRatio`

If an older panel still contains them, the Property Editor shows them as read-only, and they have no effect. Older panels open as **Scrollable**. To get **Fixed** or **Scaled** behaviour, choose that option and export the panel again.

`uiViewPortWidth` and `uiViewPortHeight` used to record the window's current size. They are now the initial window size for **Scrollable** panels. An old panel may hold a stale value from an earlier run. Values larger than the panel are reduced to the panel size, so the window never opens larger than the panel.

If you previously used a `luaViewPortResized` script to scale the panel with the window, you no longer need it. Choose **Scaled** instead, and remove any script that sets `uiPanelZoom`, because it would fight the built-in scaling.


