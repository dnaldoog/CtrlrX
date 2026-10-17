#include "CtrlrLuaMethodManager.h"
#include "CtrlrComponents/CtrlrCustomComponent.h"
#include "CtrlrInlineUtilitiesGUI.h"
#include "CtrlrLog.h"
#include "CtrlrLua/MethodEditor/CtrlrLuaMethodEditor.h"
#include "CtrlrLuaManager.h"
#include "CtrlrLuaObjectWrapper.h"
#include "CtrlrManager/CtrlrManager.h"
#include "CtrlrPanel/CtrlrPanel.h"
#include "CtrlrWindowManagers/CtrlrManagerWindowManager.h"
#include "luabind/object.hpp" // added to check if method has a certain property in isMethodValid, which is used to disable methods that have thrown an error during execution. This is needed to prevent the same error from happening repeatedly and crashing Ctrlr.
#include "stdafx.h"

// #include "luabind/types.hpp"

CtrlrLuaMethodManager::CtrlrLuaMethodManager(CtrlrLuaManager &_owner)
	: managerTree(Ids::luaManagerMethods),
	  owner(_owner),
	  methodTemplates("methods"),
	  debug(false),
	  emptyMethod(*this),
	  currentMethodEditor(nullptr),
	  utilityMethods(nullptr) {
	XmlDocument methodsDocument(
		String(BinaryData::CtrlrLuaMethodTemplates_xml, BinaryData::CtrlrLuaMethodTemplates_xmlSize));

	std::unique_ptr<XmlElement> methodsXml(methodsDocument.getDocumentElement().release());

	if (methodsXml) {
		methodTemplates = XmlElement(*methodsXml);
		utilityMethods = methodTemplates.getChildByName("utilityMethods");
	}

	managerTree.addListener(this);
}

CtrlrLuaMethodManager::~CtrlrLuaMethodManager() {
	currentMethodEditor = nullptr;
	managerTree.removeListener(this);
}

CtrlrLuaManager &CtrlrLuaMethodManager::getOwner() { return (owner); }

int CtrlrLuaMethodManager::getNumUtilities() {
	if (utilityMethods) {
		return (utilityMethods->getNumChildElements());
	}

	return (0);
}

String CtrlrLuaMethodManager::getUtilityCode(const int index) {
	XmlElement *utilityXml = utilityMethods->getChildElement(index);

	if (utilityXml) {
		if (utilityXml->hasTagName("utility")) {
			if (utilityXml->hasAttribute("resource")) {
				int resourceSize;
				const char *resourceData =
					BinaryData::getNamedResource(utilityXml->getStringAttribute("resource").toUTF8(), resourceSize);
				return (String(CharPointer_UTF8(resourceData)));
			} else {
				return (utilityXml->getAllSubText());
			}
		}
	}

	return ("-- No code for utility index " + String(index));
}

String CtrlrLuaMethodManager::getUtilityName(const int index) {
	XmlElement *utilityXml = utilityMethods->getChildElement(index);

	if (utilityXml) {
		if (utilityXml->hasTagName("utility"))
			return (utilityXml->getStringAttribute("name", ""));
	}

	return ("UNKNOWN");
}

bool CtrlrLuaMethodManager::getUtilityAlwaysUpdate(const int index) {
	XmlElement *utilityXml = utilityMethods->getChildElement(index);

	if (utilityXml) {
		if (utilityXml->hasTagName("utility"))
			return (utilityXml->getBoolAttribute("alwaysUpdate", false));
	}

	return (false);
}

String CtrlrLuaMethodManager::getUtilityDescription(const int index) {
	XmlElement *utilityXml = utilityMethods->getChildElement(index);

	if (utilityXml) {
		if (utilityXml->hasTagName("utility"))
			return (utilityXml->getStringAttribute("description", ""));
	}

	return ("");
}

String CtrlrLuaMethodManager::getUtilityUuid(const int index) {
	XmlElement *utilityXml = utilityMethods->getChildElement(index);

	if (utilityXml) {
		if (utilityXml->hasTagName("utility"))
			return (utilityXml->getStringAttribute("uuid", ""));
	}

	return ("");
}

bool CtrlrLuaMethodManager::isLuaDisabled() {
	return ((bool)owner.getOwner().getCtrlrManagerOwner().getProperty(Ids::ctrlrLuaDisabled));
}

void CtrlrLuaMethodManager::restoreState(const ValueTree &savedState) {
	for (int i = 0; i < savedState.getNumChildren(); i++) {
		ValueTree child = savedState.getChild(i);
		if (child.hasType(Ids::luaMethodGroup)) {
			// Skip the Built-In group entirely
			if (child.getProperty(Ids::name).toString() == "Built-In") {
				continue;
			}
			restoreGroup(child);
		}
	}

	for (int i = 0; i < savedState.getNumChildren(); i++) {
		ValueTree child = savedState.getChild(i);
		if (child.hasType(Ids::luaMethod)) {
			restoreMethod(child);
		}

		if (child.hasType(Ids::luaMethodGroup)) {
			if (child.getProperty(Ids::name).toString() == "Built-In") {
				continue;
			}
			restoreMethodsRecursivly(child);
		}
	}
}

void CtrlrLuaMethodManager::restoreGroup(const ValueTree &savedState, const Uuid parentUuid) {
	String groupName = savedState.getProperty(Ids::name).toString();
	if (groupName == "Built-In") {
		return; // Skip Built-In group
	}

	addGroup(groupName, parentUuid, Uuid(savedState.getProperty(Ids::uuid).toString()));

	for (int i = 0; i < savedState.getNumChildren(); i++) {
		ValueTree child = savedState.getChild(i);
		if (child.hasType(Ids::luaMethodGroup)) {
			if (child.getProperty(Ids::name).toString() == "Built-In") {
				continue;
			}
			restoreGroup(child, Uuid(savedState.getProperty(Ids::uuid).toString()));
		}
	}
}

void CtrlrLuaMethodManager::removeGroup(ValueTree groupToRemove) {
	if (!groupToRemove.isValid())
		return;

	// --- GUARD AGAINST DELETING BUILT-IN/READ-ONLY GROUPS ---
	if ((bool)groupToRemove.getProperty("isReadOnly", false) ||
		groupToRemove.getProperty(Ids::luaMethodGroup).toString() == "Built-In") {
		AW::showMessageBox(AW::Warning, "Protected Group", "The Built-In system group cannot be deleted.");
		return;
	}
	// --------------------------------------------------------

	ValueTree parent = groupToRemove.getParent();

	for (int i = 0; i < groupToRemove.getNumChildren(); i++) {
		ValueTree child = groupToRemove.getChild(i);
		if (child.isValid() && child.hasType(Ids::luaMethod)) {
			removeMethod(child.getProperty(Ids::uuid).toString());
		}
	}
	parent.removeChild(groupToRemove, nullptr);
}

void CtrlrLuaMethodManager::addGroup(const String &groupName, const Uuid parentUuid, const Uuid groupUuid) {
	if (parentUuid.isNull()) {
		managerTree.addChild(getDefaultGroupTree(groupName, groupUuid), -1, nullptr);
	} else {
		if (getGroupByUuid(parentUuid).isValid()) {
			getGroupByUuid(parentUuid).addChild(getDefaultGroupTree(groupName, groupUuid), -1, nullptr);
		}
	}
	// Notify the panel about the modification
	owner.getOwner().luaManagerChanged();
}

void CtrlrLuaMethodManager::restoreMethod(const ValueTree &savedState, const Uuid parentUuid) {
	if ((int)savedState.getProperty(Ids::luaMethodSource) == (int)CtrlrLuaMethod::codeInFile) {
		addMethodFromFile(getGroupByUuid(parentUuid),
						  // File(savedState.getProperty(Ids::luaMethodSourcePath).toString()), // Updated v5.6.23+
						  owner.getOwner().getLuaMethodSourceFile(&savedState),
						  Uuid(savedState.getProperty(Ids::uuid).toString()));
	} else {
		addMethod(getGroupByUuid(parentUuid), savedState.getProperty(Ids::luaMethodName),
				  savedState.getProperty(Ids::luaMethodCode), savedState.getProperty(Ids::luaMethodLinkedProperty),
				  savedState.getProperty(Ids::uuid).toString());
	}
}

void CtrlrLuaMethodManager::restoreMethodsRecursivly(const ValueTree &savedState, const Uuid parentUuid) {
	for (int i = 0; i < savedState.getNumChildren(); i++) {
		if (savedState.getChild(i).hasType(Ids::luaMethodGroup)) {
			restoreMethodsRecursivly(savedState.getChild(i), parentUuid);
		}

		if (savedState.getChild(i).hasType(Ids::luaMethod)) {
			restoreMethod(savedState.getChild(i), savedState.getProperty(Ids::uuid).toString());
		}
	}
}

void CtrlrLuaMethodManager::addMethod(ValueTree groupToAddTo, const String &methodName, const String &initialCode,
									  const String &methodProperty, const Uuid methodUuid,
									  const bool forceIfAlreadyExists) {
	ValueTree group;

	if (groupToAddTo.isValid())
		group = groupToAddTo;
	else
		group = managerTree;

	// --- Prevent Duplicates: If this UUID already exists in the group, handle accordingly ---
	ValueTree existingChild = group.getChildWithProperty(Ids::uuid, methodUuid.toString());
	if (existingChild.isValid()) {
		if (forceIfAlreadyExists) {
			group.removeChild(existingChild, nullptr);
		} else {
			return; // Skip adding duplicate!
		}
	}
	// -------------------------------------------------------------------------------------

	ValueTree newMethodTree;

	// --- Check if external editing is enabled ---
	if ((bool)owner.getOwner().getProperty(Ids::ctrlrLuaExternalEditorEnabled))
	{
		juce::File scriptsDir = owner.getPanelWorkspaceDir();
		if (!scriptsDir.exists())
		{
			scriptsDir.createDirectory();
		}
		
		String codeToWrite = initialCode;
		if (codeToWrite.isEmpty())
		{
			codeToWrite = getDefaultMethodCode(methodName, methodProperty);
		}

		juce::File methodFile = scriptsDir.getChildFile(methodName + ".lua");
		// if (!methodFile.existsAsFile()) { this will create a new file every time, which is not desired. We want to
		// overwrite the existing file if it exists.
		methodFile.replaceWithText(codeToWrite);
		//}

		newMethodTree = getDefaultMethodTree(methodFile, methodUuid);
		newMethodTree.setProperty(Ids::luaMethodCode, codeToWrite, nullptr);
		newMethodTree.setProperty(Ids::luaMethodLinkedProperty, methodProperty, nullptr);
	} else {
		newMethodTree = getDefaultMethodTree(methodName, initialCode, methodProperty, methodUuid);
	}

	group.addChild(newMethodTree, -1, nullptr);

	// Notify the panel about the modification
	owner.getOwner().luaManagerChanged();
}
void CtrlrLuaMethodManager::addMethodFromFile(ValueTree groupToAddTo, const File &fileToUse, const Uuid methodUid) {
	ValueTree group;
	if (groupToAddTo.isValid())
		group = groupToAddTo;
	else
		group = managerTree;

	// --- Prevent Duplicates: If this method UID already exists, update it instead of duplicating ---
	ValueTree existingChild = group.getChildWithProperty(Ids::uuid, methodUid.toString());
	if (existingChild.isValid()) {
		existingChild.setProperty(Ids::luaMethodName, fileToUse.getFileNameWithoutExtension(), nullptr);

		File panelLuaDir = owner.getOwner().getPanelLuaDir();
		if (panelLuaDir.exists() && panelLuaDir.isDirectory() && fileToUse.isAChildOf(panelLuaDir)) {
			existingChild.setProperty(Ids::luaMethodSourcePath, fileToUse.getRelativePathFrom(panelLuaDir), nullptr);
		} else {
			existingChild.setProperty(Ids::luaMethodSourcePath, fileToUse.getFullPathName(), nullptr);
		}
		existingChild.setProperty(Ids::luaMethodSource, (int)CtrlrLuaMethod::codeInFile, nullptr);
		return;
	}
	// ---------------------------------------------------------------------------------------------

	group.addChild(getDefaultMethodTree(fileToUse, methodUid), -1, nullptr);

	// Notify the panel about the modification
	owner.getOwner().luaManagerChanged();
}

void CtrlrLuaMethodManager::valueTreeChildAdded(ValueTree &parentTree, ValueTree &child) {
	if (child.hasType(Ids::luaMethod)) {
		methods.add(new CtrlrLuaMethod(*this, child));
	}
}

void CtrlrLuaMethodManager::deleteMethod(const Uuid &methodUuid) {}

void CtrlrLuaMethodManager::valueTreeChildRemoved(ValueTree &parentTree, ValueTree &child, int) {
	if (child.hasType(Ids::luaMethod)) {
		methods.removeObject(getMethodByUuid(child.getProperty(Ids::uuid).toString()));
	}
}

void CtrlrLuaMethodManager::setEditedMethod(const Uuid &methodUuid) {
	if (currentMethodEditor) {
		currentMethodEditor->setEditedMethod(methodUuid);
	}
}

void CtrlrLuaMethodManager::setEditedMethod(const String &methodName) {
	if (currentMethodEditor) {
		if (getMethodByName(methodName)) {
			currentMethodEditor->setEditedMethod(getMethodByName(methodName)->getUuid());
		}
	}
}

const String CtrlrLuaMethodManager::getMethodListForCombo() {
	String ret;
	return (ret.upToLastOccurrenceOf(":", false, false));
}

const StringArray CtrlrLuaMethodManager::getMethodList() {
	StringArray ret;

	for (int i = 0; i < methods.size(); i++) {
		if (methods[i] == nullptr)
			continue;

		String name = methods[i]->getName();
		String code = methods[i]->getCode();

		// Skip tables entirely to keep the callback list clean
		if (code.contains("{") && !code.contains("function")) {
			continue;
		}

		String prefix = "[U] "; // Default: User/Generic function
		if (name == "what" || name == "how" || name == "debugger" || name == "json4lua" || name == "inspect" ||
			name == "table_dump" || name == "isLoading" || name == "memoize") {

			prefix = "[S] ";
		} else if (code.contains("--[[ CtrlrModulator --]]") || code.contains("source")) {
			prefix = "[C] "; // Component/Modulator Callback
		} else if (code.contains("--[[ MouseEvent --]]") || code.contains("event")) {
			prefix = "[E] "; // UI Mouse Event
		}
		//DBG("name " + name);
		ret.add(prefix + name);
	}

	ret.sortNatural();

	return (ret);
}
void CtrlrLuaMethodManager::valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged,
													 const Identifier &property) {}

CtrlrLuaMethod *CtrlrLuaMethodManager::getMethodByName(const String &methodName) {
	for (int i = 0; i < methods.size(); i++) {
		if (methods[i]->getName() == methodName) {
			return (methods[i]);
		}
	}
	return (nullptr);
}

CtrlrLuaMethod *CtrlrLuaMethodManager::getMethodByUuid(const Uuid &methodUuid) {
	for (int i = 0; i < methods.size(); i++) {
		if (methods[i]->getUuid() == methodUuid) {
			return (methods[i]);
		}
	}

	return (nullptr);
}

ValueTree CtrlrLuaMethodManager::getGroupByUuid(const Uuid &groupUuid) {
	if (managerTree.getChildWithProperty(Ids::uuid, groupUuid.toString()).isValid()) {
		return (managerTree.getChildWithProperty(Ids::uuid, groupUuid.toString()));
	} else {
		for (int i = 0; i < managerTree.getNumChildren(); i++) {
			if (managerTree.getChild(i).hasType(Ids::luaMethodGroup) && managerTree.getChild(i).getNumChildren() > 0) {
				ValueTree foundGroup = findGroupRecursive(managerTree.getChild(i), groupUuid);
				if (foundGroup.isValid()) {
					return (foundGroup);
				}
			}
		}
	}

	return (ValueTree());
}

ValueTree CtrlrLuaMethodManager::findGroupRecursive(ValueTree treeToSearch, const Uuid &groupUuid) {
	for (int i = 0; i < treeToSearch.getNumChildren(); i++) {
		if (treeToSearch.getChild(i).hasType(Ids::luaMethodGroup)) {
			if (treeToSearch.getChild(i).getProperty(Ids::uuid) == groupUuid.toString()) {
				return (treeToSearch.getChild(i));
			} else {
				ValueTree t = findGroupRecursive(treeToSearch.getChild(i), groupUuid);
				if (t.isValid()) {
					return (t);
				}
			}
		}
	}
	return (ValueTree());
}

const String CtrlrLuaMethodManager::cleanupMethod(XmlElement *methodElement, const String &methodName) {
	String methodCode = "";
	String value;
	StringArray lines;

	if (methodElement->hasAttribute("resource")) {
		const String resourceName = methodElement->getStringAttribute("resource");
		int resourceSize = 0;
		const char *resourceData = BinaryData::getNamedResource(resourceName.toUTF8(), resourceSize);

		if (resourceData && resourceSize > 0) {
			value = String(CharPointer_UTF8(resourceData));
		}
	} else {
		methodCode = methodElement->getAllSubText();
		lines.addTokens(methodCode, "\n", "");

		for (int i = 0; i < lines.size(); i++) {
			value << lines[i].trim() << "\n";
		}
	}

	return (value.replace("__method_name", methodName, false).trim());
}

const String CtrlrLuaMethodManager::getTemplateForProperty(const String &methodName, const String &propertyName) {
	forEachXmlChildElement(methodTemplates, e) {
		if (e->hasTagName("luaMethod")) {
			String linkedNames = e->getStringAttribute("name");
			StringArray ar;
			ar.addTokens(linkedNames, ",", "\"\'");

			for (int i = 0; i < ar.size(); i++) {
				if (ar[i] == propertyName) {
					return (cleanupMethod(e, methodName));
				}
			}
		}
	}

	_ERR("CtrlrLuaMethodManager::getTemplateForProperty failed to find template for this method \"" + methodName +
		 "\" propertyName \"" + propertyName + "\"");
	return ("");
}

const StringArray CtrlrLuaMethodManager::getTemplateList() {
	StringArray ret;
	ret.add(COMBO_NONE_ITEM);
	forEachXmlChildElement(methodTemplates, el) {
		if (el->hasTagName(Ids::luaMethod.toString())) {
			ret.add(el->getStringAttribute("name", ""));
		}
	}

	return (ret);
}

const String CtrlrLuaMethodManager::getDefaultMethodCode(const String &methodName, const String &linkedToProperty) {
	if (linkedToProperty.isEmpty() || linkedToProperty == COMBO_NONE_ITEM) {
		return ("function " + methodName +
				"()\n"
				"\t-- Safety Check: Prevents methods firing during boot, DAW load, or preset changes.\n"
				"\t-- Combines panel:getBootstrapState() panel:getRestoreState()  panel:getProgramState().\n"
				"\t-- (Optionally delete this line if this function must run during initialization)\n"
				"\tif panel:isLoading() then return end\n\n"
				"end");
	} else {
		return (getTemplateForProperty(methodName, linkedToProperty));
	}
}
ValueTree CtrlrLuaMethodManager::getDefaultMethodTree(const String &methodName, const String &methodCode,
													  const String &methodProperty, const Uuid methodUuid) {
	ValueTree methodTree(Ids::luaMethod);
	methodTree.setProperty(Ids::luaMethodName, methodName, nullptr);

	if (methodCode.isEmpty())
		methodTree.setProperty(Ids::luaMethodCode, getDefaultMethodCode(methodName, methodProperty), nullptr);
	else
		methodTree.setProperty(Ids::luaMethodCode, methodCode, nullptr);

	methodTree.setProperty(Ids::luaMethodLinkedProperty, methodProperty, nullptr);
	methodTree.setProperty(Ids::luaMethodSource, (int)CtrlrLuaMethod::codeInProperty, nullptr);
	methodTree.setProperty(Ids::uuid, methodUuid.isNull() ? Uuid().toString() : methodUuid.toString(), nullptr);
	return (methodTree);
}

ValueTree CtrlrLuaMethodManager::getDefaultMethodTree(const File &methodFileSource, const Uuid methodUuid) {
	ValueTree methodTree(Ids::luaMethod);
	methodTree.setProperty(Ids::luaMethodName, methodFileSource.getFileNameWithoutExtension(), nullptr);

	File panelLuaDir = getOwner().getOwner().getPanelLuaDir();
	if (panelLuaDir.exists() && panelLuaDir.isDirectory() &&
		methodFileSource.isAChildOf(panelLuaDir)) { // Try and get realtive path
		methodTree.setProperty(Ids::luaMethodSourcePath, methodFileSource.getRelativePathFrom(panelLuaDir), nullptr);
	} else {
		methodTree.setProperty(Ids::luaMethodSourcePath, methodFileSource.getFullPathName(), nullptr);
	}
	methodTree.setProperty(Ids::luaMethodSource, (int)CtrlrLuaMethod::codeInFile, nullptr);
	methodTree.setProperty(Ids::uuid, methodUuid.isNull() ? Uuid().toString() : methodUuid.toString(), nullptr);
	return (methodTree);
}

ValueTree CtrlrLuaMethodManager::getDefaultGroupTree(const String &groupName, const Uuid groupUid) {
	ValueTree group(Ids::luaMethodGroup);
	group.setProperty(Ids::name, groupName, nullptr);
	if (groupUid.isNull()) {
		group.setProperty(Ids::uuid, Uuid().toString(), nullptr);
	} else {
		group.setProperty(Ids::uuid, groupUid.toString(), nullptr);
	}

	return (group);
}

bool CtrlrLuaMethodManager::isValidMethodName(const String &methodName) {
	if (methodName.isEmpty())
		return false;

	const juce_wchar firstCharacter = methodName[0];
	const bool startsWithLetterOrUnderscore =
		(firstCharacter >= 'a' && firstCharacter <= 'z') || (firstCharacter >= 'A' && firstCharacter <= 'Z') ||
		firstCharacter == '_';

	return startsWithLetterOrUnderscore &&
		   methodName.containsOnly("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
}

void CtrlrLuaMethodManager::setMethodEditor(CtrlrLuaMethodEditor *_currentMethodEditor) {
	currentMethodEditor = _currentMethodEditor;
}

void CtrlrLuaMethodManager::removeMethod(const Uuid methodUuid) {
    CtrlrLuaMethod *method = getMethodByUuid(methodUuid);
    if (!method) {
        DBG("removeMethod: Method not found for UUID.");
        return;
    }

    ValueTree child = method->getMethodTree();
    String methodName = child.getProperty(Ids::luaMethodName).toString();
    
    // --- DEBUG GUARDS ---
    bool isReadOnly = (bool)child.getProperty("isReadOnly", false);
    DBG("removeMethod checking method: " + methodName + " | isReadOnly: " + (isReadOnly ? "TRUE" : "FALSE"));

    if (isReadOnly) {
        DBG(" -> BLOCKED: Method is marked as read-only.");
        AW::showMessageBox(AW::Warning, "Protected Method", "Built-in system methods cannot be deleted.");
        return;
    }
    
    ValueTree parent = child.getParent();
    if (parent.isValid()) {
        String groupName = parent.getProperty(Ids::luaMethodGroup).toString();
        if (groupName.isEmpty())
            groupName = parent.getProperty(Ids::name).toString(); // Fallback check
            
        DBG(" -> Parent group name: " + groupName);

        if (groupName == "Built-In") {
            DBG(" -> BLOCKED: Method belongs to Built-In group.");
            AW::showMessageBox(AW::Warning, "Protected Method", "Methods inside the Built-In group cannot be deleted.");
            return;
        }
    }
    // ---------------------------------------------------------

    // --- Delete the physical .lua file from disk if external editing is enabled ---
    if ((bool)owner.getOwner().getProperty(Ids::ctrlrLuaExternalEditorEnabled)) {
        juce::File scriptsDir = owner.getPanelWorkspaceDir();
        juce::String sourcePath = child.getProperty(Ids::luaMethodSourcePath).toString();
        
        if (sourcePath.isNotEmpty()) {
            juce::File methodFile = scriptsDir.getChildFile(sourcePath);
            if (methodFile.existsAsFile()) {
                methodFile.deleteFile();
            }
        } else {
            juce::File methodFile = scriptsDir.getChildFile(methodName + ".lua");
            if (methodFile.existsAsFile()) {
                methodFile.deleteFile();
            }
        }
    }

    method->remove();
    parent.removeChild(child, nullptr);

    methods.removeObject(method);
    DBG("Successfully removed regular method: " + methodName);
}
	// -----------------------------------------------------------------------------



const int CtrlrLuaMethodManager::getNumMethods() { return (methods.size()); }

CtrlrLuaMethod *CtrlrLuaMethodManager::getMethodByIndex(const int index) { return (methods[index]); }

ValueTree CtrlrLuaMethodManager::getGroupByName(const String &groupName) {
	for (int i = 0; i < getManagerTree().getNumChildren(); i++) {
		if (getManagerTree().getChild(i).hasType(Ids::luaMethodGroup) &&
			groupName == getManagerTree().getChild(i).getProperty(Ids::name).toString()) {
			return (getManagerTree().getChild(i));
		}
	}

	return (ValueTree());
}

const bool CtrlrLuaMethodManager::attachDefaultGroups() {
	bool ret = false;

	if (!getGroupByName("Built-In").isValid()) {
		addGroup("Built-In", Uuid::null(), Uuid());
		ret = true;
	}

	return (ret);
}

void CtrlrLuaMethodManager::wrapUtilities() {
	if (getNumUtilities() > 0) {
		attachDefaultGroups();

		ValueTree builtInGroup = getGroupByName("Built-In");

		for (int i = 0; i < getNumUtilities(); i++) {
			// Add the method normally
			addMethod(builtInGroup, getUtilityName(i), getUtilityCode(i).trim(), "",
				  getUtilityUuid(i), getUtilityAlwaysUpdate(i));
			ValueTree newMethod = builtInGroup.getChildWithProperty(Ids::uuid, getUtilityUuid(i));

			// --- MARK AS READ-ONLY / BUILT-IN ---
			if (newMethod.isValid()) {
				newMethod.setProperty("isReadOnly", true, nullptr);
			}
		}

		// Also mark the "Built-In" group itself as protected
		if (builtInGroup.isValid()) {
			builtInGroup.setProperty("isReadOnly", true, nullptr);
		}
	}
}

bool CtrlrLuaMethodManager::isMethodValid(CtrlrLuaMethod *o) {
	if (o == nullptr)
		return false;

	try {
		if (!o->isValid())
			return false;
	} catch (...) {
		return false;
	}

	lua_State *L = owner.getLuaState();
	if (!L)
		return false;

	try {
		luabind::object methodObj = o->getObject().getLuabindObject();
		if (methodObj && luabind::type(methodObj) == LUA_TFUNCTION)
			return true;
	} catch (...) {
	}

	return false;
}

void CtrlrLuaMethodManager::handleExternalFileChange(const juce::File &file, bool isDeleted)
//when an external file is deleted, we need to remove the corresponding method from Ctrlr's internal tree without triggering a back-loop of file deletion
{
    // 1. Find if a method in our tree points to this file path
	ValueTree methodTree;
	for (int i = 0; i < methods.size(); ++i)
	{
		if (methods[i] == nullptr)
			continue;

		ValueTree candidateTree = methods[i]->getMethodTree();
		if (owner.getOwner().getLuaMethodSourceFile(&candidateTree) == file)
		{
			methodTree = candidateTree;
			break;
		}
	}
    
    if (methodTree.isValid())
    {
        if (!file.existsAsFile() || isDeleted)
        {
            // 2. The file was deleted externally! Remove it from Ctrlr without 
            // triggering our own file-deletion back-loop.
            Uuid methodUuid(methodTree.getProperty(Ids::uuid).toString());
            
			// Remove the method from the manager without deleting the external file.
			CtrlrLuaMethod *method = getMethodByUuid(methodUuid);
			if (method)
			{
				ValueTree child = method->getMethodTree();
				method->remove();
				child.getParent().removeChild(child, nullptr);
			}
        }
        else
        {
            // Normal file update / hot reload
            String newCode = file.loadFileAsString();
            methodTree.setProperty(Ids::luaMethodCode, newCode, nullptr);
            
            // Recompile/reload bytecode in the background...
        }
    }
}