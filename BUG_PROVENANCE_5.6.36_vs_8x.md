# Did CtrlrX 8.x break the panel's tab control? — version comparison

**Question:** the EDITOR button worked in CtrlrX 5.6.36 and does nothing in 8.7.7.
Is that a regression in the 8.x line (i.e. something dnaldoog must fix), or was the
panel relying on something that never existed?

**Answer: it is a regression in the CtrlrX 8.x line.** The behaviour the panel relies on
is present and working in 5.6.36, 5.6.37 and the original Ctrlr; it is absent from 8.66
and 8.77.

> Companion document: `EDITOR_TAB_BUG_REPORT.md` — the defect analysis, fix and
> verification. This document only establishes **where** the regression came from.

---

## 1. Method

Several engine trees happen to exist on this machine, which allowed a direct source
comparison rather than speculation or binary archaeology. For each tree I inspected
`Source/UIComponents/CtrlrComponents/Groups/CtrlrTabsComponent.cpp` and looked for the
three things the panel needs:

1. a **reader** for `Ids::uiTabsCurrentTab` (the property the panel sets),
2. a **slot** that resolves `uiTabsCurrentTabChanged` into `tabChangedCbk`,
3. an **invocation** of `tabChangedCbk`.

| Tree | `VERSION` |
|---|---|
| `ctrlr-project\build\ctrlr-master\ctrlr-master` | (no VERSION file — original Ctrlr) |
| `ctrlr-project\build\CtrlrX-damiensellier\CtrlrX` | **5.6.36** |
| `ctrlr-project\build\CtrlrX-pr337` | **5.6.37** |
| `ctrlr-project\probe\src` | **5.6.36** |
| `ctrlr-project\build\CtrlrX-dnaldoog\CtrlrX-master` | **8.66** |
| `ctrlrx-juce8\build\CtrlrX-dnaldoog\CtrlrX-master` | **8.77** (the build under test, pre-fix) |

The 5.6.36 installer (`C:\Users\Superdad\Downloads\CtrlrX-5.6.36-Windows.exe`) was not
needed and was not decompiled — the sources were available.

---

## 2. Results

| Tree | Version | Reads `uiTabsCurrentTab` | Resolves `tabChangedCbk` | **Invokes** `tabChangedCbk` |
|---|---|---:|---:|---:|
| ctrlr-master (original) | — | **yes** | yes | **yes** |
| CtrlrX-damiensellier | 5.6.36 | **yes** | yes | **yes** |
| CtrlrX-pr337 | 5.6.37 | **yes** | yes | **yes** |
| probe/src | 5.6.36 | **yes** | yes | **yes** |
| CtrlrX-dnaldoog | 8.66 | **no** | yes | **no** |
| CtrlrX-dnaldoog | 8.77 | **no** | yes | **no** |

File size tells the same story at a glance: the 5.6.x files are ~695 lines, the 8.x files
~598. About 97 lines were removed from this component, and the tab-input branch was among
them.

---

## 3. What 5.6.36 did

`CtrlrX-damiensellier\CtrlrX\...\CtrlrTabsComponent.cpp`, lines 308-330:

```cpp
else if (property == Ids::uiTabsCurrentTabChanged)
{
    if (getProperty(property) == "")
        return;

    tabChangedCbk = owner.getOwnerPanel().getCtrlrLuaManager().getMethodManager().getMethod(getProperty(property));
}

else if (property == Ids::uiTabsCurrentTab)
{
    if (ctrlrTabs->getCurrentTabIndex() != (int)getProperty (property))
    {
        ctrlrTabs->setCurrentTabIndex (getProperty(property), false);
    }

    if (tabChangedCbk && !tabChangedCbk.wasObjectDeleted())
    {
        if (tabChangedCbk->isValid())
        {
            owner.getOwnerPanel().getCtrlrLuaManager().getMethodManager()
                 .call (tabChangedCbk, &owner, (int)getProperty(property));
        }
    }
}
```

Two consequences, both of which the panel depends on:

1. Setting `uiTabsCurrentTab` on the component **moves the tab bar**.
2. Setting it also **fires the Lua callback** named by `uiTabsCurrentTabChanged`.

Notice `currentTabChanged()` in 5.6.36 only wrote the property (line 184). Manual tab
clicks therefore *also* flowed through this same branch — write property → handler →
switch (no-op) → callback. One path for both cases, which is why the panel works whether
the user clicks EDITOR or the tab itself.

---

## 4. What 8.x does instead

`CtrlrX-dnaldoog` 8.66 and 8.77 are structurally identical here:

```cpp
// 8.x — currentTabChanged(), output only
void CtrlrTabsInternal::currentTabChanged(int newCurrentTabIndex, const String &newCurrentTabName) {
    owner.setProperty(Ids::uiTabsCurrentTab, newCurrentTabIndex);
}
```

```cpp
// 8.x — the resolution survived …
else if (property == Ids::uiTabsCurrentTabChanged) {
    if (getProperty(property) == "")
        return;
    tabChangedCbk = owner.getOwnerPanel()...getMethod(getProperty(property));
}
// … but there is no branch for Ids::uiTabsCurrentTab, and no call to tabChangedCbk.
```

`grep` for `uiTabsCurrentTab` in the 8.66 and 8.77 component finds exactly **one** site:
the write in `currentTabChanged()`. The property became an output with no input.

---

## 5. Is it really "a bug in dnaldoog's version"?

**Yes, in the sense that matters: the 8.x tree is where the behaviour disappeared, and it
is the tree that ships.** A panel built for 5.6.x uses a documented, working mechanism;
8.x silently accepts the call and does nothing. No error, no deprecation, no log line.

The strongest argument that this was **unintentional** rather than a deliberate API
removal:

- `tabChangedCbk` is still **resolved** in 8.x — the identifier, the `WeakReference`
  member and the lookup code were all kept — but the resolved callback is never invoked,
  in any component: a full-tree search finds only the assignment and the declaration.
  Dead code of this exact shape is what an interrupted refactor looks like.
- If input via `uiTabsCurrentTab` had been intentionally dropped, the natural cleanup
  would have been to remove the resolution code too, or to warn.

It is nevertheless a **behavioural** API change whether or not it was intended, and any
panel written before 8.x that drives tabs this way will fail the same way this one did.

---

## 6. What I could not determine

- **The exact version where it broke.** 5.6.37 has it, 8.66 does not; there are no
  intermediate snapshots on this machine (8.7x versions 8.70-8.76 are not present as
  source). Pinning it down would need those tags from the upstream repository.
- **Intent.** No comment, changelog entry or deprecation note was found. `CHANGELOG.md`
  in 8.77 mentions neither the property nor the callback.
- **Whether upstream 8.77 differs from the copy we have.** Our tree came from the
  dnaldoog repository export; if his branch has since changed this file, the comparison
  should be repeated.

---

## 7. Who fixes what

| Layer | Action | Status |
|---|---|---|
| **dnaldoog / CtrlrX 8.x** | Restore the `uiTabsCurrentTab` input branch **and** invoke `tabChangedCbk`; or document the removal and tell panel authors to migrate | **recommended upstream** |
| Our engine build | Same two changes implemented in `CtrlrTabsComponent.{h,cpp}` | done, `9119ece` |
| The panel itself | Drive the tab with `setComponentValue(n, false)` and refresh explicitly, so it also works on a stock 8.7.7 | done — `KronosKontrol_1.0i_fixed.panel` |

Upstream has a legitimate choice to make about the property, which is why both layers were
fixed:

- If the property is meant to stay output-only, panels should call
  `component:setComponentValue(n, false)` (exposed at `CtrlrComponent.cpp:727-728`) and the
  tab callback should be deleted so nobody depends on it.
- If backward compatibility is wanted, the 5.6.36 branch can be restored almost verbatim;
  the fix here is equivalent but routes through `setComponentValue()` instead of touching
  the `TabbedButtonBar` directly, and it suppresses the callback during panel restore.

---

## Appendix — how to reproduce this comparison

```powershell
$trees = @(
  'C:\Korg\kronos-corpus\ctrlr-project\build\ctrlr-master\ctrlr-master',
  'C:\Korg\kronos-corpus\ctrlr-project\build\CtrlrX-damiensellier\CtrlrX',
  'C:\Korg\kronos-corpus\ctrlr-project\build\CtrlrX-dnaldoog\CtrlrX-master',
  'C:\Korg\kronos-corpus\ctrlr-project\build\CtrlrX-pr337'
)
foreach ($t in $trees) {
  $v = Join-Path $t 'VERSION'
  $ver = if (Test-Path $v) { (Get-Content $v -Raw).Trim() } else { '(none)' }
  $f = Join-Path $t 'Source\UIComponents\CtrlrComponents\Groups\CtrlrTabsComponent.cpp'
  $input = (Select-String -Path $f -Pattern 'property == Ids::uiTabsCurrentTab\b' | Measure-Object).Count
  $call  = (Select-String -Path $f -Pattern 'tabChangedCbk' | Where-Object { $_.Line -match '\.call\s*\(' } | Measure-Object).Count
  "{0,-18} {1,-10} inputBranch={2} cbkCalls={3}" -f (Split-Path $t -Leaf), $ver, $input, $call
}
```
