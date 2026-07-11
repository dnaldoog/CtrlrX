#ifndef __CTRLR_LUA_FILE__
#define __CTRLR_LUA_FILE__
#ifdef _WIN32
#pragma warning(disable:4100)
#endif

#include "JuceHeader.h"
#include "CtrlrLuaMemoryBlock.h"

//==============================================================================
/** @brief A class that represents a file see http://www.rawmaterialsoftware.com/juce/api/classFile.html for details
 *			Never use this class directly always use the File class
 *
 */
class CtrlrLuaFile
{
	public:
		CtrlrLuaFile();
		CtrlrLuaFile (const File &file);
		CtrlrLuaFile (const String &path);
		~CtrlrLuaFile();

		/** @brief Replace the file content with a block of data
			
			@param	data	The data to be written to the file
		*/
		void replaceFileContentWithData (CtrlrLuaMemoryBlock &data);

		/** @brief Load a file into a memory block

			@return	A MemoryBlock object containing the file data
		*/
		CtrlrLuaMemoryBlock loadFileAsData();

		/** @brief Find child files

			@param	table				A LUA table that will contain the results
			@param	whatToLookFor		What to look for (File::findChildFiles constants)
			@param	searchRecursively	If true, searches recursively
			@param	wildcardPattern		A wildcard pattern for filtering results
		*/
		void findChildFiles (luabind::object const& table, int whatToLookFor, bool searchRecursively, const String &wildcardPattern);

		/** @brief Get a special location

			@param	loc	The special location type
			@return	A CtrlrLuaFile object representing the special location
		*/
		static CtrlrLuaFile getSpecialLocation(const File::SpecialLocationType loc);

		// Expose File methods - delegates to underlying file
		File &getFile() { return file; }
		const File &getFile() const { return file; }
		
		bool exists() const { return file.exists(); }
		bool isDirectory() const { return file.isDirectory(); }
		String getFullPathName() const { return file.getFullPathName(); }
		String getFileName() const { return file.getFileName(); }
		File getParentDirectory() const { return file.getParentDirectory(); }
		File getChildFile(const String& relativePath) const { return file.getChildFile(relativePath); }
		
		static void wrapForLua (lua_State *L);

	private:
		File file;
};

#endif
