#include "CtrlrLuaFile.h"
#include "stdafx.h"
#include "stdafx_luabind.h"

CtrlrLuaFile::CtrlrLuaFile() : file() {}

CtrlrLuaFile::CtrlrLuaFile(const String &path) : file(path) {}

CtrlrLuaFile::CtrlrLuaFile(const File &fileToUse) : file(fileToUse) {}

CtrlrLuaFile::~CtrlrLuaFile() {}


void CtrlrLuaFile::replaceFileContentWithData(CtrlrLuaMemoryBlock &data) {
	file.replaceWithData(data.getData(), data.getSize());
}

CtrlrLuaMemoryBlock CtrlrLuaFile::loadFileAsData() {

	CtrlrLuaMemoryBlock block;

	MemoryBlock fileData;

	file.loadFileAsData(fileData); // File::loadFileAsData(MemoryBlock&) const

	block.getMemoryBlock() = fileData;

	return block;
}


void CtrlrLuaFile::findChildFiles(luabind::object const &table, int whatToLookFor, bool searchRecursively,
								  const String &wildcardPattern) {
	Array<File> results;
	file.findChildFiles(results, whatToLookFor, searchRecursively, wildcardPattern);
	// Add results to Lua table...
}

CtrlrLuaFile CtrlrLuaFile::getSpecialLocation(const File::SpecialLocationType loc) {
	return CtrlrLuaFile(File::getSpecialLocation(loc));
}
