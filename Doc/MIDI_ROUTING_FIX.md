# MIDI routing and timing fixes

**Date:** 11 November 2026
**Engine:** CtrlrX 8.0.79 / JUCE 8
**Branch:** `fix/midi-routing` (includes the VST parameter fix from PR #7)
**Status:** engine fixes complete; Windows suite **44/44**; CTest 44/44; panel probes and GUI smoke passed; exporter rebuilt; portable beta updated
**Upstream:** nothing sent for this MIDI change

## 1. Scope and result

The eight MIDI failures previously listed in `Tests/known_failures_windows.txt` are fixed on
Windows. The full suite now reports:

```text
44 tests from 6 test suites
[ PASSED ] 44 tests
```

The Windows allowlist has been cleared. The Linux and macOS allowlists are left untouched until
those platforms are re-run.

The failures split into three groups:

1. host MIDI sample positions were mangled through `MidiMessageCollector` (five tests);
2. host input was echoed independently of H2D/H2H/pause routing (two tests);
3. LUA multi-message/NRPN reassembly never matched because of pattern/data and channel setup
   (one test).

## 2. Group A — host sample positions

### Failures

- `ProcessorInstance.midi_block_processing_without_panel`
- `TestPanelGroup/...test_panel_midi_block_processing_with_notes/0`
- `TestPanelGroup/...test_panel_midi_block_processing_with_notes/1`
- `TestPanelGroup/...test_panel_midi_block_processing_with_cc_as_defined_in_panel/0`
- `TestPanelGroup/...test_panel_midi_block_processing_with_cc_as_defined_in_panel/1`

Before the fix, positions were `0` or `1023` instead of the expected values around the original
sample position.

### Root cause

Host input is block-relative sample data. `CtrlrProcessor::processBlock` passed it into
`MidiMessageCollector`, whose `addMessageToQueue` treats the timestamp as absolute seconds:

```cpp
sampleNumber = (int) ((message.getTimeStamp() - 0.001 * lastCallbackTime) * sampleRate);
```

The code was setting `m.setTimeStamp(time)` where `time` was a sample index. The old
`time == 0 -> 1` workaround avoided the assert but produced meaningless positions.

### Fix

Host input is no longer sent through the collector as if it were live MIDI. `processBlock` keeps a
copy of the host input and writes it to the output with its original sample position. A
block-to-block elapsed-time correction is applied. A late block has already missed its nominal
boundary; applying the entire negative overshoot would pull every event a full block early, so this
controller deliberately damps late corrections to half. Early corrections are applied in full.
This is a late-block damping convention, not compatibility with the old broken sample-index path.

The host copy is emitted only when a panel actually routes host input to the host:

- no panel: direct pass-through;
- at least one non-paused panel with `panelMidiThruH2H`: pass-through once;
- paused input or no H2H panel: no host pass-through.

This also gives multi-panel H2H once, instead of once per panel.

## 3. Group B — duplicate and paused host echo

### Failures

- `MidiRouting.h2d_host_cc_reaches_device_once_and_host_once` — host got the CC twice;
- `MidiRouting.pause_in_blocks_host_echo` — host got the CC while `panelMidiPauseIn` was active.

### Root cause

`CtrlrProcessor::processBlock` unconditionally echoed every host event into the host output. For an
H2D panel, `CtrlrPanelProcessor` then called `owner.sendMidi(m)`, which sends to the device and also
queues a host copy when `panelMidiOutputToHost` is on. Result: two host messages.

For pause-in, `CtrlrPanel::queueMessageForHostOutput` correctly checked the pause flag, but the
processor-level unconditional echo happened before that and ignored it.

### Fix

- Added `CtrlrPanel::sendMidiToDevice()` for the H2D path. H2D now sends device MIDI without adding
  a host copy.
- H2H host output comes from the single gated host pass-through described above.
- `CtrlrPanelProcessor::processBlock` returns immediately while `panelMidiPauseIn` is active. This
  suppresses the entire per-block host-input path: the host comparator, H2D/H2H routing and the Lua
  audio callback. Device input runs on its own thread and UI/value-driven sends run outside
  `processBlock`, so neither is stopped by this guard.

## 4. Group C — LUA multi-message callback

### Failure

`MidiLua.device_input_invokes_lua_multiMidiReceived_once` expected one sentinel device send from
the LUA `multiMidiReceived` callback; it received zero.

### Root causes

Three separate issues blocked the match:

1. **No input device in the fixture.** `fixture_lua_multi_received.panel` had
   `panelMidiInputDevice="-- None"`, so injected device MIDI never reached the panel comparator.
   It now uses the same mock device as the passing single-message Lua test.
2. **Wrong channel in the fixture.** The multi target was emitted on channel 4 while the test
   injected channel 2. The fixture now sets `midiMessageChannelOverride="1"` and
   `midiMessageChannel="2.0"`. Here `"1"` is a boolean enable flag, not channel 1: it makes the
   target honor its configured channel 2 instead of the panel's current channel.
3. **Comparator compared the wrong representation.**
   - `CtrlrMidiMessage::getData()` returned an empty `LMemoryBlock` in this path, while
     `getMidiPattern()` contained the 12 assembled bytes.
   - `compareMemory(haystack, needle)` treats `0xFF` bytes in the **first** argument as wildcards.
     The comparator was passing the concrete input as the haystack and the wildcard pattern as the
     needle, so a valid NRPN sequence could never match.

   `CtrlrMidiInputComparatorMulti` now uses `getMidiPattern()` for assembly/size/cache/map
   comparison and passes the wildcard pattern first:

   ```cpp
   if (compareMemory(messageContainer.getMidiPattern(), key))
   ```

The multi comparator now reaches `CtrlrPanel::multiMidiReceived`, queues the callback, and the LUA
sentinel is emitted once. Both the map and cache call sites, plus the `compareMemory` definition,
now document the pattern-first convention.

## 5. Test-side changes

- `Tests/test_ProcessorPanel.cpp`: the two H2D parameterized tests pin the legitimate device sends
  exactly: 4 note sends and 6 CC sends across the two host rounds, and 0 for the H2H fixtures. Their
  existing exact `openOutput(0, 1)` expectation is preserved.
- `Tests/fixtures/fixture_lua_multi_received.panel`: mock input device set to
  `Device 00, port 00`; `midiMessageChannelOverride="1"`.
- `Tests/known_failures_windows.txt`: the eight MIDI entries removed.

No panel design or shipped panel was changed.

## 6. Verification status

Completed:

- Windows full suite: **44/44**
- CTest: **44/44**, 0 failures
- Clean rebuild of `TestsProcessor`
- KronosKontrol 1.0c panel probe: exit 0, 0 errors
- OasysOmni 1.0d panel probe: exit 0, 0 errors
- Standalone GUI smoke: KronosKontrol and OasysOmni both opened and remained responsive
- Standalone, VST3 and probe copied into `ctrlrx-8.0.79-portable`
- `CtrlrXExport.exe` rebuilt with the same fixes; EXE and VST3 preview exports both verified
  into the portable folder

Windows beta artifact hashes:

```text
AE6569CAE34215A36C9CA41A918508AE049CAF703C81B646930024C55B4B7EB0  CtrlrX.exe
EA9F7054D5DEFFD01ED010566BB7295B99342992A5A88126595A68AFD73B0392  vst3\CtrlrX.vst3
25B4F66E112D1DF34D3A74C8B4095B3A0002BE6F0160081C79A4DE980F0BFFC1  probe\CtrlrPanelProbe.exe
71D3D96253A7E35DD664B5DD6E467B8B05A239A80CFA67AA661F7F56D0FF0BD2  CtrlrXExport.exe
```

Pending:

- post-fix DAW/REAPER automation and MIDI validation by the maintainer;
- cross-platform CI (Linux/macOS allowlists still contain the old entries).

## 7. Files changed

- `Source/Plugin/CtrlrProcessor.cpp`
- `Source/Plugin/CtrlrProcessor.h`
- `Source/Core/CtrlrPanel/CtrlrPanel.cpp`
- `Source/Core/CtrlrPanel/CtrlrPanel.h`
- `Source/Core/CtrlrPanel/CtrlrPanelProcessor.cpp`
- `Source/MIDI/CtrlrMidiInputComparatorMulti.cpp`
- `Tests/test_ProcessorPanel.cpp`
- `Tests/fixtures/fixture_lua_multi_received.panel`
- `Tests/known_failures_windows.txt`

## 8. State

- MIDI fixes committed on `fix/midi-routing` as `ff3f144c0`.
- The VST parameter fix (PR #7) is unchanged.
- Portable beta artifacts updated in `ctrlrx-8.0.79-portable`.
- No MIDI note or PR has been sent upstream.
