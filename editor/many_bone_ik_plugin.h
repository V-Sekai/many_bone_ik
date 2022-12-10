#ifndef MANY_BONE_IK_PLUGIN_H
#define MANY_BONE_IK_PLUGIN_H

#include "editor/plugins/node_3d_editor_gizmos.h"
#include "editor/editor_inspector.h"
#include "editor/plugins/node_3d_editor_plugin.h"
#include "modules/many_bone_ik/src/many_bone_ik_3d.h"
#include "editor/plugins/skeleton_3d_editor_plugin.h"

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
	BoneId select_bone = -1;
	
	EditorInspectorSection *bone_pinning_section = nullptr;
	EditorPropertyCheck *pin_checkbox = nullptr;
	EditorPropertyNodePath *target_nodepath= nullptr;

	EditorInspectorSection *kusudama_twist_constraint_section = nullptr;
	EditorPropertyFloat *twist_from_float = nullptr;
	EditorPropertyFloat *twist_range_float = nullptr;
	EditorPropertyFloat *twist_current_float = nullptr;

	EditorInspectorSection *kusudama_orientation_constraint_section = nullptr;
	EditorPropertyVector3 *center_vector3 = nullptr;
	EditorPropertyFloat *radius_float = nullptr;

	EditorInspectorSection *twist_constraint_basis_section = nullptr;
	EditorPropertyBasis *twist_constraint_basis = nullptr;

	EditorInspectorSection *orientation_constraint_basis_section = nullptr;
	EditorPropertyBasis *orientation_constraint_basis = nullptr;

	EditorInspectorSection *bone_direction_transform_section = nullptr;
	EditorPropertyTransform3D *bone_direction_transform = nullptr;

protected:
	void _notification(int p_what);

public:
	ManyBoneIK3DEditor(EditorInspectorPluginManyBoneIK *e_plugin, ManyBoneIK3D *p_ik);
	void _update_properties();
	void update_joint_tree();
	void create_editors();
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
