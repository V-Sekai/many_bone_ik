#include "ewbik_editor_inspector.h"
#include "editor/editor_inspector.h"
#include "editor/editor_node.h"
#include "editor/editor_properties.h"
#include "editor/editor_properties_array_dict.h"

bool EditorInspectorPluginEWBIK::can_handle(Object *p_object) {
	return Object::cast_to<SkeletonModification3DEWBIK>(p_object) != nullptr;
}

void EditorInspectorPluginEWBIK::parse_begin(Object *p_object) {
	SkeletonModification3DEWBIK *ik = Object::cast_to<SkeletonModification3DEWBIK>(p_object);
	ERR_FAIL_NULL(ik);

	ik_editor = memnew(SkeletonModification3DEWBIKEditor(this, editor, ik));
	add_custom_control(ik_editor);
}

SkeletonModification3DEWBIKEditor::SkeletonModification3DEWBIKEditor(EditorInspectorPluginEWBIK *e_plugin, EditorNode *p_editor, SkeletonModification3DEWBIK *p_ik) :
		editor_plugin(e_plugin),
		editor(p_editor),
		ik(p_ik) {
	const Color section_color = get_theme_color("prop_subsection", "Editor");
	EditorInspectorSection *ewbik_section = memnew(EditorInspectorSection);
	ewbik_section->setup("ewbik_properties", "EWBIK", this, section_color, true);
	add_child(ewbik_section);
	Skeleton3D *skeleton = ik->skeleton;
	if (skeleton) {
		EditorPropertyTextEnum *root_bone_property = memnew(EditorPropertyTextEnum);
		Vector<String> names;
		names.resize(skeleton->get_bone_count());
		for (int i = 0; i < skeleton->get_bone_count(); i++) {
			names.write[i] = skeleton->get_bone_name(i);
		}
		root_bone_property->setup(names, false);
		root_bone_property->set_label(TTR("Root Bone"));
		add_child(root_bone_property);
	} else {
		EditorPropertyText *root_bone_property = memnew(EditorPropertyText);
		root_bone_property->set_label(TTR("Root Bone"));
		add_child(root_bone_property);
	}
	EditorPropertyInteger *ik_iterations_property = memnew(EditorPropertyInteger);
	ik_iterations_property->set_label(TTR("IK Iterations"));
	// 	p_list->push_back(PropertyInfo(Variant::INT, "ik_iterations", PROPERTY_HINT_RANGE, "0,65535,1"));
	add_child(ik_iterations_property);

	EditorPropertyFloat *default_damp_property = memnew(EditorPropertyFloat);
	default_damp_property->set_label(TTR("Default Damp Degrees"));
	default_damp_property->setup(0.01f, 180.0f, 0.01, false, false, false, false, String(), true);

	add_child(default_damp_property);

	EditorInspectorSection *effector_section = memnew(EditorInspectorSection);
	effector_section->setup("effector_properties", "Effector", this, section_color, true);
	add_child(effector_section);

	EditorPropertyDictionary *effectors_property = memnew(EditorPropertyDictionary);

	Array array;
	for (Map<BoneId, Ref<IKBone3D>>::Element *E = ik->effectors_map.front(); E; E = E->next()) {
		array.push_back(E->get());
	}
	add_child(effectors_property);
}