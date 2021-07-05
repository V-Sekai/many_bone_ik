#pragma once

#include "editor/editor_plugin.h"
#include "skeleton_modification_3d_ewbik.h"
#include "editor/editor_inspector.h"

class EWBIKEditorPlugin;
class EditorInspectorPluginEWBIK;
class SkeletonModification3DEWBIKEditor : public VBoxContainer {
	GDCLASS(SkeletonModification3DEWBIKEditor, VBoxContainer);

	friend class EditorInspectorPluginEWBIK;
	friend class SkeletonModification3DEWBIK;
	EditorInspectorPluginEWBIK *editor_plugin;
	EditorNode *editor;
	Ref<SkeletonModification3DEWBIK> ik;

protected:

public:
	Ref<SkeletonModification3DEWBIK> get_ik() const { return ik; };
	SkeletonModification3DEWBIKEditor(EditorInspectorPluginEWBIK *e_plugin, EditorNode *p_editor, SkeletonModification3DEWBIK *p_ik);
	~SkeletonModification3DEWBIKEditor() {}
};

class EditorInspectorPluginEWBIK : public EditorInspectorPlugin {
	GDCLASS(EditorInspectorPluginEWBIK, EditorInspectorPlugin);

	friend class EWBIKEditorPlugin;
	SkeletonModification3DEWBIKEditor *ik_editor;
	EditorNode *editor;

public:
	virtual bool can_handle(Object *p_object) override;
	virtual void parse_begin(Object *p_object) override;
	virtual void parse_end() override {}
};

class EWBIKEditorPlugin : public EditorPlugin {
	GDCLASS(EWBIKEditorPlugin, EditorPlugin);

	EditorInspectorPluginEWBIK *ik_plugin;
	EditorNode *editor;

public:
	virtual String get_name() const override { return "EWBIK"; }

	EWBIKEditorPlugin(EditorNode *p_node) {
		editor = p_node;

		ik_plugin = memnew(EditorInspectorPluginEWBIK);
		ik_plugin->editor = editor;

		EditorInspector::add_inspector_plugin(ik_plugin);
	}
};