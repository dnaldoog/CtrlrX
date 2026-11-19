# Multi-column combo popups

CtrlrX supports opt-in, panel-bounded multi-column popups for `uiCombo`
components. The behavior is off by default; a panel changes only when it sets
the properties below.

## Properties

| Property | Type | Default | Meaning |
|---|---|---|---|
| `uiComboMenuMaxColumns` | `VarNumeric` | `1` | Maximum number of popup columns, clamped to 1–7 |
| `uiComboMenuBoundToPanel` | `Bool` | `false` | Use the panel editor as the popup parent so the menu is bounded by the panel |
| `uiComboMenuDirection` | `VarText` | `Automatic` | `Automatic`, `Upwards`, or `Downwards` |
| `uiComboMenuMaxWidth` | `VarNumeric` | `0` | Maximum width of one menu item in pixels; `0` means unlimited |

The values are stored in the panel XML. A missing property uses the default.

## Native popup

When `uiComboSearch` is **off**, the combo uses the native JUCE popup:

- `uiComboMenuMaxColumns` sets the column limit (1–7).
- `uiComboMenuBoundToPanel` uses the panel editor as the parent.
- `uiComboMenuDirection` controls the preferred opening direction.
- `uiComboMenuMaxWidth` caps the ideal item width.

`uiComboMenuMaxWidth` is an **item-width cap**, not a window-width clamp. The
popup also keeps `.withMinimumWidth(box.getWidth())`, so:

> The cap is effective when `uiComboMenuMaxWidth` is greater than or equal to
> the combo width. If the cap is smaller, the combo width wins as the popup
> minimum.

Long labels are fitted to the capped item rectangle. A value of `0` means no
width change.

## Fuzzy search

When `uiComboSearch` is **on**, the combo opens `FuzzySearchPanel`; the native
popup is not used. Search therefore takes precedence over the native popup, but
the search grid still uses the menu properties:

- `uiComboMenuMaxColumns` sets the number of result columns.
- `uiComboMenuMaxWidth` caps the search-panel width.

When both properties are set, the width cap applies to the whole search panel,
so the approximate cell width is `uiComboMenuMaxWidth / uiComboMenuMaxColumns`.
A four-column grid with a 240 px cap therefore has roughly 60 px cells; choose
the cap with that division in mind.

Interaction:

- Clicking a result cell selects that item **and closes the popup**.
- Enter selects the first item in the **keyboard-selected row**.
- Up/Down move between rows.
- Scrolling is by row.
- Left/Right remain with the search text editor for caret movement; they do
  not move between result columns.

## Opt-in behavior

- Existing panels keep their single-column, screen-bounded behavior.
- A panel author can enable columns only for large lists.
- A width cap can be set only where long names would otherwise overflow.
- The panel XML records the choice; the engine does not need a global switch.

## Recommended starting values

| List | Columns | Bound | Max width |
|---|---:|---|---:|
| 128 Program names | 3–4 | Yes | At least the combo width; measure |
| 3,584 GE names | 4–6 | Yes | At least the combo width; measure |
| Small menus | 1 | No | 0 |

Measure the actual panel at native size and with host scaling before fixing the
values. The correct column count depends on item text, font size, panel height,
and available screen width.

## Limits

- The native popup still uses JUCE's popup layout; `uiComboMenuMaxWidth` caps
  item width but does not guarantee that the popup stays inside the panel’s
  horizontal bounds.
- A window-level horizontal clamp is a separate feature.
- The search grid scrolls by rows; per-column keyboard selection is not part of
  this patch.
- The properties affect only combos that opt in.
