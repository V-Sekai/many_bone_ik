#pragma once

#include "editor/editor_plugin.h"
#include "skeleton_modification_3d_ewbik.h"
#include "editor/editor_inspector.h"

class EWBIKEditorPlugin;
class EditorInspectorPluginEWBIK;
class SkeletonModification3DEWBIKEditor : public VBoxContainer {
	GDCLASS(SkeletonModification3DEWBIKEditor, VBoxContainer);

	friend class EditorInspectorPluginEWBIK;
	EditorInspectorPluginEWBIK *editor_plugin;
	EditorNode *editor;
	Ref<SkeletonModification3DEWBIK> ik;

protected:
	// void _validate_property(PropertyInfo &property) const {
	// 	if (property.name == "root_bone") {
	// 		if (skeleton) {
	// 			String names = "None";
	// 			for (int i = 0; i < skeleton->get_bone_count(); i++) {
	// 				names += ",";
	// 				names += skeleton->get_bone_name(i);
	// 			}

	// 			property.hint = PROPERTY_HINT_ENUM;
	// 			property.hint_string = names;
	// 		} else {
	// 			property.hint = PROPERTY_HINT_NONE;
	// 			property.hint_string = "";
	// 		}
	// 	}
	// }
	// void _get_property_list(List<PropertyInfo> *p_list) const {
	// 	p_list->push_back(PropertyInfo(Variant::INT, "ik_iterations", PROPERTY_HINT_RANGE, "0,65535,1"));
	// 	p_list->push_back(PropertyInfo(Variant::FLOAT, "default_damp_degrees", PROPERTY_HINT_RANGE, "0.01,180,0.01"));
	// 	p_list->push_back(PropertyInfo(Variant::DICTIONARY, "effectors"));
	// }

	// bool _get(const StringName &p_name, Variant &r_ret) const {
	// 	String name = p_name;
	// 	if (name == "ik_iterations") {
	// 		r_ret = get_ik_iterations();
	// 		return true;
	// 	} else if (name == "default_damp_degrees") {
	// 		r_ret = Math::rad2deg((double)get_default_damp());
	// 		return true;
	// 	} else if (name.begins_with("effectors")) {
	// 		Dictionary dict;
	// 		for (Map<BoneId, Ref<IKBone3D>>::Element *E = effectors_map.front(); E; E = E->next()) {
	// 			dict[E->key()] = E->get();
	// 		}
	// 		r_ret = dict;
	// 		return true;
	// 	}

	// 	return false;
	// }

	// bool _set(const StringName &p_name, const Variant &p_value) {
	// 	String name = p_name;
	// 	if (name == "ik_iterations") {
	// 		set_ik_iterations(p_value);
	// 		return true;
	// 	} else if (name == "default_damp_degrees") {
	// 		set_default_damp(Math::deg2rad((double)p_value));
	// 		return true;
	// 	} else if (name.begins_with("effectors")) {
	// 		Dictionary dict = p_value;
	// 		List<Variant> keys;
	// 		dict.get_key_list(&keys);
	// 		effectors_map.clear();
	// 		for (int32_t i = 0; i < keys.size(); i++) {
	// 			effectors_map[keys[i]] = dict[keys[i]];
	// 		}
	// 		return true;
	// 	}

	// 	return false;
	// }

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