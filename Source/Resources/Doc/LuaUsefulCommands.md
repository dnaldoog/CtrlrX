# lua useful commands

### Tables

<span style="color:blue">indexed tables start at 1</span>

```
local t={
"lfo",
"vcf",
"vca",
"resonance"
}
```
<span style="color:green">Loop through table with **ipairs**</span>
```
for i,v in ipairs(t) do
panel:getModulatorByName(v):setProperty("modulatorCustomIndex",i,true)
end
```
<hr>
<span style="color:blue">Key/Value pair tables will print in no set order</span>

```
local t={
["lfo delay"]=100,
vcf=panel:getModulatorByName("Vcf"):getModulatorValue(),
vca=20,
resonance=0
}
```
<span style="color:green">Loop through table with **pairs**</span>
```
for k,v in pairs(t) do
console(string.format("Key %s=value %d",k,v))
end
```

### Get channel out
```
    local channelOut = panel:getProperty("panelMidiOutputChannelDevice") - 1
```
### Block/Allow MIDI
```
    panel:setPropertyInt("panelMidiPauseOut",1)  -- MIDI cannot be sent
    panel:setPropertyInt("panelMidiPauseOut",0)  -- reenable MIDI can be sent

```
### Loop through all modulators
```
    local n = panel:getNumModulators()
    for i = 0, n - 1 do
        mod = panel:getModulatorByIndex(i)
        local name = L(mod:getName())
        local group = mod:getComponent():getProperty("componentGroupName")
        if group == "FXGROUP" then
            table.insert(t, name)
        end
    end
```
### Generate VST Index
- First clear all current VST indexes
```
    local n = panel:getNumModulators()
    for i = 0, no - 1 do
        --CLEAR ALL FIRST
        mod = panel:getModulatorByIndex(i)
        mod:setPropertyString("vstIndex", "")
        mod:setPropertyString("modulatorVstExported", "0")
    end
```
- Where t= lua table of modulators in vst order
```

 for i, v in ipairs(t) do
        local n = panel:getModulatorByName(v)
        n:setPropertyString("vstIndex", tostring(i))
        n:setPropertyString("modulatorVstExported", "1")
        console(String(string.format('"%s", --[[ set to %d--]]', v, i - 1)))
    end
```
### Timers
- Set timer to run
```
    timer:setCallback(1, TimerCallback) -- TimerId 1
    timer:startTimer(1, 800) -- Timer Id 1 fires after 800ms
```
- Timer code that will run after 800ms
```	
TimerCallback = function(timerId)
    if timerId == 1 then
       utils.warnWindow("Hello","World")
        timer:stopTimer(timerId)
	end
end
```
### Get modulator value
```
panel:getModulatorByName("lfo"):getModulatorValue()
```
### Set modulator value
```
value=20
panel:getModulatorByName("lfo"):setModulatorValue(false,true,false,value)
```
### Load Data as JSON
```
loadJson = function()
    -- File open dialog
    local jsonfile =
        utils.openFileWindow(
        "Select preset...",
        File.getSpecialLocation(File.userHomeDirectory),
        "*.json",
        true
    )

    if jsonfile:existsAsFile() then
        local str = jsonfile:loadFileAsString()
        obj = json.decode(str)
    end
    if type(obj) ~= "table" then
        return
    end -- bail out if something's not right
    for k, v in pairs(obj) do
        if data[k] then -- safeguard against non existent keys
            panel:getModulatorByName(k):setModulatorValue(v, false, true, false)
        end
    end
end
```
### Save data as JSON
```
data = {
    Pulse_Colour=true,
    Pulse_Level=true,
    Amp_Attack=true,
    Amp_Decay=true,
    Drive=true,
    Tone=true,
    Resonator_Pitch=true,
    Resonator_Bend=true,
    Resonator_Time=true,
    Accent=true,
}

savePreset = function()
    tmp = {} -- this table will converted to JSON and saved to file
    for key in pairs(data) do -- assign values to each item
        tmp[key] = panel:getModulatorByName(key):getModulatorValue()
    end
    -- Default filename
    local cPresName = "preset"

    -- File to save
    local jsonfile = utils.saveFileWindow("Save to file...", File(cPresName), "*.json", true)
    -- Writing data to the file
    jsonfile:replaceWithText(json.encode(tmp), true, true)
end
```
### Progress Bar example
```
function round(num, numDecimalPlaces) -- rounding function
  local mult = 10^(numDecimalPlaces or 0)
  return math.floor(num * mult + 0.5) / mult
end --function


startProgressBar = function(--[[ CtrlrComponent --]] comp --[[ MouseEvent --]], event)
    killtimer = false
    counter = 0
    repeatMe = tonumber(L(panel:getLabel("rev"):getText())) or 16
    local tDelay = tonumber(L(panel:getLabel("speed"):getText())) or 150
    panel:getComponent("PBAR"):setVisible(true)
    pbar = panel:getComponent("PBAR")
    pbar:setComponentValue(0, true) -- stops those rolling barber pole stripes
    pbar:setComponentText("Ready")
    for i = 1, repeatMe do
        timer:setCallback(3, timerCallback)
        timer:startTimer(3, tDelay)
    end
end

```
### Show/Hide layers
```
- Here we set layer WAVE to visible hding other layers

showLayer("WAVE")

layers={PULSE=true,FM=true,WAVE=true,NOISE=true}
showLayer = function(LAYER)
    if boot == true then
        for k in pairs(layers) do
            panel:getCanvas():getLayerByName(k):setVisible(false)
        end -- loop
        panel:getCanvas():getLayerByName(LAYER):setVisible(true)
    end
end --function

```
### Popup menu
```
pu = function(mod, value)
	if panel:getBootstrapState() then
		return
	end

	m 	= PopupMenu()
	i	= Image(resources:getResourceAsImage("star"))
	sm 	= PopupMenu()
	sm:addItem(10, "Sub menu - item one", true, false, Image())
	sm:addItem(11, "Sub menu - item two", true, false, Image())
	sm:addItem(12, "Sub menu - item three", true, false, Image())
	sm:addItem(13, "Sub menu - item four", true, false, Image())

	m:addItem(1, "Item number one", true, false, Image())
	m:addItem(2, "Item number two (with icon)", true, false, i)
	m:addSeparator()
	m:addItem(3, "Item number three (ticked)", true, true, Image())
	
	m:addItem(4, "Item number four", true, false, Image())
	m:addSectionHeader ("Section header")
	m:addSubMenu("Submenu", sm, true, Image(), false, 0)

	ret = m:show(0,0,0,0)

	if ret ~= 0 then
		utils.infoWindow("Menu", "Selected item: "..ret)
	end
end
```
### Cache Ctrlr modulators as lua variables
```
<span style="color:red">Call</span>
_m["control"]:getModulatorValue()
_c["control"]:getProperty("modulatorCustomIndex")
_c["control"]:setProperty("modulatorCustomIndex",2,true)

mt = {}
mt.__call = function(t, k)
    return t[k]
end

mt.__index = function(t, k)
    if myDebug then
        assert(panel:getModulator(k),sf("\n\npanel:getComponent(\"%s\") not found!", k))
    end
    local uiType = panel:getComponent(k):getProperty("uiType")
    if uiType == "uiLabel" then
        t[k] = panel:getLabelComponent(k)
    elseif uiType == "uiLCDLabel" then
        t[k] = panel:getLCDLabelComponent(k)
    elseif uiType == "uiToggleButton" then
        t[k] = panel:getToggleButtonComponent(k)
    elseif uiType == "uiButton" then
        t[k] = panel:getButtonComponent(k)
    elseif uiType == "uiImageButton" then
        t[k] = panel:getImageButtonComponent(k)
    elseif uiType == "uiCombo" then
        t[k] = panel:getComboComponent(k)
    elseif uiType == "uiListBox" then
        t[k] = panel:getListBoxComponent(k)
    elseif uiType == "uiFileListBox" then
        t[k] = panel:getSliderComponent(k)
    elseif uiType == "uiImageSlider" then
        t[k] = panel:getImageSliderComponent(k)
    elseif uiType == "uiFixedSlider" then
        t[k] = panel:getFixedSliderComponent(k)
    elseif uiType == "uiFixedImageSlider" then
        t[k] = panel:getFixedImageSliderComponent(k)
    elseif uiType == "uiCustomComponent" then
        t[k] = panel:getComponent(k)
    elseif uiType == "uiProgressBar" then
        t[k] = panel:getComponent(k)
    elseif uiType == "uiTabs" then
        t[k] = panel:getComponent(k)
    else
        t[k] = panel:getModulatorByName(k)
    end

    return t[k]
end

_c = {}

setmetatable(_c, mt)

mtm = {}
mtm.__call = function(t, k)
    return t[k]
end

```