/**************************************************************************/
/*  many_bone_ik_plugin.cpp                                               */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "many_bone_ik_plugin.h"

#include "editor/plugins/node_3d_editor_plugin.h"
#include "ik_bone_3d.h"
#include "many_bone_ik_3d.h"
#include "scene/gui/tree.h"
#include "scene/resources/skeleton_profile.h"

EditorPlugin::AfterGUIInput ManyBoneIK3DEditorPlugin::forward_3d_gui_input(Camera3D *p_camera, const Ref<InputEvent> &p_event) {
	Skeleton3DEditor *se = Skeleton3DEditor::get_singleton();
	Node3DEditor *ne = Node3DEditor::get_singleton();
	if (se && se->is_edit_mode()) {
		const Ref<InputEventMouseButton> mb = p_event;
		if (mb.is_valid() && mb->get_button_index() == MouseButton::LEFT) {
			if (ne->get_tool_mode() != Node3DEditor::TOOL_MODE_SELECT) {
				if (!ne->is_gizmo_visible()) {
					return EditorPlugin::AFTER_GUI_INPUT_STOP;
				}
			}
			if (mb->is_pressed()) {
				se->update_bone_original();
			}
		}
		return EditorPlugin::AFTER_GUI_INPUT_CUSTOM;
	}
	return EditorPlugin::AFTER_GUI_INPUT_PASS;
}

bool ManyBoneIK3DEditorPlugin::has_main_screen() const {
	return false;
}

bool ManyBoneIK3DEditorPlugin::handles(Object *p_object) const {
	return p_object->is_class("ManyBoneIK3D");
}

String ManyBoneIK3DEditorPlugin::get_name() const {
	return "ManyBoneIK3D";
}

bool EditorInspectorPluginManyBoneIK::can_handle(Object *p_object) {
	return Object::cast_to<ManyBoneIK3D>(p_object) != nullptr;
}

void EditorInspectorPluginManyBoneIK::parse_begin(Object *p_object) {
	ManyBoneIK3D *ik = Object::cast_to<ManyBoneIK3D>(p_object);
	ERR_FAIL_COND(!ik);

	skel_editor = memnew(ManyBoneIK3DEditor(this, ik));
	add_custom_control(skel_editor);
}

void ManyBoneIK3DEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			if (joint_tree && ik) {
				update_joint_tree(ManyBoneIK3D::HumanoidMode(ik->get_humanoid_mode()));
				joint_tree->connect("item_selected", callable_mp(this, &ManyBoneIK3DEditor::_joint_tree_selection_changed));
			}
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			if (ik) {
				update_joint_tree(ManyBoneIK3D::HumanoidMode(ik->get_humanoid_mode()));
			}
		} break;
	}
}

void ManyBoneIK3DEditor::_update_properties() {
	Node3DEditor::get_singleton()->update_transform_gizmo();
}

void ManyBoneIK3DEditor::update_joint_tree(ManyBoneIK3D::HumanoidMode humanoid_mode) {
	if (!ik || !joint_tree) {
		return;
	}

	Skeleton3D *skeleton = ik->get_skeleton();
	if (!skeleton) {
		return;
	}

	joint_tree->clear();

	TreeItem *root = joint_tree->create_item();
	HashMap<int, TreeItem *> items;
	items.insert(-1, root);

	Ref<Texture> bone_icon = get_theme_icon(SNAME("BoneAttachment3D"), SNAME("EditorIcons"));
	Vector<int> bones_to_process = skeleton->get_parentless_bones();

	Ref<SkeletonProfileHumanoid> profile;
	profile.instantiate();

	HashSet<StringName> eleven_point_tracking_bones;
	eleven_point_tracking_bones.insert("Root");
	eleven_point_tracking_bones.insert("Hips");
	eleven_point_tracking_bones.insert("Head");
	eleven_point_tracking_bones.insert("LeftHand");
	eleven_point_tracking_bones.insert("RightHand");
	eleven_point_tracking_bones.insert("LeftUpperArm");
	eleven_point_tracking_bones.insert("RightUpperArm");
	eleven_point_tracking_bones.insert("LeftLowerLeg");
	eleven_point_tracking_bones.insert("RightLowerLeg");
	eleven_point_tracking_bones.insert("LeftFoot");
	eleven_point_tracking_bones.insert("RightFoot");

	HashSet<StringName> humanoid_bones;

	for (int i = 0; i < profile->get_bone_size(); ++i) {
		StringName bone_name = profile->get_bone_name(i);

		bool is_humanoid_bone = profile->has_bone(bone_name);
		if (is_humanoid_bone) {
			humanoid_bones.insert(bone_name);
		}
	}
	while (!bones_to_process.is_empty()) {
		int current_bone_idx = bones_to_process[0];
		bones_to_process.remove_at(0);

		StringName bone_name = skeleton->get_bone_name(current_bone_idx);
		const int parent_idx = skeleton->get_bone_parent(current_bone_idx);

		bool is_humanoid_bone = profile->has_bone(bone_name);

		bool should_add_bone = false;
		switch (humanoid_mode) {
			case ManyBoneIK3D::HumanoidMode::HUMANOID_MODE_ALL:
				should_add_bone = true;
				break;
			case ManyBoneIK3D::HumanoidMode::HUMANOID_MODE_HUMANOID:
				should_add_bone = is_humanoid_bone || is_bone_in_path_between_pins(current_bone_idx, humanoid_bones);
				break;
			case ManyBoneIK3D::HumanoidMode::HUMANOID_MODE_BODY:
				should_add_bone = eleven_point_tracking_bones.has(bone_name) || is_bone_in_path_between_pins(current_bone_idx, eleven_point_tracking_bones);
				break;
			default:
				break;
		}
		if (items.find(parent_idx) && should_add_bone) {
			TreeItem *parent_item = items.find(parent_idx)->value;
			TreeItem *joint_item = joint_tree->create_item(parent_item);
			items.insert(current_bone_idx, joint_item);

			joint_item->set_text(0, bone_name);
			joint_item->set_icon(0, bone_icon);
			joint_item->set_selectable(0, true);
			joint_item->set_metadata(0, "bones/" + itos(current_bone_idx));
		}
		// Add the bone's children to the list of bones to be processed.
		Vector<int> current_bone_child_bones = skeleton->get_bone_children(current_bone_idx);
		bones_to_process.append_array(current_bone_child_bones);
	}
}

void ManyBoneIK3DEditor::create_editors() {
	if (!ik || !ik->get_skeleton()) {
		return;
	}
	set_h_size_flags(SIZE_EXPAND_FILL);
	set_focus_mode(FOCUS_ALL);

	const Color section_color = get_theme_color(SNAME("prop_subsection"), SNAME("Editor"));
	EditorInspectorSection *bones_section = memnew(EditorInspectorSection);
	bones_section->setup("bones", "Bones", ik->get_skeleton(), section_color, true);
	add_child(bones_section);
	bones_section->unfold();

	ScrollContainer *s_con = memnew(ScrollContainer);
	s_con->set_h_size_flags(SIZE_EXPAND_FILL);
	s_con->set_custom_minimum_size(Size2(1, 350) * EDSCALE);
	bones_section->get_vbox()->add_child(s_con);

	joint_tree = memnew(Tree);
	joint_tree->set_columns(1);
	joint_tree->set_focus_mode(Control::FOCUS_NONE);
	joint_tree->set_select_mode(Tree::SELECT_SINGLE);
	joint_tree->set_hide_root(true);
	joint_tree->set_v_size_flags(SIZE_EXPAND_FILL);
	joint_tree->set_h_size_flags(SIZE_EXPAND_FILL);
	joint_tree->set_allow_rmb_select(true);
	s_con->add_child(joint_tree);
	constraint_bone_section = memnew(EditorInspectorSection);
	constraint_bone_section->setup("constraint_bone_properties", TTR("Constraint"), this, section_color, true);
	add_child(constraint_bone_section);

	bone_damp_float = memnew(EditorPropertyFloat());
	bone_damp_float->hide();
	bone_damp_float->setup(0, 180, 0.01, false, false, false, false, String::utf8("°"), true);
	bone_damp_float->set_label(TTR("Bone Damp"));
	bone_damp_float->set_selectable(false);
	bone_damp_float->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
	constraint_bone_section->get_vbox()->add_child(bone_damp_float);
	constraint_bone_section->unfold();

	target_nodepath = memnew(EditorPropertyNodePath());
	target_nodepath->hide();
	target_nodepath->set_label(TTR("Target NodePath"));
	target_nodepath->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
	constraint_bone_section->get_vbox()->add_child(target_nodepath);

	passthrough_float = memnew(EditorPropertyFloat());
	passthrough_float->hide();
	passthrough_float->setup(0, 1, 0.01, false, false, false, false, "", false);
	passthrough_float->set_label(TTR("Passthrough Factor"));
	passthrough_float->set_tooltip_text(TTR("Set to 0 to make this the exclusive target and set to 1 to have no more priority than any targets of any descendant bones."));
	passthrough_float->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
	constraint_bone_section->get_vbox()->add_child(passthrough_float);

	weight_float = memnew(EditorPropertyFloat());
	weight_float->hide();
	weight_float->setup(0, 1, 0.01, false, false, false, false, "", false);
	weight_float->set_label(TTR("Weight"));
	weight_float->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
	constraint_bone_section->get_vbox()->add_child(weight_float);

	direction_priorities_vector3 = memnew(EditorPropertyVector3());
	direction_priorities_vector3->setup(-1, 1, 0.05, false, false, "", false);
	direction_priorities_vector3->hide();
	direction_priorities_vector3->set_label(TTR("Direction Priorities"));
	direction_priorities_vector3->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
	constraint_bone_section->get_vbox()->add_child(direction_priorities_vector3);

	twist_current_float = memnew(EditorPropertyFloat());
	twist_current_float->hide();
	twist_current_float->setup(0, 1, 0.01, false, false, false, false);
	twist_current_float->set_label(TTR("Twist Current"));
	twist_current_float->set_selectable(false);
	twist_current_float->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
	constraint_bone_section->get_vbox()->add_child(twist_current_float);

	twist_from_float = memnew(EditorPropertyFloat());
	twist_from_float->hide();
	twist_from_float->setup(-360, 360, 0.01, false, false, false, false, String::utf8("°"), true);
	twist_from_float->set_label(TTR("Twist From"));
	twist_from_float->set_selectable(false);
	twist_from_float->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
	constraint_bone_section->get_vbox()->add_child(twist_from_float);

	twist_range_float = memnew(EditorPropertyFloat());
	twist_range_float->hide();
	twist_range_float->setup(-360, 360, 0.01, false, false, false, false, String::utf8("°"), true);
	twist_range_float->set_label(TTR("Twist Range"));
	twist_range_float->set_selectable(false);
	twist_range_float->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
	constraint_bone_section->get_vbox()->add_child(twist_range_float);

	cone_count_float = memnew(EditorPropertyFloat());
	cone_count_float->hide();
	cone_count_float->setup(0, 30, 1, false, false, "", false);
	cone_count_float->set_label(TTR("Limit Cone Count"));
	cone_count_float->set_selectable(false);
	cone_count_float->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
	constraint_bone_section->get_vbox()->add_child(cone_count_float);

	for (int32_t cone_i = 0; cone_i < MAX_KUSUDAMA_CONES; cone_i++) {
		center_vector3[cone_i] = memnew(EditorPropertyVector3());
		center_vector3[cone_i]->hide();
		center_vector3[cone_i]->setup(-1, 1, 0.001f, false, false, "", false);
		center_vector3[cone_i]->set_label(TTR(vformat("Cone Center Point %d", cone_i + 1)));
		center_vector3[cone_i]->set_selectable(false);
		center_vector3[cone_i]->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
		constraint_bone_section->get_vbox()->add_child(center_vector3[cone_i]);

		radius_float[cone_i] = memnew(EditorPropertyFloat());
		radius_float[cone_i]->hide();
		radius_float[cone_i]->setup(0, 180, 0.001f, false, false, false, false, String::utf8("°"), true);
		radius_float[cone_i]->set_label(TTR(vformat("Cone Radius %d", cone_i + 1)));
		radius_float[cone_i]->set_selectable(false);
		radius_float[cone_i]->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
		constraint_bone_section->get_vbox()->add_child(radius_float[cone_i]);
	}

	twist_constraint_transform = memnew(EditorPropertyTransform3D());
	twist_constraint_transform->hide();
	twist_constraint_transform->set_label(TTR("Twist Constraint"));
	twist_constraint_transform->set_selectable(false);
	twist_constraint_transform->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
	constraint_bone_section->get_vbox()->add_child(twist_constraint_transform);

	orientation_constraint_transform = memnew(EditorPropertyTransform3D());
	orientation_constraint_transform->hide();
	orientation_constraint_transform->set_label(TTR("Orientation Constraint"));
	orientation_constraint_transform->set_selectable(false);
	orientation_constraint_transform->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
	constraint_bone_section->get_vbox()->add_child(orientation_constraint_transform);

	bone_direction_transform = memnew(EditorPropertyTransform3D());
	bone_direction_transform->hide();
	bone_direction_transform->set_label(TTR("Bone Direction"));
	bone_direction_transform->set_selectable(false);
	bone_direction_transform->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
	constraint_bone_section->get_vbox()->add_child(bone_direction_transform);
}

void ManyBoneIK3DEditor::_joint_tree_selection_changed() {
	TreeItem *selected = joint_tree->get_selected();
	if (!selected) {
		return;
	}
	const String path = selected->get_metadata(0);
	if (!path.begins_with("bones/")) {
		return;
	}
	const int b_idx = path.get_slicec('/', 1).to_int();
	selected_bone = b_idx;

	bone_damp_float->hide();
	target_nodepath->hide();
	twist_current_float->hide();
	twist_from_float->hide();
	twist_range_float->hide();
	cone_count_float->hide();
	for (int32_t cone_i = 0; cone_i < MAX_KUSUDAMA_CONES; cone_i++) {
		center_vector3[cone_i]->hide();
		radius_float[cone_i]->hide();
	}
	twist_constraint_transform->hide();
	orientation_constraint_transform->hide();
	bone_direction_transform->hide();
	passthrough_float->hide();
	weight_float->hide();
	direction_priorities_vector3->hide();

	Skeleton3D *skeleton = ik->get_skeleton();
	if (!skeleton) {
		return;
	}
	String bone_name = ik->get_skeleton()->get_bone_name(selected_bone);
	if (bone_name.is_empty()) {
		return;
	}
	bone_damp_float->set_object_and_property(ik, vformat("bone/%d/damp", selected_bone));
	bone_damp_float->update_property();
	bone_damp_float->show();
	target_nodepath->set_object_and_property(ik, vformat("pins/%d/target_node", selected_bone));
	target_nodepath->update_property();
	target_nodepath->show();
	passthrough_float->set_object_and_property(ik, vformat("pins/%d/passthrough_factor", selected_bone));
	passthrough_float->update_property();
	passthrough_float->show();
	weight_float->set_object_and_property(ik, vformat("pins/%d/weight", selected_bone));
	weight_float->update_property();
	weight_float->show();
	direction_priorities_vector3->set_object_and_property(ik, vformat("pins/%d/direction_priorities", selected_bone));
	direction_priorities_vector3->update_property();
	direction_priorities_vector3->show();
	twist_current_float->set_object_and_property(ik, vformat("constraints/%d/twist_current", selected_bone));
	twist_current_float->update_property();
	twist_current_float->show();
	twist_from_float->set_object_and_property(ik, vformat("constraints/%d/twist_from", selected_bone));
	twist_from_float->update_property();
	twist_from_float->show();
	twist_range_float->set_object_and_property(ik, vformat("constraints/%d/twist_range", selected_bone));
	twist_range_float->update_property();
	twist_range_float->show();
	cone_count_float->set_object_and_property(ik, vformat("constraints/%d/kusudama_limit_cone_count", selected_bone));
	cone_count_float->update_property();
	cone_count_float->show();
	for (int32_t cone_i = 0; cone_i < MAX_KUSUDAMA_CONES; cone_i++) {
		center_vector3[cone_i]->hide();
		radius_float[cone_i]->hide();
	}
	for (int32_t cone_i = 0; cone_i < ik->get_kusudama_limit_cone_count(selected_bone); cone_i++) {
		center_vector3[cone_i]->set_object_and_property(ik, vformat("constraints/%d/kusudama_limit_cone/%d/center", selected_bone, cone_i));
		center_vector3[cone_i]->update_property();
		center_vector3[cone_i]->show();
		radius_float[cone_i]->set_object_and_property(ik, vformat("constraints/%d/kusudama_limit_cone/%d/radius", selected_bone, cone_i));
		radius_float[cone_i]->update_property();
		radius_float[cone_i]->show();
	}
	twist_constraint_transform->set_object_and_property(ik, vformat("constraints/%d/kusudama_twist", selected_bone));
	twist_constraint_transform->update_property();
	twist_constraint_transform->show();
	orientation_constraint_transform->set_object_and_property(ik, vformat("constraints/%d/kusudama_orientation", selected_bone));
	orientation_constraint_transform->update_property();
	orientation_constraint_transform->show();
	bone_direction_transform->set_object_and_property(ik, vformat("constraints/%d/bone_direction", selected_bone));
	bone_direction_transform->update_property();
	bone_direction_transform->show();
	_update_properties();
}

void ManyBoneIK3DEditor::select_bone(int p_idx) {
	if (p_idx < 0) {
		selected_bone = -1;
		return;
	}
	selected_bone = p_idx;
	_joint_tree_selection_changed();
}

TreeItem *ManyBoneIK3DEditor::_find(TreeItem *p_node, const NodePath &p_path) {
	if (!p_node) {
		return nullptr;
	}

	NodePath np = p_node->get_metadata(0);
	if (np == p_path) {
		return p_node;
	}

	TreeItem *children = p_node->get_first_child();
	while (children) {
		TreeItem *n = _find(children, p_path);
		if (n) {
			return n;
		}
		children = children->get_next();
	}

	return nullptr;
}

void ManyBoneIK3DEditor::_value_changed(const String &p_property, Variant p_value, const String &p_name, bool p_changing) {
	if (!is_visible() || !ik) {
		return;
	}

	EditorUndoRedoManager *undo_redo = EditorUndoRedoManager::get_singleton();
	undo_redo->create_action(TTR("Set ManyBoneIK Property"), UndoRedo::MERGE_ENDS);
	undo_redo->add_undo_property(ik, p_property, ik->get(p_property));
	undo_redo->add_do_property(ik, p_property, p_value);
	undo_redo->commit_action();

	for (int32_t cone_i = 0; cone_i < MAX_KUSUDAMA_CONES; cone_i++) {
		center_vector3[cone_i]->hide();
		radius_float[cone_i]->hide();
	}

	int32_t limit_cone_count = ik->get_kusudama_limit_cone_count(selected_bone);
	for (int32_t cone_i = 0; cone_i < limit_cone_count; cone_i++) {
		String center_property = vformat("constraints/%d/kusudama_limit_cone/%d/center", selected_bone, cone_i);
		center_vector3[cone_i]->set_object_and_property(ik, center_property);
		center_vector3[cone_i]->update_property();
		center_vector3[cone_i]->show();

		String radius_property = vformat("constraints/%d/kusudama_limit_cone/%d/radius", selected_bone, cone_i);
		radius_float[cone_i]->set_object_and_property(ik, radius_property);
		radius_float[cone_i]->update_property();
		radius_float[cone_i]->show();
	}

	_update_properties();
}

ManyBoneIK3DEditorPlugin::ManyBoneIK3DEditorPlugin() {
	skeleton_plugin = memnew(EditorInspectorPluginManyBoneIK);

	EditorInspector::add_inspector_plugin(skeleton_plugin);

	Ref<Skeleton3DGizmoPlugin> gizmo_plugin = Ref<Skeleton3DGizmoPlugin>(memnew(Skeleton3DGizmoPlugin));
	Node3DEditor::get_singleton()->add_gizmo_plugin(gizmo_plugin);
}

ManyBoneIK3DEditor::ManyBoneIK3DEditor(EditorInspectorPluginManyBoneIK *e_plugin, ManyBoneIK3D *p_ik) {
	ik = p_ik;
	create_editors();
}

EditorInspectorPluginManyBoneIK::EditorInspectorPluginManyBoneIK() {
}

void ManyBoneIK3DEditor::_bind_methods() {
}

bool ManyBoneIK3DEditor::is_bone_in_path_between_pins(int p_bone_idx, const HashSet<StringName> &p_pins) {
	Skeleton3D *skeleton = ik->get_skeleton();
	if (!skeleton || p_pins.is_empty()) {
		return false;
	}

	for (const StringName &pin : p_pins) {
		int pin_bone_idx = skeleton->find_bone(pin);
		if (pin_bone_idx == -1) {
			continue;
		}

		while (pin_bone_idx != -1) {
			if (pin_bone_idx == p_bone_idx) {
				return true;
			}
			pin_bone_idx = skeleton->get_bone_parent(pin_bone_idx);
		}
	}

	return false;
}
