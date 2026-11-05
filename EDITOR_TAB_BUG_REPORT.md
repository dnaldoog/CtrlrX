# Kronos Kontrol — EDITOR button does nothing (CtrlrX 8.7.7)

**Status:** fixed in the engine **and** in the panel, verified by clicking the button
**Date:** 5 November 2026
**Engine:** CtrlrX 8.7.7 (JUCE 8.0.12), Release x64, MSVC 19.51
**Panel:** `KronosKontrol_1.0i.panel`
**Commits:** `9119ece` (engine), `7f938eb` (panel + provenance), `6c12d84` / this revision (docs)
**Companion:** `BUG_PROVENANCE_5.6.36_vs_8x.md` — proves it is an 8.x regression, with the
5.6.36 vs 8.x source comparison. That is the document to send upstream.

**Contents:** §5–§6 the two defects · §7 how they compound · **§8 the fix, both layers,
with code** · §9 verification · §11–§12 compatibility and risk · §13 upstream options.

---

## 1. Summary

Clicking the panel's **EDITOR** button did nothing at all — no error, no window, no
tab change. Investigation found **two independent regressions** in the same
component, both silent:

| # | Defect | Effect on the panel |
|---|---|---|
| 1 | `uiTabsCurrentTab` had become write-only — nothing read it back | The button's tab switch did nothing |
| 2 | `tabChangedCbk` (`uiTabsCurrentTabChanged`) was resolved but never invoked | Even a successful switch would show a stale EDIT page |

Either defect alone would have broken the button. Fixing only #1 would have produced
"the page opens but is empty/stale" — a worse-looking bug.

---

## 2. Symptom

- Load the panel: main rack works — program/combi display, PCG loading, MIDI traffic.
- Click **EDITOR**: no response whatsoever. No Lua error, no console output.
- `< RACK` (the return path) was equally affected, because it used the same mechanism.

The panel does not report the failure because its click handler is wrapped in
`kkGuard("kkClick", ...)`, which catches exceptions; a clean no-op produces nothing to
report.

---

## 3. Reproduction

1. Launch `CtrlrX.exe` and open `KronosKontrol_1.0i.panel`.
2. Click **EDITOR** (rack position ≈ 486, 223 in panel coordinates).
3. Before the fix: nothing. After the fix: the EDIT page opens, populated.

Deterministic, no MIDI or hardware state required.

---

## 4. Investigation path

Worth recording, because the failure gave no signal at all:

1. Found the button in the panel builder: `build_panel.py:285` —
   `button('kkBtnEdit', *lay('editor'))`, which defaults to callback `kkClick`.
2. Extracted the `kkClick` Lua method from the built `.panel`. Dispatch is by
   modulator name; the relevant branch was:

   ```lua
   elseif n == "kkBtnEdit" then
     panel:getModulatorByName("kkTabs"):getComponent():setPropertyInt("uiTabsCurrentTab", 1)
   ```

3. Checked what consumes `uiTabsCurrentTab` in the engine — and found the property is
   only ever **written**, never **read** as an input.
4. Checked the sibling mechanism `uiTabsCurrentTabChanged`, which the panel sets to
   `kkTabChanged` — and found the resolved callback is never invoked.

---

## 5. Bug 1 — `uiTabsCurrentTab` is write-only

### 5.1 What the panel does

```lua
-- panels/KronosKontrol/lua/kkMain.lua:1593-1594
elseif n == "kkBtnEdit"   then panel:getModulatorByName("kkTabs"):getComponent():setPropertyInt("uiTabsCurrentTab", 1)
elseif n == "kkBtnToRack" then panel:getModulatorByName("kkTabs"):getComponent():setPropertyInt("uiTabsCurrentTab", 0)
```

A third site, the busy-guard in the tab callback:

```lua
-- panels/KronosKontrol/lua/kkMain.lua:1787
if KK.busy then
  panel:getModulatorByName("kkTabs"):getComponent():setPropertyInt("uiTabsCurrentTab", 1)
```

### 5.2 Why the call is delivered but ignored

`CtrlrComponent::setProperty()` writes into the component's own `componentTree`
(`CtrlrComponent.cpp:522-527`), and the component listens to that tree
(`CtrlrComponent.cpp:55`). So `CtrlrTabsComponent::valueTreePropertyChanged()` **does**
receive the property — it simply had no branch for it.

The only branch that existed treated the property as an *output*:

```cpp
// CtrlrTabsComponent.cpp:152 (before the fix)
void CtrlrTabsInternal::currentTabChanged(int newCurrentTabIndex, const String &newCurrentTabName) {
    owner.setProperty(Ids::uiTabsCurrentTab, newCurrentTabIndex);   // write only
}
```

`grep uiTabsCurrentTab` across the whole engine returned exactly four hits:

| File | Line | Role |
|---|---|---|
| `CtrlrIDs.h` | 543 | identifier declaration |
| `CtrlrTabsComponent.cpp` | 153 | **write** (tab changed) |
| `CtrlrIDs.h` | 546 | identifier declaration |
| `CtrlrTabsComponent.cpp` | 272 | read, but for `uiTabsCurrentTabChanged`, a different identifier |

There is no reader that would move the tab bar. Hence: silent no-op.

### 5.3 The supported route

```cpp
// CtrlrTabsComponent.cpp:239
void CtrlrTabsComponent::setComponentValue(const double newValue, const bool sendChangeMessage) {
    if (newValue >= ctrlrTabs->getNumTabs())
        ctrlrTabs->setCurrentTabIndex(0);
    else
        ctrlrTabs->setCurrentTabIndex(newValue);
    ...
}
```

and it *is* exposed to Lua (`CtrlrComponent.cpp:727-728`):

```cpp
.def("setComponentValue", &CtrlrComponent::setComponentValue)
.def("setValue",          &CtrlrComponent::setComponentValue)
```

So the panel script was relying on a property path that no longer exists, while a
working API sat right next to it.

---

## 6. Bug 2 — the tab callback is dead code

### 6.1 What the panel expects

`build_panel.py:216` stores `uiTabsCurrentTabChanged='kkTabChanged'` on the tabs
component. The built panel confirms it:

```
uiTabsCurrentTabChanged = 'kkTabChanged'
```

The panel uses that callback to refresh the EDIT page on arrival:

```lua
-- panels/KronosKontrol/lua/kkMain.lua:1783-1793
-- onglet RACK -> EDIT (uiTabsCurrentTab) : rafraîchir la page EDIT à l'arrivée
local function kkTabChangedBody(mod, value)
  if KK.busy then
    panel:getModulatorByName("kkTabs"):getComponent():setPropertyInt("uiTabsCurrentTab", 1)
    kkEdStatus('Write in progress…')
    return
  end
  if math.floor((value or 0) + 0.5) == 1 then kkEditShow() end
end
```

### 6.2 The engine resolves it, then drops it

```cpp
// CtrlrTabsComponent.cpp:272 (before the fix)
else if (property == Ids::uiTabsCurrentTabChanged) {
    if (getProperty(property) == "")
        return;
    tabChangedCbk = owner.getOwnerPanel()
                        .getCtrlrLuaManager().getMethodManager()
                        .getMethod(getProperty(property));
}
```

A search for every reference to `tabChangedCbk` in the source tree returned exactly
**two**: the assignment above, and its `WeakReference` declaration in
`CtrlrTabsComponent.h:128`. There was **no invocation anywhere**.

A scan of all 17 `WeakReference<CtrlrLuaMethod>` callback members across the engine
(`.def`-style registration + `.call(...)` sites) showed every other callback
(`labelChangedCbk`, `valueChangedCbk`, `paintCbk`, `mouseMoveCbk`,
`fileClickedCbk`, `itemClickedCbk`, `setValueCbk`, …) has at least one invocation site.
`tabChangedCbk` was the only orphan. *(Scan is name-based and not a proof of semantic
correctness, but it isolates the defect.)*

### 6.3 Consequence

`kkTabChanged` never ran — not on the EDITOR button, and **not when the user clicks the
EDIT tab in the tab bar either**. The EDIT page could therefore be shown without
`kkEditShow()` ever filling it, i.e. stale or blank.

---

## 7. Why the two compound

```
EDITOR click
  └─ kkClick → kkBtnEdit
       └─ setPropertyInt("uiTabsCurrentTab", 1)     ← Bug 1: nobody reads it
            (nothing happens)

[even if Bug 1 were fixed]
  └─ setCurrentTabIndex(1)
       └─ currentTabChanged()
            ├─ writes uiTabsCurrentTab                (output, fine)
            └─ [MISSING] tab callback call             ← Bug 2
                 └─ kkTabChanged → kkEditShow()        (never runs)
```

Fixing only the first would have swapped "nothing happens" for "an empty page appears".

---

## 8. The fix

The fix was applied at **two layers**, deliberately:

| Layer | Change | Why |
|---|---|---|
| Engine | `uiTabsCurrentTab` accepted as input; `tabChangedCbk` invoked | Repairs every panel written for older Ctrlr, not just this one |
| Panel | drive the tab with `setComponentValue()`; refresh explicitly | Makes this panel work on a **stock 8.7.7** and on 5.6.x too |

The engine layer alone is enough for this panel (that is what §9 verified first).
The panel layer was added afterwards as belt-and-braces: a released panel should not
depend on a patched engine to function.

### 8.1 Accept `uiTabsCurrentTab` as an input

```cpp
// CtrlrTabsComponent.cpp:289
} else if (property == Ids::uiTabsCurrentTab) {
    // Compatibility: panels written against older Ctrlr select a tab by setting
    // this property on the component (panel:...getComponent():setPropertyInt).
    // Nothing read it back, so those panel scripts silently did nothing.
    // currentTabChanged() also WRITES this property, so only act on a value
    // that actually differs from the live index.
    const int wantedTab = (int)getProperty(Ids::uiTabsCurrentTab);

    if (ctrlrTabs != nullptr && wantedTab >= 0 && wantedTab < ctrlrTabs->getNumTabs()
        && wantedTab != ctrlrTabs->getCurrentTabIndex()) {
        setComponentValue((double)wantedTab, false);
    }
}
```

**Recursion:** `setComponentValue` → `setCurrentTabIndex` → `currentTabChanged` →
writes `uiTabsCurrentTab` → listener re-enters this branch. It terminates for two
reasons: JUCE's `TabbedButtonBar::setCurrentTabIndex` guards
`if (currentTabIndex != newIndex)`, and the branch itself compares against the live
index. Also guarded against `ctrlrTabs == nullptr` (pre-construction) and out-of-range
indices.

### 8.2 Actually invoke the callback

```cpp
// CtrlrTabsComponent.cpp:152
void CtrlrTabsInternal::currentTabChanged(int newCurrentTabIndex, const String &newCurrentTabName) {
    owner.setProperty(Ids::uiTabsCurrentTab, newCurrentTabIndex);
    owner.notifyTabChanged(newCurrentTabIndex);
}

// CtrlrTabsComponent.cpp:157
void CtrlrTabsComponent::notifyTabChanged(const int newIndex) {
    // uiTabsCurrentTabChanged names a Lua callback. It was resolved when the
    // property changed but never invoked, so a panel could not react to a tab
    // change (e.g. refresh a page on arrival).
    if (owner.getOwnerPanel().getRestoreState())
        return; // values are still settling while the panel is being restored

    if (tabChangedCbk && !tabChangedCbk.wasObjectDeleted() && tabChangedCbk->isValid())
        owner.getOwnerPanel().getCtrlrLuaManager().getMethodManager()
             .call(tabChangedCbk, &owner, newIndex);
}
```

Declared in `CtrlrTabsComponent.h:100`.

**Deliberate choices:**

- Callback suppressed while `CtrlrPanel::getRestoreState()` is true, so panel loading
  does not fire panel Lua for tab state that is being restored. Same convention the
  panel itself uses in `panelResourcesChanged()`.
- The overload `call(CtrlrLuaMethod*, CtrlrModulator*, const int)` matches the panel's
  `kkTabChanged(mod, value)` signature; `value` is the new tab index.
- `WeakReference` checked for `wasObjectDeleted()` / `isValid()` before use, matching
  the pattern in `CtrlrLabel.cpp:262`.

### 8.3 Panel-side change (defensive, second layer)

`panels/KronosKontrol/lua/kkMain.lua` — three call sites:

```lua
-- before                                                  -- after
setPropertyInt("uiTabsCurrentTab", 1)                      setComponentValue(1, false); kkEditShow()
setPropertyInt("uiTabsCurrentTab", 0)                      setComponentValue(0, false)
setPropertyInt("uiTabsCurrentTab", 1)   -- busy guard     setComponentValue(1, false)
```

| Line | Context | Change |
|---:|---|---|
| 1593 | `kkBtnEdit` | `setComponentValue(1, false); kkEditShow()` |
| 1594 | `kkBtnToRack` | `setComponentValue(0, false)` |
| 1787 | `kkTabChanged` busy guard (force back to EDIT during a write) | `setComponentValue(1, false)` |

`setComponentValue(double, bool)` is the API exposed to Lua at
`CtrlrComponent.cpp:727-728` and routes straight to
`TabbedButtonBar::setCurrentTabIndex()`; the second argument is
`sendChangeMessage`, and `false` keeps the modulator value untouched, exactly as the
property write did.

**Why `kkEditShow()` is called explicitly here:** on a **stock 8.7.7** the tab callback
is still dead, so nothing would refresh the page on arrival. Calling it directly makes
the panel independent of the engine patch. On this patched engine it runs twice
(once from the button, once from the callback) — harmless, the function is idempotent.

The built panel was regenerated from the same strings (3 replacements in the embedded
`kkMain` Lua method, no `setPropertyInt("uiTabsCurrentTab"` left anywhere), so panel and
Lua source agree.

---

## 9. Verification

Driven against the running application (engine deployed from the release build):

| Step | Action | Result |
|---|---|---|
| 1 | Launch with the panel | Rack renders correctly (program 000, bank select, KARMA, EDITOR button) |
| 2 | Click **EDITOR** | EDIT page opens **populated**: program name field, `COMMON (1/3)`, `(2/3)`, `(3/3)`, TRACK EQ, area tabs (GENERAL/OSC/FILTER/AMP/LFO MIX/…) |
| 3 | Click **`< RACK`** | Returns to the rack; rendering pixel-identical to before |

Step 2 also proves Bug 2 is fixed: the page being *populated* means `kkEditShow()` ran,
which only happens through the tab callback.

Screenshots captured during verification (working copies, not part of the repo):
`C:\Korg\ctrlrx-window.png`, `C:\Korg\ctrlrx-after-click.png`,
`C:\Korg\ctrlrx-back-to-rack.png`.

---

## 10. Side observations (not defects)

- **Multiple panel tabs.** During repeated launches the app restored previously opened
  panels, ending with three `Kronos Kontrol*` tabs and ~327 MB RSS versus ~190 MB for a
  single instance. Close extra tabs before judging memory numbers.
- **Asterisk in the tab title** appears after load, indicating a modified flag. Not
  investigated here; may be pre-existing.
- **Memory per instance** remains ~190 MB, consistent with the performance work
  (741 MB → 183 MB at load).

---

## 11. Compatibility impact

- Any panel that selects a tab via
  `component:setPropertyInt("uiTabsCurrentTab", n)` works again — this is the
  behaviour older Ctrlr exposed and many existing panels may rely on.
- Any panel that registers a `uiTabsCurrentTabChanged` callback now actually receives
  it, on both programmatic and user-driven tab changes (except during panel restore).
- No saved-panel format change; the property is already stored and restored.

## 12. Regression risk

| Risk | Mitigation |
|---|---|
| Recursion between property write and tab switch | JUCE equal-index guard + live-index comparison |
| Panel Lua firing during load | suppressed by `getRestoreState()` |
| Stale callback after panel teardown | `WeakReference` validity checks |
| Invalid tab index from a script | range check against `getNumTabs()` |
| Behaviour change for panels that used the property as a pure output | none — write path unchanged; the input branch only acts on a *different* index |

**To watch when testing:** clicking the EDIT tab in the tab bar directly should now
refresh the EDIT page, and the busy-guard path (`KK.busy`) should keep the user on EDIT
during a write. Both exercise the new callback.

## 13. Suggested upstream disposition

Two changes could be proposed independently:

1. **Document `uiTabsCurrentTab` as two-way** (or restore the input branch) — otherwise
   any panel written against older Ctrlr silently fails.
2. **Either invoke or delete `tabChangedCbk`.** Resolving a Lua callback and never
   calling it is invisible to panel authors and cost several hours to find here.

An alternative to (1), if upstream prefers the property to stay output-only, is to fix
the panel instead:

```lua
panel:getModulatorByName("kkTabs"):getComponent():setComponentValue(1, false)
```

which is the API `CtrlrComponent.cpp:727-728` exposes and which routes straight to
`TabbedButtonBar::setCurrentTabIndex`. **Both were actually done here** (§8.3): the
engine fix repairs every panel at once, and the panel fix keeps this release working
on a stock 8.7.7.

**Was this a regression?** Yes — introduced in the CtrlrX 8.x line; 5.6.36 and 5.6.37
have both mechanisms and 8.66/8.77 have neither. The evidence, code excerpts and the
limits of what can be established are in
`BUG_PROVENANCE_5.6.36_vs_8x.md`, which is the document to send upstream.

---

## Appendix — files touched

| File | Change |
|---|---|
| `Source/UIComponents/CtrlrComponents/Groups/CtrlrTabsComponent.cpp` | input branch for `uiTabsCurrentTab`; `notifyTabChanged()`; call from `currentTabChanged()` |
| `Source/UIComponents/CtrlrComponents/Groups/CtrlrTabsComponent.h` | declaration of `notifyTabChanged(const int)` |
| `panels/KronosKontrol/lua/kkMain.lua` | 3 call sites now use `setComponentValue()`; explicit `kkEditShow()` on the EDITOR button |
| `panels/KronosKontrol/KronosKontrol_1.0i.panel` | rebuilt panel with the same Lua change embedded |

Build: `cmake --build <build-dir> --config Release --target CtrlrX_Standalone --parallel 8`
Deployed: `C:\Korg\CtrlrX-8.7.7-portable\CtrlrX.exe`
Panel deployed: `C:\Korg\CtrlrX-8.7.7-portable\KronosKontrol_1.0i_fixed.panel`
Commits: `9119ece` (engine), `7f938eb` (panel + provenance), `6c12d84` (this report)
