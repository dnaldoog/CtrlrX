# VST automation parameter-count defect

**Date:** 11 November 2026
**Engine:** CtrlrX 8.0.79 / JUCE 8
**Branch:** `fix/vst-parameter-count` (based on `dnaldoog/CtrlrX` `master` at `335798cab`)
**Status:** fix implemented and regression-tested; pull request prepared

---

## 1. Executive summary

CtrlrX tells a VST3/AU host how many parameters the plugin has when the host initialises the
instance. The old implementation could change that number later when a panel was loaded. The
VST3 wrapper had already built its parameter map from the initial count, so parameter notifications
above that count could index beyond the wrapper's map.

The existing guard from merged PR #324 checked the **512-slot internal parameter pool**, not the
count advertised to the host. It therefore did not stop the out-of-range notification.

The local fix:

- freezes one host-facing count at construction (with a single pre-host raise allowed for an
  already-embedded panel);
- makes `getNumParameters()` return the fixed advertised value;
- uses the advertised count for notification guards;
- allows an exported instance's embedded panel to raise the count once, before the host queries it,
  up to the 512-slot pool.

Current panels export 34 parameters (`vstIndex` 0–33), so they were not exposed in practice, but the
engine defect is real for arbitrary panels and for any future panel that needs more automation.

---

## 2. Observed failures

Two tests in `Tests/test_ProcessorVstParameters.cpp` failed before the fix and are **not** in
`Tests/known_failures_windows.txt`:

```text
ProcessorInstance.test_num_parameters_is_constant_across_panel_load
ProcessorInstance.test_out_of_range_vst_index_never_reaches_host
```

Observed failure 1:

```text
before: 64
expected: CTRLR_MAX_EXPORTED_VST_PARAMETERS (128)
after loading fixture_vst_param_overflow.panel: 201
```

Observed failure 2:

```text
advertised count: 64
largest index delivered to the AudioProcessorListener: 200
```

The third test, `ProcessorInstance.test_in_range_vst_index_still_reaches_host`, passed and is the
anti-regression guard: a fix must not simply block every parameter change.

Before the fix the full suite was:

```text
44 tests: 34 passed, 10 failed
8 failures already known/allowlisted
2 VST-parameter failures not allowlisted
```

---

## 3. Root cause

### 3.1 Internal slot pool

`CtrlrProcessor` registers `CTRLR_MAX_PARAMETER_SLOTS` (512) `CtrlrParameter` objects:

```cpp
for (int i = 0; i < numParamSlots; i++)
    addParameter (new CtrlrParameter (*this, i));
```

`AudioProcessor::getParameters().size()` is therefore 512.

### 3.2 Legacy advertised count

Before the fix, `CtrlrProcessor::getNumParameters()` returned:

- the largest exported VST index + 1 in some states, or
- `jmax(manager count, ctrlrMaxExportedVstParameters)` in others.

`CtrlrProcessor` also seeded the override tree with **64**, while
`CTRLR_MAX_EXPORTED_VST_PARAMETERS` is **128** in `CtrlrMacros.h`.

That produced the pre-fix values 64 before a panel and 201 after the overflow fixture.

### 3.3 JUCE VST3 wrapper snapshot

JUCE's `LegacyAudioParametersWrapper::update()` calls `getNumParameters()` and compares it with
`getParameters().size()`:

```cpp
auto numParameters = audioProcessor.getNumParameters();
usingManagedParameters = audioProcessor.getParameters().size() == numParameters;
```

With 64 vs 512 (or 128 vs 512), the wrapper creates 64/128 legacy parameter wrappers and caches
them. The host is never told about the later 201. The wrapper does not resize its map after a panel
load.

### 3.4 Why the merged PR #324 guard missed it

Commit `decc196f0` (merged PR #324) changed
`CtrlrModulatorProcessor::setParameterNotifyingHost()` to guard with:

```cpp
auto& params = getProcessor()->getParameters();
if (index >= params.size())
    return;
```

But `params.size()` is the internal 512-slot pool. Index 200 passes, and
`setValueNotifyingHost()` emits a notification the VST3 wrapper maps through its 64-entry IDs.

PR #324 was directionally correct but used the wrong bound. That is why the two VST-parameter
tests still fail on 8.0.79.

---

## 4. Local fix

Branch: `fix/vst-parameter-count`, three source files changed. The VST-parameter regression tests in
`Tests/test_ProcessorVstParameters.cpp` already existed; two were failing on 8.0.79 and the third
passed as the anti-regression guard. None are new files in this change.

### `Source/Plugin/CtrlrProcessor.h`

Added:

```cpp
int getAdvertisedParameterCount() const noexcept { return advertisedParameterCount; }
...
int advertisedParameterCount;
```

### `Source/Plugin/CtrlrProcessor.cpp`

1. Initialise the override default from the public maximum:
   ```cpp
   overridesTree.setProperty(Ids::ctrlrMaxExportedVstParameters,
                             CTRLR_MAX_EXPORTED_VST_PARAMETERS, 0);
   ```
2. Compute the advertised count:
   ```cpp
   const int configuredParameterCount = ...;
   advertisedParameterCount =
       jmax(CTRLR_MAX_EXPORTED_VST_PARAMETERS, configuredParameterCount);
   ```
3. Keep the 512-slot pool as-is; use the advertised count only for `getNumParameters()` and the
   notification guard.
4. After an exported instance's embedded panel is initialised, if the panel is already present,
   raise the advertised count once to its exported range, capped at 512. A panel loaded later cannot
   change it.
5. `getNumParameters()` now returns `advertisedParameterCount` unconditionally.

The advertised count now has a floor of **128**, matching `CTRLR_MAX_EXPORTED_VST_PARAMETERS` and
the tests. It can be higher through the override or an already-embedded panel. The constructor
previously seeded the override with 64, which was inconsistent with the public constant.

### `Source/Core/CtrlrModulator/CtrlrModulatorProcessor.cpp`

The notification guard now uses the host-facing count:

```cpp
const int advertised = processor->getAdvertisedParameterCount();

if (index >= advertised)
    return;

auto& params = processor->getParameters();
if (index >= params.size())
    return;
```

This keeps in-range gestures and automation working while refusing indices the host was never
given.

---

## 5. Test results after the fix

Targeted suite:

```text
ProcessorInstance.test_num_parameters_is_constant_across_panel_load   PASS
ProcessorInstance.test_out_of_range_vst_index_never_reaches_host     PASS
ProcessorInstance.test_in_range_vst_index_still_reaches_host         PASS
```

Full suite:

```text
44 tests: 36 passed, 8 failed
```

The 8 remaining failures are the pre-existing MIDI allowlist entries, unrelated to VST parameters:

- `TestPanelGroup/ProcessorInstanceWithPanel.test_panel_midi_block_processing_with_notes/0`
- `TestPanelGroup/ProcessorInstanceWithPanel.test_panel_midi_block_processing_with_notes/1`
- `TestPanelGroup/ProcessorInstanceWithPanel.test_panel_midi_block_processing_with_cc_as_defined_in_panel/0`
- `TestPanelGroup/ProcessorInstanceWithPanel.test_panel_midi_block_processing_with_cc_as_defined_in_panel/1`
- `ProcessorInstance.midi_block_processing_without_panel`
- `MidiRouting.h2d_host_cc_reaches_device_once_and_host_once`
- `MidiRouting.pause_in_blocks_host_echo`
- `MidiLua.device_input_invokes_lua_multiMidiReceived_once`

The targeted parameter fix is complete. The 8 MIDI failures are separate work and were not changed.

The defect was observed in REAPER while testing automation against the latest exported VST3
instance: the host-side parameter count changed after a panel load. That REAPER test identified the
bug; **post-fix DAW verification is still pending**.

### Build status

`TestsProcessor`, `CtrlrX_Standalone`, `CtrlrX_VST3` and `CtrlrPanelProbe` build successfully with
the fix. The local app binaries must be rebuilt/copied to the portable folder before distribution
testing; that has not been done as part of this note.

---

## 6. Compatibility and limitations

- Hosts now see **at least 128** parameters by default instead of 64.
- An exported instance whose embedded panel has up to 512 exported parameters can advertise that
  range at construction.
- A panel loaded **after** host initialisation cannot raise the count; exports above the advertised
  count are ignored by the guard. This is deliberate and matches host snapshot semantics.
- `getParameters()` still contains the 512 internal slot objects. JUCE uses the legacy wrapper path
  because the size differs from the advertised count; that is intentional and preserves flexibility
  for instance panels up to 512.
- No **post-fix** DAW host test has been run yet. The defect was observed in REAPER while testing
  automation against an exported VST3; the engine-level wrapper path is covered by the listener test,
  but post-fix VST3 host validation is still required before release.
- Current KronosKontrol/OasysOmni panels export 34 parameters each, so their existing automation is
  far below both 64 and 128.

---

## 7. Upstream context checked

- **PR #324** (merged, `decc196f0`, “Update to CtrlrModulatorProcessor::setParameterNotifyingHost”):
  added the guard but used `getParameters().size()`, which is 512. This is the partial fix that did
  not close the defect.
- **Issue #229** — “CtrlrX 8.0.0 UPDATE Parameter to Host Logic”: related host parameter design.
- **Issue #31** — max exported VST parameter count discussion (closed/historical).
- **Issue #342** — active JUCE 8 migration plan; upstream has no open pull requests at the time of
  this note.

The PR should reference #229 and PR #324, and explain that this is a follow-up to the merged but
incomplete guard.

---

## 8. Remaining verification

The final targets and full engine suite are already verified (§5). Remaining checks:

1. Run all Kronos/OASYS panel probes.
2. GUI smoke-test both panels.
3. Load the exported VST3 in a DAW and verify:
   - parameter list is stable across panel load;
   - 0–127 automation reaches the host;
   - out-of-range indices do not crash or corrupt the host.
4. Prepare and review the upstream diff/patch; do not push until authorised.

---

## 9. Files and evidence

- `Tests/test_ProcessorVstParameters.cpp`
- `Tests/known_failures_windows.txt`
- `Source/Plugin/CtrlrProcessor.cpp`
- `Source/Plugin/CtrlrProcessor.h`
- `Source/Core/CtrlrModulator/CtrlrModulatorProcessor.cpp`
- `JUCE/modules/juce_audio_processors_headless/format_types/juce_LegacyAudioParameter.h`
- `JUCE/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp`

## 10. State

- Fix implemented locally on `fix/vst-parameter-count`.
- Tests pass for the targeted defect.
- No upstream push, issue comment or pull request has been made.
- Waiting for maintainer review before any upstream action.
