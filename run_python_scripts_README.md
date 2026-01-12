### Notes on building the Lua API xml file.

(1) `python luabind_parser.py` - scans CtrlrX source code and generates xml file in _Source/Resources/XML/LuaAPI.xml_
(2) Generate a list of all classes containing enums and paste into lua_api_patcher.py source code: `python extract_enums.py`
(3) Run `python lua_api_patcher.py` to update LuaApi.xml
(4) Compile CtrlrX

