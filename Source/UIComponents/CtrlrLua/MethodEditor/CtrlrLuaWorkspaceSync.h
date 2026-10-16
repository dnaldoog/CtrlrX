/*
  ==============================================================================

    CtrlrLuaWorkspaceSync.h
    Created: 16 Oct 2026 9:25:05pm
    Author:  zan64

  ==============================================================================
*/

#pragma once
#include "Core/CtrlrPanel/CtrlrPanel.h"
// ---------------------------------------------------------------------------
// CtrlrLuaWorkspaceSync.h
//
// Keeps the files on disk in step with the Lua method/group tree while external
// editing is enabled (tree -> disk). Header-only; adjust the include to match
// your project (JuceHeader.h / CtrlrPch.h, CtrlrPanel.h, CtrlrIDs).
//
// ASSUMPTIONS - change to match your tree:
//   Ids::luaMethodGroup      ValueTree type of a group node
//   Ids::name                group display-name property
//   Ids::luaMethodName       method name property
//   Ids::luaMethodSourcePath stored path property (seen in getLuaMethodSourceFile)
// ---------------------------------------------------------------------------

class CtrlrLuaWorkspaceSync : public juce::ValueTree::Listener, private juce::AsyncUpdater
{
public:
    CtrlrLuaWorkspaceSync(CtrlrPanel &panel, juce::ValueTree methodsRoot, const juce::File &workspaceDir)
        : owner(panel), root(methodsRoot), scriptsDir(workspaceDir)
    {
        root.addListener(this); // deep: hears the whole subtree
    }

    ~CtrlrLuaWorkspaceSync() override
    {
        cancelPendingUpdate();
        root.removeListener(this);
    }

    // Call once after enabling so existing mismatches are fixed immediately.
    void resyncNow() { handleAsyncUpdate(); }

private:
    CtrlrPanel &owner;
    juce::ValueTree root;
    juce::File scriptsDir;
    bool resyncing = false;

    // ---- listener: react only to structural / naming changes ----------------
    void valueTreeChildAdded(juce::ValueTree &, juce::ValueTree &) override { triggerAsyncUpdate(); }
    void valueTreeChildRemoved(juce::ValueTree &, juce::ValueTree &, int) override { triggerAsyncUpdate(); }
    void valueTreeParentChanged(juce::ValueTree &) override { triggerAsyncUpdate(); }

    void valueTreePropertyChanged(juce::ValueTree &, const juce::Identifier &property) override
    {
        // NOT luaMethodSourcePath - we write that ourselves (avoids feedback loop).
        if (property == Ids::name || property == Ids::luaMethodName)
            triggerAsyncUpdate();
    }

    // ---- helpers -------------------------------------------------------------
    static void collectMethods(const juce::ValueTree &parent, juce::Array<juce::ValueTree> &out)
    {
        for (int i = 0; i < parent.getNumChildren(); ++i)
        {
            auto c = parent.getChild(i);
            if (c.hasType(Ids::luaMethod)) out.add(c);
            else collectMethods(c, out);
        }
    }

    // scripts/<group>/<subgroup>/<method>.lua, from the node's position in the tree.
    juce::File expectedFileFor(const juce::ValueTree &method) const
    {
        juce::StringArray groups;
        for (auto p = method.getParent(); p.isValid() && p != root; p = p.getParent())
            if (p.hasType(Ids::luaMethodGroup))
                groups.insert(0, juce::File::createLegalFileName(p.getProperty(Ids::name).toString()));

        juce::File dir = scriptsDir;
        for (auto &g : groups)
            dir = dir.getChildFile(g);

        return dir.getChildFile(method.getProperty(Ids::luaMethodName).toString() + ".lua");
    }

    void handleAsyncUpdate() override
    {
        if (resyncing) return;
        const juce::ScopedValueSetter<bool> guard(resyncing, true);

        juce::Array<juce::ValueTree> methods;
        collectMethods(root, methods);

        for (auto &m : methods)
        {
            const juce::File current = owner.getLuaMethodSourceFile(&m);
            const juce::File expected = expectedFileFor(m);

            if (current == expected)
                continue;

            if (expected.existsAsFile())
            {
                DBG("WorkspaceSync: target already exists, leaving alone: " + expected.getFullPathName());
                continue;
            }

            expected.getParentDirectory().createDirectory();

            if (current.existsAsFile())
            {
                if (!current.moveFileTo(expected))
                {
                    DBG("WorkspaceSync: move failed: " + current.getFullPathName());
                    continue;
                }

                // tidy the folder we just emptied (never the workspace root)
                auto oldDir = current.getParentDirectory();
                if (oldDir != scriptsDir && oldDir.isAChildOf(scriptsDir)
                    && oldDir.getNumberOfChildFiles(juce::File::findFilesAndDirectories) == 0)
                    oldDir.deleteFile();
            }
            else
            {
                // TODO: no file exists yet for this method (e.g. just created) - write its
                // code to `expected` here, using whatever saveLuaCode() uses per method.
            }

            m.setProperty(Ids::luaMethodSourcePath,
                          expected.getRelativePathFrom(owner.getPanelLuaDir()), nullptr);
        }
    }
};

