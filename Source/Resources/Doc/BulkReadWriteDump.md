## HOW TO PROCESS BULK MIDI MESSAGES

Of course, you can create your own MIDI parsing functions by looping over an incoming MIDI hexstring and assigning each modulator with data, using perhaps a lua lookup with byte data positions and assigning with something like:
`panel:getModulatorByName(n):setModulatorValue(value,false,false,false)`, but CtrlrX offers another, not so well known, way of performing the same task.
<br>
<br>
## How to code Bulk Dump Send/Receive in lua
<br>

### STEP 1: CREATE TABLE OF MODULATORS IN SYSEX DUMP ORDER
<br>
<br>
The first item in the table represents byte 1, so you may need to offset it from the header:
Imagine your header Hex string is `F0 41 00 00 11` and data starts at byte 5.
We create a lua variable `local offset=5`

```
  listOfModulators = {
      "lfoDelay",
      "lfoRate",
      "VCF Resonance",
      "VCF Cutoff",
      "Delay",
      "High Cut",
      "Low Cut"
  }
```
List all modulators here in order of sysex message data position
<br>
<br>

### STEP 2: FILL modulatorCustomIndex WITH VALUES
<br>
<br>
Assign a panel property e.g. `modulatorCustomIndex` which will store the byte positions, but be sure all other modulators that are not in the list do not already have a value stored in `modulatorCustomIndex`
<br>
Actually, it's probably better to create your own custom property, which you can do easily in Ctrlr.
Let's create a custom property for all modulators that need to be updated with data from a MIDI dump.
We shall call it "**incomingMidiByte**"
<br>

Run this in the console editor:
```
  local offset=5
  local t = listOfModulators
  for i, v in ipairs(t) do
      panel:getModulatorByName(v):setProperty("incomingMidiByte", tostring(i + offset),
                                              false)
  end
```
<br>
<br>
### STEP 2b: Remove custom property
<br>
<br>
You can completely remove the custom index you created:
<br>

```
  local t = listOfModulators
  for i, v in ipairs(t) do
      panel:getModulatorByName(v):removeProperty("incomingMidiByte")
  end
```
<br>
<br>
### STEP 3: SEND THE BULK MIDI MESSAGE
<br>
<br>
Here we create a lua variable for the header and EOX:
<br>
  local header = "F0 41 00 00 11"
  local EOX = "F7"
```
  local data = panel:getModulatorValuesAsData("incomingMidiByte",
                                              CtrlrPanel.EncodeNormal,
                                              1, false)
  panel:sendMidiMessageNow(CtrlrMidiMessage(string.format("%s %s %s",
                                                          header,
                                                          data:toHexString(1),
                                                          EOX)))
```
<br>
<br>
### STEP 4: RECEIVE A MIDI MESSAGE
<br>
<br>
Create a method in 'Called when a panel receives a MIDI message':
<br>

```
  local headerSize = MemoryBlock(header):getSize()
  panel:setModulatorValuesFromData(midi:getData(), "incomingMidiByte",
                                   CtrlrPanel.EncodeNormal,
                                   -headerSize, 1, false)
```

<span style="color:red">negate the header size: e.g. headerSize `-header`</span>

The last argument of these methods when changed to true reads/writes
mapped values (_See below for more detail_)

<HR>
<br>
<br>
### ENCODING TYPES:

- **EncodeNormal**  Single 7-bit byte 0-127
- **EncodeMSBFirst**  7-bit: MSB, LSB
- **EncodeLSBFirst**  7-bit: LSB, MSB
- **EncodeNibbleMsbFirst**  4-bit: MSB nibble, LSB nibble (unsigned)
- **EncodeNibbleLsbFirst**  4-bit: LSB nibble, MSB nibble (unsigned)
- **EncodeSignedNibbleMsbFirst**  4-bit: MSB nibble, LSB nibble (signed int8)
- **EncodeSignedNibbleLsbFirst**  4-bit: LSB nibble, MSB nibble (signed int8)
<br>
- **Encode16bitLsbFirst**
*Encodes a 16 - bit value as four 4 - bit nibbles, least significant first*
_Tokens_: `q0 q1 q2 q3`<br>
_Example_ : 51379 ? 03 0B 08 0C
- **Encode16bitMsbFirst** Encodes a 16 - bit value as four 4 - bit nibbles, most significant first.
_Tokens_: `Q0 Q1 Q2 Q3`
_Example_ : 51379 ? 0C 08 0B 03

<hr>
<br>
<br>
### Difference between mapped/non-mapped
<br>
<br>
**Non mapped**:<br>
  panel:getModulatorValuesAsData(CUSTINDEX, CtrlrPanel.EncodeNormal, 1, **false**)<br>
**Mapped**:<br>
  panel:getModulatorValuesAsData(CUSTINDEX, CtrlrPanel.EncodeNormal, 1, **true**)

<hr>
<br>
<br>
### EXAMPLE - SEND (4 bit nibble)
<br>
<br>
LSB/MSB two byte 4-bit nibble:
  panel:getModulatorValuesAsData(CUSTINDEX, CtrlrPanel.EncodeNibbleLsbFirst,
                                 2, false)
<br>
<br>
### EXAMPLE - RECEIVE (Where Header is 5 bytes in length):
<br>
<br>
  panel:setModulatorValuesFromData(midi:getData(), "modulatorCustomIndex",
                                   CtrlrPanel.EncodeMSBFirst, -5, 2, false)

  panel:setModulatorValuesFromData(midi:getData(), "modulatorCustomIndex",
                                   CtrlrPanel.EncodeNormal, -5, 1, false)

  panel:setModulatorValuesFromData(midi:getData(), "modulatorCustomIndex",
                                   CtrlrPanel.EncodeSignedNibbleMsbFirst,
                                   -54, 2, false)

<br>
<br>
