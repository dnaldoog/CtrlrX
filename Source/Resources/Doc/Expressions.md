Constants:
modulatorValue : The current linear value of the modulator, this is the index of the
array of values; is always positive.
modulatorMappedValue : The current mapped value in case of components that have
mappings. This might be negative.
modulatorMax : The maximum value the modulator can have (non mapped)
modulatorMin : The minimum value the modulator can have (non mapped)
modulatorMappedMax : the maximum value the modulator can have (mapped)
modulatorMappedMin : the maximum value the modulator can have (mapped)
vstIndex : The VST/AU index of the parameter as seen by the host program
midiValue : The current value stored in the MIDI MESSAGE assosiated with the
modulator.
midiNumber : The number of the MIDI MESSAGE controller if applicable
Functions:
ceil(x) : Returns the smallest integral value of the parameter
abs(x) : Returns the absolute value of the parameter
floor(x) : Returns the largest integral value that is not greater than the parameter
mod(a,b) : Divides two numbers and returns the result of the MODULO operation “%”.
Examples 10 % 3 = 1, 0 % 5 = 0; 30 % 6 = 0; 32 % 5 = 2 For more info
fmod(numerator,denominator) : Returns the floating-point remainder of the two
parameters passed in
pow(a,b) : Returns the first parameter raised to the power of the second (a^b)
gte(a,b,retTrue,retFalse) : Return the larger or equal of the two passed
parameters (a >= b). For example
gte (modulatorValue, 0, modulatorValue, 128 - modulatorValue) will return
modulatorValue if modulatorValue is greater then 0 and (128 – modulatorValue) if it is
less then zero
gt(a,b,retTrue,retFalse) : Same as gte but greater then without the equal sign (a
> b)
lt(a,b,retTrue,retFalse) : Same as gte but less then (a < b)
lte(a,b,retTrue,retFalse): Same as gte but less then or equal (a <= b)
eq(a,b,retTrue,retFalse) : Equals sign true if (a == b)
max(a,b) : Returns the bigger of two parameters.
min(a,b) : Returns the smaller of two parameters.
getBitRangeAsInt (value, startBit, numBits) : Gets a number of bits (numBits)
starting at position startBit as an Integer and returns that integer.
setBitRangeAsInt (value, startBit, numBits, valueToSet) :
clearBit (value, bitToClear) : Clears a bit at position bitToClear in the value and
return that modified value.
isBitSet (value, bitPosition) : Return true if a bit at position bitPosition in value
is set, false otherwise.
setBit (value, bitToSet) : Sets one bit in an integer at position (bitToSet) and
returns the modified value with the bit set.
setGlobal (globalIndex, newValueToSet) : This sets the value of one of the global
variables in the panel, and returns that set value so the expression can continue.
