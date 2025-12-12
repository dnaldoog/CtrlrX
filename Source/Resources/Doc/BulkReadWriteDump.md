## HOW TO PROCESS BULK MIDI MESSAGES

### ENCODING TYPES:

- **EncodeNormal**  Single 7-bit byte 0-127
- **EncodeMSBFirst**  7-bit: MSB, LSB
- **EncodeLSBFirst**  7-bit: LSB, MSB
- **EncodeNibbleMsbFirst**  4-bit: MSB nibble, LSB nibble (unsigned)
- **EncodeNibbleLsbFirst**  4-bit: LSB nibble, MSB nibble (unsigned)
- **EncodeSignedNibbleMsbFirst**  4-bit: MSB nibble, LSB nibble (signed int8)
- **EncodeSignedNibbleLsbFirst**  4-bit: LSB nibble, MSB nibble (signed int8)
- **Encode16bitLsbFirst**
*Encodes a 16 - bit value as four 4 - bit nibbles, least significant first*
_Tokens_: `q0 q1 q2 q3`<br>
_Example_ : 51379 ? 03 0B 08 0C
- **Encode16bitMsbFirst** Encodes a 16 - bit value as four 4 - bit nibbles, most significant first.
_Tokens_: `Q0 Q1 Q2 Q3`
_Example_ : 51379 ? 0C 08 0B 03
<hr>
### Difference between mapped/non-mapped
**Non mapped**:<br>
  panel:getModulatorValuesAsData(CUSTINDEX, CtrlrPanel.EncodeNormal, 1, **false**)<br>
**Mapped**:<br>
  panel:getModulatorValuesAsData(CUSTINDEX, CtrlrPanel.EncodeNormal, 1, **true**)

<hr>
### EXAMPLE - SEND (4 bit nibble)
LSB/MSB two byte 4-bit nibble:
  panel:getModulatorValuesAsData(CUSTINDEX, CtrlrPanel.EncodeNibbleLsbFirst,
                                 2, false)

### EXAMPLE - RECEIVE (Where Header is 5 bytes in length):

  panel:setModulatorValuesFromData(midi:getData(), "modulatorCustomIndex",
                                   CtrlrPanel.EncodeMSBFirst, -5, 2, false)

  panel:setModulatorValuesFromData(midi:getData(), "modulatorCustomIndex",
                                   CtrlrPanel.EncodeNormal, -5, 1, false)

  panel:setModulatorValuesFromData(midi:getData(), "modulatorCustomIndex",
                                   CtrlrPanel.EncodeSignedNibbleMsbFirst,
                                   -54, 2, false)

## How to code Bulk Dump Send/Receive in lua
### STEP 1: CREATE TABLE OF MODULATORS IN SYSEX DUMP ORDER
```
  listOfModulators = {
      "lfoDelay",
      "lfoRate",
      "VCF Resonance",
      "VCF Cutoff",
      "Delay"
  }
```
List all modulators here in order of sysex message data position


### STEP 2: FILL modulatorCustomIndex WITH VALUES

You can create your own custom modulator property (e.g. "CUSTINDEX")
Run this in the console editor:
```
  local t = listOfModulators
  for i, v in ipairs(t) do
      panel:getModulatorByName(v):setProperty("CUSTINDEX", tostring(i - 1),
                                              false)
  end
```

### STEP 2b: REMOVE CUSTOM INDEX (UNDO)

You can completely remove the custom index you created:
```
  local t = listOfModulators
  for i, v in ipairs(t) do
      panel:getModulatorByName(v):removeProperty("CUSTINDEX")
  end
```

### STEP 3: SEND THE BULK MIDI MESSAGE

Create a (GLOBAL) header string and an EOX string:

  HEADER = "F0 41 00 00 11"
  EOX = "F7"
```
  local data = panel:getModulatorValuesAsData("CUSTINDEX",
                                              CtrlrPanel.EncodeNormal,
                                              1, false)
  panel:sendMidiMessageNow(CtrlrMidiMessage(string.format("%s %s %s",
                                                          HEADER,
                                                          data:toHexString(1),
                                                          EOX)))
```

### STEP 4: RECEIVE A MIDI MESSAGE

Create a method in 'Called when a panel receives a MIDI message':
```
  local headerSize = MemoryBlock(HEADER):getSize()
  panel:setModulatorValuesFromData(midi:getData(), "CUSTINDEX",
                                   CtrlrPanel.EncodeNormal,
                                   -headerSize, 1, false)
```
<span style="color:red">Note that headerSize = headerSize * -1</span>

The last argument of these methods when changed to true reads/writes
mapped values
<br>
<br>
