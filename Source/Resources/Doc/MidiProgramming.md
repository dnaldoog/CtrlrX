# MIDI Programming Guide

## Understanding MIDI Data

**MIDI** messages can only send 7 bits of data per byte. This means when you send a value to a MIDI device using one byte, you are limited to values 0–127.

### How Bits and Bytes Work

One byte of data consists of 8 bits, numbered from 0 to 7:

- **bit 7** (MSB - Most Significant Bit, leftmost) = 128 — Reserved for status
- **bit 6** = 64
- **bit 5** = 32
- **bit 4** = 16
- **bit 3** = 8
- **bit 2** = 4
- **bit 1** = 2
- **bit 0** (LSB - Least Significant Bit, rightmost) = 1

In MIDI **data bytes**, bit 7 must always be 0, which limits values to 0–127.

In binary, if you set all the data bits to 1, you get: `01111111`

This equals: `64 + 32 + 16 + 8 + 4 + 2 + 1 = 127`

Including `00000000` (which equals 0), we have 128 possible values: 0–127.

**Status bytes** have bit 7 set to 1, which means they range from 128–255 (`10000000` to `11111111`).

---

## MIDI Control Change (CC) Messages

A CC message consists of three bytes sent in order:

- **Byte 0** (Status byte): 176 for channel 1, 177 for channel 2, up to 191 for channel 16
- **Byte 1** (Controller number): 0–127
- **Byte 2** (Value): 0–127

For example, a CC message could be sent as: **176 127 127**

In programming, it's more convenient to view these values in hexadecimal (hex).

---

## What is Hexadecimal (Hex)?

Hexadecimal is a base-16 number system that uses 16 characters:

**0-1-2-3-4-5-6-7-8-9-A-B-C-D-E-F**

Where:
- **A** = 10
- **B** = 11
- **C** = 12
- **D** = 13
- **E** = 14
- **F** = 15

### Why Use Hex?

A byte (8 bits) can be conveniently split into two groups of 4 bits. Each group of 4 bits is called a **nibble**.

Since 4 bits can represent values 0–15, each nibble maps perfectly to one hex digit.

**Example 1:** The binary value `1111` equals 15, which is **F** in hex.

**Example 2:** The binary value `1100` equals 12 (8+4), which is **C** in hex.

### Understanding Nibbles

In a byte represented as two hex digits:

- The **upper nibble** (leftmost hex digit) is the **most significant nibble**
- The **lower nibble** (rightmost hex digit) is the **least significant nibble**

**Example:** The byte `11111111` in binary equals **FF** in hex:
- Upper nibble: `1111` = F = 15
- Lower nibble: `1111` = F = 15

To convert FF to decimal:
- Upper nibble: F × 16 = 15 × 16 = 240
- Lower nibble: F × 1 = 15 × 1 = 15
- **Total: 240 + 15 = 255**

### MIDI's 7-Bit Limit in Hex

Since MIDI data bytes are limited to 7 bits (bit 7 must be 0), the maximum value is `01111111` in binary, which is **7F** in hex:

- Upper nibble: 7 × 16 = 112
- Lower nibble: F × 1 = 15
- **Total: 112 + 15 = 127**

### CC Message in Hex

Our CC message **176 127 127** in decimal is represented in hex as: **B0 7F 7F**

- **B0** = 176 (status byte for CC on channel 1)
- **7F** = 127 (controller number)
- **7F** = 127 (value)

---

## System Exclusive (SysEx) Messages

SysEx messages allow manufacturers to send custom data to their devices.

### SysEx Structure

1. **Start byte:** F0 (240 decimal) — Always marks the beginning
2. **Manufacturer ID:** One or more bytes identifying the manufacturer
3. **Data bytes:** Custom data specific to the device
4. **End byte:** F7 (247 decimal) — Always marks the end

Both F0 and F7 are **status bytes** (bit 7 is set to 1), which is why they exceed 127.

**Important:** All data bytes between F0 and F7 must be in the range 0–127 (00–7F in hex). You cannot send values like 80 (128) or higher as a single data byte because bit 7 would be set to 1, making it a status byte.

---

## Sending Values Greater Than 127

Many parameters (like delay times, sample numbers, or memory addresses) require values greater than 127. Here are two common methods:

### Method 1: 4-Bit Nibbles (0–255 range)

Each byte is split into two nibbles, and each nibble is sent as a separate byte with the upper 4 bits set to 0.

**Example:** To send the value **254** (FE in hex):

1. Split FE into nibbles: **F** (upper) and **E** (lower)
2. Send as two bytes: **0F** and **0E**

The order depends on the device:
- **MSB first (ms ls):** 0F 0E
- **LSB first (ls ms):** 0E 0F

This method gives you 0–255 possible values (8 bits total).

### Method 2: 14-Bit Value (0–16,383 range)

The value is split into two 7-bit bytes:
- **MSB (Most Significant Byte):** Upper 7 bits
- **LSB (Least Significant Byte):** Lower 7 bits

**Example:** To send the value **254**:

1. 254 in binary is: `00000001 11111110` (using 9 bits)
2. Split into 7-bit portions:
   - **MSB:** `0000001` = 01 in hex
   - **LSB:** `1111110` = 7E in hex
3. Send as: **01 7E** (MSB first) or **7E 01** (LSB first)

**To calculate from 14-bit format back to decimal:**
- MSB first: (MSB × 128) + LSB = (1 × 128) + 126 = 254
- LSB first: (MSB × 128) + LSB = (1 × 128) + 126 = 254

This method gives you 0–16,383 possible values (14 bits total).

---

## Complete SysEx Examples

### Example 1: Sending 254 as 4-bit nibbles (LSB first)

```
F0 0E 0F F7
```

- F0 = SysEx start
- 0E = lower nibble (14)
- 0F = upper nibble (15)
- F7 = SysEx end
- Reconstructed value: (15 × 16) + 14 = 240 + 14 = **254**

### Example 2: Sending 256 as 4-bit nibbles (LSB first)

256 in hex is **100**, which splits as: upper nibble = **1**, lower nibble = **0**

```
F0 00 01 F7
```

- F0 = SysEx start
- 00 = lower nibble (0)
- 01 = upper nibble (1)
- F7 = SysEx end
- Reconstructed value: (1 × 16) + 0 = **16** — Wait, this is wrong!

**Note:** For values above 255, you need more than two nibbles or must use the 14-bit method.

### Example 3: Sending 256 as 14-bit value (MSB first)

256 in binary is `100000000` (9 bits needed)

Split into 7-bit portions:
- MSB: `0000010` = 02 in hex (2 in decimal)
- LSB: `0000000` = 00 in hex (0 in decimal)

```
F0 02 00 F7
```

- F0 = SysEx start
- 02 = MSB (2)
- 00 = LSB (0)
- F7 = SysEx end
- Reconstructed value: (2 × 128) + 0 = **256**

---

## Summary Table

| Method | Range | Token Names | Calculation |
|--------|-------|-------------|-------------|
| 4-bit nibbles | 0–255 | ms ls / ls ms | (upper × 16) + lower |
| 14-bit value | 0–16,383 | MS LS / LS MS | (MSB × 128) + LSB |

- **Lowercase (ms/ls):** 4-bit nibbles
- **Uppercase (MS/LS):** 14-bit values

---

## Negative Numbers

Negative numbers are represented in computers using **signed integers**. In a signed number, **bit 7** (the MSB - Most Significant Bit) of the byte acts as the **sign bit**:
- If bit 7 = **0**, the number is positive
- If bit 7 = **1**, the number is negative

### Two's Complement

Most computers use a system called **two's complement** to represent negative numbers.

For example, in an 8-bit signed system:
- `01111111` (7F in hex) = +127 (the maximum positive value)
- `10000000` (80 in hex) = -128 (the minimum negative value)
- `11111111` (FF in hex) = -1
- `11111110` (FE in hex) = -2

A **signed** 8-bit number has a range of **-128 to +127**.

### Why This Matters for MIDI

Because MIDI data bytes must have bit 7 set to 0 (limiting values to 0-127), **MIDI cannot directly transmit signed values** in a single byte. 

If a device needs to send a negative number via MIDI, it must use one of these approaches:

1. **Offset method:** Add an offset to make all values positive. For example, a range of -50 to +50 can be represented as 0 to 100 (where 50 = zero, 0 = -50, and 100 = +50). Roland devices commonly use this approach.

2. **Multi-byte method:** Use multiple bytes with a sign indicator byte, or use 14-bit values with custom interpretation.

3. **SysEx with sign byte:** Send a separate byte indicating whether the value is positive or negative, followed by the absolute value.

### Example 1: Roland's Offset Method

Roland often represents a parameter range of **-64 to +63** as MIDI values **0 to 127**:
- MIDI value **64** = 0 (center/zero point)
- MIDI value **0** = -64
- MIDI value **127** = +63

This allows negative values to be transmitted within MIDI's 0-127 data byte limitation.

### Example 2: Korg's 4-Bit Nibble Method

Korg devices often transmit signed values by breaking them into 4-bit nibbles, typically sent LSB first.

**Sending -1:**
- -1 in two's complement = `11111111` (FF in hex)
- Split into nibbles: upper = F, lower = F
- Sent as two bytes: **0F 0F** (LSB first)

**Sending -2:**
- -2 in two's complement = `11111110` (FE in hex)
- Split into nibbles: upper = F, lower = E
- Sent as two bytes: **0E 0F** (LSB first)

**Sending -128:**
- -128 in two's complement = `10000000` (80 in hex)
- Split into nibbles: upper = 8, lower = 0
- Sent as two bytes: **00 08** (LSB first)

This method preserves the signed value representation while keeping each transmitted byte within MIDI's 0-127 range.

---

## Key Terminology

- **Bit:** A single binary digit (0 or 1)
- **Byte:** 8 bits grouped together
- **Nibble:** 4 bits (half a byte)
- **MSB (Most Significant Bit):** The leftmost bit in a binary number (bit 7 in a byte)
- **LSB (Least Significant Bit):** The rightmost bit in a binary number (bit 0 in a byte)
- **MSB (Most Significant Byte):** In multi-byte values, the byte representing the larger portion
- **LSB (Least Significant Byte):** In multi-byte values, the byte representing the smaller portion
- **Status Byte:** A MIDI byte with bit 7 set to 1 (values 128–255)
- **Data Byte:** A MIDI byte with bit 7 set to 0 (values 0–127)
