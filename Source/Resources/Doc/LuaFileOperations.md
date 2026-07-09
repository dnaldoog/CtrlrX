# lua file operations save/load
<hr>

### Load file as text

```
readFileAsText = function(mod, value)
	fileToRead = utils.openFileWindow(
		"Open file to read as text",
		File.getSpecialLocation(File.userHomeDirectory),
		"*.*",
		true
	)

	if fileToRead:existsAsFile() then

		console(fileToRead:loadFileAsString())
	end
end
```
<hr>

### Load file as data

```
readFileAsData = function(mod, value)
    fileToRead =
        utils.openFileWindow("Open file to read as data", 
        File.getSpecialLocation(File.userHomeDirectory), "*.syx", 
        true)

    if fileToRead:existsAsFile() then
    fileData = MemoryBlock()
  --  fileData = MemoryBlock(fileToRead:getSize())
    fileToRead:loadFileAsData(fileData)
    console(string.format("%s\n%d bytes", fileData:getRange(0, 16):toHexString(1), fileData:getSize()))
end
end
```
<hr>

### Save file as text

```
myText = "a string"
saveContentAsText = function(mod, value)
	fileToWrite = utils.saveFileWindow(
		"Save content as text",
		File.getSpecialLocation(File.userDesktopDirectory),
		"*.txt",
		true
	)

	if fileToWrite:isValid() == false then
		return
	end

	-- Let's see if the file exists
	if fileToWrite:existsAsFile() == false then
		
		-- The file is not there, that's ok, let's try to create it
		if fileToWrite:create() == false then

			-- Ooooh we can't create it, we need to fail here
			utils.warnWindow ("File write", "The destination file does not exist")

			return
		end
	end

	textToWrite = myText

	if textToWrite:length() == 0 then
		utils.warnWindow ("Data to write", "There is no data to write")
		textToWrite = "OK"
	end

	-- If we reached this point, we have a valid file we can try to write to
	
	-- There are two flags when writing text
	-- asUnicode - if should write our text as UNICODE
	-- writeUnicodeHeaderBytes - if we should add a special 
  -- unicode header at the beginning of the file
	-- we set both to false

	if fileToWrite:replaceWithText (textToWrite, false, false) == false then
		utils.warnWindow ("File write", "Failed to write data to file: "..fileToWrite.getFullPathName())
	end
end

```
<hr>

### Save file as data

```
data = MemoryBlock of your data you want to save

saveContentAsData = function(mod, value)
	fileToWrite = utils.saveFileWindow(
		"Save content as data",
		File.getSpecialLocation(File.userDesktopDirectory),
		"*.syx",
		true
	)

	if fileToWrite:isValid() == false then
		return
	end

	-- Let's see if the file exists
	if fileToWrite:existsAsFile() == false then
		
		-- The file is not there, that's ok, let's try to create it
		if fileToWrite:create() == false then

			-- Ooooh we can't create it, we need to fail here
			utils.warnWindow ("File write", "The destination file does not exist, and I can't create it")

			return
		end
	end

	
	dataToWrite = data

	if dataToWrite:getSize() <= 0 then
		utils.warnWindow ("Data to write", "There is no data to write")
		return
	end

	-- If we reached this point, we have a valid file we can try to write to
	
	if fileToWrite:replaceWithData (dataToWrite) == false then
		utils.warnWindow ("File write", "Failed to write data to file: "..fileToWrite.getFullPathName())
	end
end

```
<hr>

### Convert MemoryBlock data to string

local s="F0 41 12 00 49 20 61 6D 20 61 20 73 74 72 69 6E 67 f7"
local m=MemoryBlock(s)
console(string.format("%s",m:getRange(4,13):toString()))
prints "I am a string"
