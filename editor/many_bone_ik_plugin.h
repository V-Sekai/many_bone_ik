#ifndef MANY_BONE_IK_PLUGIN_H
#define MANY_BONE_IK_PLUGIN_H

#include "editor/editor_inspector.h"
#include "editor/plugins/node_3d_editor_gizmos.h"
#include "editor/plugins/node_3d_editor_plugin.h"
#include "editor/plugins/skeleton_3d_editor_plugin.h"
#include "modules/many_bone_ik/src/many_bone_ik_3d.h"

class ManyBoneIK3DEditorPlugin;
class ManyBoneIK3DEditor;
class EditorInspectorPluginManyBoneIK : public EditorInspectorPlugin {
	GDCLASS(EditorInspectorPluginManyBoneIK, EditorInspectorPlugin);

	friend class ManyBoneIK3DEditorPlugin;
	ManyBoneIK3DEditor *skel_editor = nullptr;

public:
	virtual bool can_handle(Object *p_object) override;
	virtual void parse_begin(Object *p_object) override;
};

class ManyBoneIK3DEditor : public VBoxContainer {
	GDCLASS(ManyBoneIK3DEditor, VBoxContainer);

	Tree *joint_tree = nullptr;
	ManyBoneIK3D *ik = nullptr;
	BoneId selected_bone = -1;

	EditorInspectorSection *constraint_bone_section = nullptr;
	EditorPropertyCheck *pin_checkbox = nullptr;
	EditorPropertyNodePath *target_nodepath = nullptr;
	EditorPropertyFloat *twist_from_float = nullptr;
	EditorPropertyFloat *twist_range_float = nullptr;
	EditorPropertyFloat *twist_current_float = nullptr;
	static const int32_t MAX_KUSUDAMA_CONES = 30;
	EditorPropertyFloat *cone_count_float = nullptr;
	EditorPropertyVector3 *center_vector3[MAX_KUSUDAMA_CONES] = {};
	EditorPropertyFloat *radius_float[MAX_KUSUDAMA_CONES] = {};
	EditorPropertyTransform3D *twist_constraint_transform = nullptr;
	EditorPropertyTransform3D *orientation_constraint_transform = nullptr;
	EditorPropertyTransform3D *bone_direction_transform = nullptr;

protected:
	void _notification(int p_what);

public:
	ManyBoneIK3DEditor(EditorInspectorPluginManyBoneIK *e_plugin, ManyBoneIK3D *p_ik);
	void _update_properties();
	void update_joint_tree();
	void create_editors();
	void _value_changed(const String &p_property, Variant p_value, const String &p_name, bool p_changing);
	void select_bone(int p_idx);
	void _joint_tree_selection_changed();
	TreeItem *_find(TreeItem *p_node, const NodePath &p_path);
};

class ManyBoneIK3DEditorPlugin : public EditorPlugin {
	GDCLASS(ManyBoneIK3DEditorPlugin, EditorPlugin);
	EditorInspectorPluginManyBoneIK *skeleton_plugin = nullptr;

public:
	virtual EditorPlugin::AfterGUIInput forward_3d_gui_input(Camera3D *p_camera, const Ref<InputEvent> &p_event) override;
	bool has_main_screen() const override;
	virtual bool handles(Object *p_object) const override;
	virtual String get_name() const override;
	ManyBoneIK3DEditorPlugin();
};

#endif