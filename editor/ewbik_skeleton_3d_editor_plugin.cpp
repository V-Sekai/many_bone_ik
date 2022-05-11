/*************************************************************************/
/*  skeleton_3d_editor_plugin.cpp                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "ewbik_skeleton_3d_editor_plugin.h"

#include "core/io/resource_saver.h"
#include "editor/editor_file_dialog.h"
#include "editor/editor_node.h"
#include "editor/editor_properties.h"
#include "editor/editor_scale.h"
#include "editor/plugins/animation_player_editor_plugin.h"
#include "editor/plugins/node_3d_editor_plugin.h"
#include "scene/3d/collision_shape_3d.h"
#include "scene/3d/joint_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/physics_body_3d.h"
#include "scene/resources/capsule_shape_3d.h"
#include "scene/resources/primitive_meshes.h"
#include "scene/resources/sphere_shape_3d.h"
#include "scene/resources/surface_tool.h"

void EWBIKBoneTransformEditor::create_editors() {
	const Color section_color = get_theme_color(SNAME("prop_subsection"), SNAME("Editor"));

	section = memnew(EditorInspectorSection);
	section->setup("trf_properties", label, this, section_color, true);
	section->unfold();
	add_child(section);

	enabled_checkbox = memnew(EditorPropertyCheck());
	enabled_checkbox->set_label("Pose Enabled");
	enabled_checkbox->set_selectable(false);
	enabled_checkbox->connect("property_changed", callable_mp(this, &EWBIKBoneTransformEditor::_value_changed));
	section->get_vbox()->add_child(enabled_checkbox);

	// Position property.
	position_property = memnew(EditorPropertyVector3());
	position_property->setup(-10000, 10000, 0.001f, true);
	position_property->set_label("Position");
	position_property->set_selectable(false);
	position_property->connect("property_changed", callable_mp(this, &EWBIKBoneTransformEditor::_value_changed));
	position_property->connect("property_keyed", callable_mp(this, &EWBIKBoneTransformEditor::_property_keyed));
	section->get_vbox()->add_child(position_property);

	// Rotation property.
	rotation_property = memnew(EditorPropertyQuaternion());
	rotation_property->setup(-10000, 10000, 0.001f, true);
	rotation_property->set_label("Rotation");
	rotation_property->set_selectable(false);
	rotation_property->connect("property_changed", callable_mp(this, &EWBIKBoneTransformEditor::_value_changed));
	rotation_property->connect("property_keyed", callable_mp(this, &EWBIKBoneTransformEditor::_property_keyed));
	section->get_vbox()->add_child(rotation_property);

	// Scale property.
	scale_property = memnew(EditorPropertyVector3());
	scale_property->setup(-10000, 10000, 0.001f, true);
	scale_property->set_label("Scale");
	scale_property->set_selectable(false);
	scale_property->connect("property_changed", callable_mp(this, &EWBIKBoneTransformEditor::_value_changed));
	scale_property->connect("property_keyed", callable_mp(this, &EWBIKBoneTransformEditor::_property_keyed));
	section->get_vbox()->add_child(scale_property);

	// Transform/Matrix section.
	rest_section = memnew(EditorInspectorSection);
	rest_section->setup("trf_properties_transform", "Rest", this, section_color, true);
	section->get_vbox()->add_child(rest_section);

	// Transform/Matrix property.
	rest_matrix = memnew(EditorPropertyTransform3D());
	rest_matrix->setup(-10000, 10000, 0.001f, true);
	rest_matrix->set_label("Transform");
	rest_matrix->set_selectable(false);
	rest_section->get_vbox()->add_child(rest_matrix);
}

void EWBIKBoneTransformEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			create_editors();
		} break;
	}
}

void EWBIKBoneTransformEditor::_value_changed(const String &p_property, Variant p_value, const String &p_name, bool p_changing) {
	if (updating) {
		return;
	}
	if (skeleton) {
		undo_redo->create_action(TTR("Set Bone Transform"), UndoRedo::MERGE_ENDS);
		undo_redo->add_undo_property(skeleton, p_property, skeleton->get(p_property));
		undo_redo->add_do_property(skeleton, p_property, p_value);
		undo_redo->commit_action();
	}
}

EWBIKBoneTransformEditor::EWBIKBoneTransformEditor(Skeleton3D *p_skeleton) :
		skeleton(p_skeleton) {
	undo_redo = EditorNode::get_undo_redo();
}

void EWBIKBoneTransformEditor::set_keyable(const bool p_keyable) {
	position_property->set_keying(p_keyable);
	rotation_property->set_keying(p_keyable);
	scale_property->set_keying(p_keyable);
}

void EWBIKBoneTransformEditor::set_target(const String &p_prop) {
	enabled_checkbox->set_object_and_property(skeleton, p_prop + "enabled");
	enabled_checkbox->update_property();

	position_property->set_object_and_property(skeleton, p_prop + "position");
	position_property->update_property();

	rotation_property->set_object_and_property(skeleton, p_prop + "rotation");
	rotation_property->update_property();

	scale_property->set_object_and_property(skeleton, p_prop + "scale");
	scale_property->update_property();

	rest_matrix->set_object_and_property(skeleton, p_prop + "rest");
	rest_matrix->update_property();
}

void EWBIKBoneTransformEditor::_property_keyed(const String &p_path, bool p_advance) {
	AnimationTrackEditor *te = AnimationPlayerEditor::get_singleton()->get_track_editor();
	if (!te->has_keying()) {
		return;
	}
	PackedStringArray split = p_path.split("/");
	if (split.size() == 3 && split[0] == "bones") {
		int bone_idx = split[1].to_int();
		if (split[2] == "position") {
			te->insert_transform_key(skeleton, skeleton->get_bone_name(bone_idx), Animation::TYPE_POSITION_3D, skeleton->get(p_path));
		}
		if (split[2] == "rotation") {
			te->insert_transform_key(skeleton, skeleton->get_bone_name(bone_idx), Animation::TYPE_ROTATION_3D, skeleton->get(p_path));
		}
		if (split[2] == "scale") {
			te->insert_transform_key(skeleton, skeleton->get_bone_name(bone_idx), Animation::TYPE_SCALE_3D, skeleton->get(p_path));
		}
	}
}

void EWBIKBoneTransformEditor::_update_properties() {
	if (!skeleton) {
		return;
	}
	int selected = EWBIKSkeleton3DEditor::get_singleton()->get_selected_bone();
	List<PropertyInfo> props;
	skeleton->get_property_list(&props);
	for (const PropertyInfo &E : props) {
		PackedStringArray split = E.name.split("/");
		if (split.size() == 3 && split[0] == "bones") {
			if (split[1].to_int() == selected) {
				if (split[2] == "enabled") {
					enabled_checkbox->set_read_only(E.usage & PROPERTY_USAGE_READ_ONLY);
					enabled_checkbox->update_property();
					enabled_checkbox->update();
				}
				if (split[2] == "position") {
					position_property->set_read_only(E.usage & PROPERTY_USAGE_READ_ONLY);
					position_property->update_property();
					position_property->update();
				}
				if (split[2] == "rotation") {
					rotation_property->set_read_only(E.usage & PROPERTY_USAGE_READ_ONLY);
					rotation_property->update_property();
					rotation_property->update();
				}
				if (split[2] == "scale") {
					scale_property->set_read_only(E.usage & PROPERTY_USAGE_READ_ONLY);
					scale_property->update_property();
					scale_property->update();
				}
				if (split[2] == "rest") {
					rest_matrix->set_read_only(E.usage & PROPERTY_USAGE_READ_ONLY);
					rest_matrix->update_property();
					rest_matrix->update();
				}
			}
		}
	}
}

EWBIKSkeleton3DEditor *EWBIKSkeleton3DEditor::singleton = nullptr;

void EWBIKSkeleton3DEditor::set_keyable(const bool p_keyable) {
	keyable = p_keyable;
	if (p_keyable) {
		animation_hb->show();
	} else {
		animation_hb->hide();
	}
};

void EWBIKSkeleton3DEditor::init_pose(const bool p_all_bones) {
	if (!skeleton) {
		return;
	}
	const int bone_len = skeleton->get_bone_count();
	if (!bone_len) {
		return;
	}

	UndoRedo *ur = EditorNode::get_singleton()->get_undo_redo();
	ur->create_action(TTR("Set Bone Transform"), UndoRedo::MERGE_ENDS);
	if (p_all_bones) {
		for (int i = 0; i < bone_len; i++) {
			Transform3D rest = skeleton->get_bone_rest(i);
			ur->add_do_method(skeleton, "set_bone_pose_position", i, rest.origin);
			ur->add_do_method(skeleton, "set_bone_pose_rotation", i, rest.basis.get_rotation_quaternion());
			ur->add_do_method(skeleton, "set_bone_pose_scale", i, rest.basis.get_scale());
			ur->add_undo_method(skeleton, "set_bone_pose_position", i, skeleton->get_bone_pose_position(i));
			ur->add_undo_method(skeleton, "set_bone_pose_rotation", i, skeleton->get_bone_pose_rotation(i));
			ur->add_undo_method(skeleton, "set_bone_pose_scale", i, skeleton->get_bone_pose_scale(i));
		}
	} else {
		// Todo: Do method with multiple bone selection.
		if (selected_bone == -1) {
			ur->commit_action();
			return;
		}
		Transform3D rest = skeleton->get_bone_rest(selected_bone);
		ur->add_do_method(skeleton, "set_bone_pose_position", selected_bone, rest.origin);
		ur->add_do_method(skeleton, "set_bone_pose_rotation", selected_bone, rest.basis.get_rotation_quaternion());
		ur->add_do_method(skeleton, "set_bone_pose_scale", selected_bone, rest.basis.get_scale());
		ur->add_undo_method(skeleton, "set_bone_pose_position", selected_bone, skeleton->get_bone_pose_position(selected_bone));
		ur->add_undo_method(skeleton, "set_bone_pose_rotation", selected_bone, skeleton->get_bone_pose_rotation(selected_bone));
		ur->add_undo_method(skeleton, "set_bone_pose_scale", selected_bone, skeleton->get_bone_pose_scale(selected_bone));
	}
	ur->commit_action();
}

void EWBIKSkeleton3DEditor::insert_keys(const bool p_all_bones) {
	if (!skeleton) {
		return;
	}

	bool pos_enabled = key_loc_button->is_pressed();
	bool rot_enabled = key_rot_button->is_pressed();
	bool scl_enabled = key_scale_button->is_pressed();

	int bone_len = skeleton->get_bone_count();
	Node *root = EditorNode::get_singleton()->get_tree()->get_root();
	String path = root->get_path_to(skeleton);

	AnimationTrackEditor *te = AnimationPlayerEditor::get_singleton()->get_track_editor();
	te->make_insert_queue();
	for (int i = 0; i < bone_len; i++) {
		const String name = skeleton->get_bone_name(i);

		if (name.is_empty()) {
			continue;
		}

		if (pos_enabled && (p_all_bones || te->has_track(skeleton, name, Animation::TYPE_POSITION_3D))) {
			te->insert_transform_key(skeleton, name, Animation::TYPE_POSITION_3D, skeleton->get_bone_pose_position(i));
		}
		if (rot_enabled && (p_all_bones || te->has_track(skeleton, name, Animation::TYPE_ROTATION_3D))) {
			te->insert_transform_key(skeleton, name, Animation::TYPE_ROTATION_3D, skeleton->get_bone_pose_rotation(i));
		}
		if (scl_enabled && (p_all_bones || te->has_track(skeleton, name, Animation::TYPE_SCALE_3D))) {
			te->insert_transform_key(skeleton, name, Animation::TYPE_SCALE_3D, skeleton->get_bone_pose_scale(i));
		}
	}
	te->commit_insert_queue();
}

void EWBIKSkeleton3DEditor::pose_to_rest(const bool p_all_bones) {
	if (!skeleton) {
		return;
	}
	const int bone_len = skeleton->get_bone_count();
	if (!bone_len) {
		return;
	}

	UndoRedo *ur = EditorNode::get_singleton()->get_undo_redo();
	ur->create_action(TTR("Set Bone Rest"), UndoRedo::MERGE_ENDS);
	if (p_all_bones) {
		for (int i = 0; i < bone_len; i++) {
			ur->add_do_method(skeleton, "set_bone_rest", i, skeleton->get_bone_pose(i));
			ur->add_undo_method(skeleton, "set_bone_rest", i, skeleton->get_bone_rest(i));
		}
	} else {
		// Todo: Do method with multiple bone selection.
		if (selected_bone == -1) {
			ur->commit_action();
			return;
		}
		ur->add_do_method(skeleton, "set_bone_rest", selected_bone, skeleton->get_bone_pose(selected_bone));
		ur->add_undo_method(skeleton, "set_bone_rest", selected_bone, skeleton->get_bone_rest(selected_bone));
	}
	ur->commit_action();
}

void EWBIKSkeleton3DEditor::create_physical_skeleton() {
	UndoRedo *ur = EditorNode::get_singleton()->get_undo_redo();
	ERR_FAIL_COND(!get_tree());
	Node *owner = get_tree()->get_edited_scene_root();

	const int bc = skeleton->get_bone_count();

	if (!bc) {
		return;
	}

	Vector<BoneInfo> bones_infos;
	bones_infos.resize(bc);

	if (bc > 0) {
		ur->create_action(TTR("Create physical bones"), UndoRedo::MERGE_ALL);
		for (int bone_id = 0; bc > bone_id; ++bone_id) {
			const int parent = skeleton->get_bone_parent(bone_id);

			if (parent < 0) {
				bones_infos.write[bone_id].relative_rest = skeleton->get_bone_rest(bone_id);
			} else {
				const int parent_parent = skeleton->get_bone_parent(parent);

				bones_infos.write[bone_id].relative_rest = bones_infos[parent].relative_rest * skeleton->get_bone_rest(bone_id);

				// Create physical bone on parent.
				if (!bones_infos[parent].physical_bone) {
					PhysicalBone3D *physical_bone = create_physical_bone(parent, bone_id, bones_infos);
					if (physical_bone && physical_bone->get_child(0)) {
						CollisionShape3D *collision_shape = Object::cast_to<CollisionShape3D>(physical_bone->get_child(0));
						if (collision_shape) {
							bones_infos.write[parent].physical_bone = physical_bone;

							ur->add_do_method(skeleton, "add_child", physical_bone);
							ur->add_do_method(physical_bone, "set_owner", owner);
							ur->add_do_method(collision_shape, "set_owner", owner);
							ur->add_do_property(physical_bone, "bone_name", skeleton->get_bone_name(parent));

							// Create joint between parent of parent.
							if (parent_parent != -1) {
								ur->add_do_method(physical_bone, "set_joint_type", PhysicalBone3D::JOINT_TYPE_PIN);
							}

							ur->add_do_method(Node3DEditor::get_singleton(), "_request_gizmo", physical_bone);
							ur->add_do_method(Node3DEditor::get_singleton(), "_request_gizmo", collision_shape);

							ur->add_do_reference(physical_bone);
							ur->add_undo_method(skeleton, "remove_child", physical_bone);
						}
					}
				}
			}
		}
		ur->commit_action();
	}
}

PhysicalBone3D *EWBIKSkeleton3DEditor::create_physical_bone(int bone_id, int bone_child_id, const Vector<BoneInfo> &bones_infos) {
	const Transform3D child_rest = skeleton->get_bone_rest(bone_child_id);

	const real_t half_height(child_rest.origin.length() * 0.5);
	const real_t radius(half_height * 0.2);

	CapsuleShape3D *bone_shape_capsule = memnew(CapsuleShape3D);
	bone_shape_capsule->set_height(half_height * 2);
	bone_shape_capsule->set_radius(radius);

	CollisionShape3D *bone_shape = memnew(CollisionShape3D);
	bone_shape->set_shape(bone_shape_capsule);
	bone_shape->set_name("CollisionShape3D");

	Transform3D capsule_transform;
	capsule_transform.basis = Basis(Vector3(1, 0, 0), Vector3(0, 0, 1), Vector3(0, -1, 0));
	bone_shape->set_transform(capsule_transform);

	/// Get an up vector not collinear with child rest origin
	Vector3 up = Vector3(0, 1, 0);
	if (up.cross(child_rest.origin).is_equal_approx(Vector3())) {
		up = Vector3(0, 0, 1);
	}

	Transform3D body_transform;
	body_transform.basis = Basis::looking_at(child_rest.origin, up);
	body_transform.origin = body_transform.basis.xform(Vector3(0, 0, -half_height));

	Transform3D joint_transform;
	joint_transform.origin = Vector3(0, 0, half_height);

	PhysicalBone3D *physical_bone = memnew(PhysicalBone3D);
	physical_bone->add_child(bone_shape);
	physical_bone->set_name("Physical Bone " + skeleton->get_bone_name(bone_id));
	physical_bone->set_body_offset(body_transform);
	physical_bone->set_joint_offset(joint_transform);
	return physical_bone;
}

Variant EWBIKSkeleton3DEditor::get_drag_data_fw(const Point2 &p_point, Control *p_from) {
	TreeItem *selected = joint_tree->get_selected();

	if (!selected) {
		return Variant();
	}

	Ref<Texture> icon = selected->get_icon(0);

	VBoxContainer *vb = memnew(VBoxContainer);
	HBoxContainer *hb = memnew(HBoxContainer);
	TextureRect *tf = memnew(TextureRect);
	tf->set_texture(icon);
	tf->set_stretch_mode(TextureRect::STRETCH_KEEP_CENTERED);
	hb->add_child(tf);
	Label *label = memnew(Label(selected->get_text(0)));
	hb->add_child(label);
	vb->add_child(hb);
	hb->set_modulate(Color(1, 1, 1, 1));

	set_drag_preview(vb);
	Dictionary drag_data;
	drag_data["type"] = "nodes";
	drag_data["node"] = selected;

	return drag_data;
}

bool EWBIKSkeleton3DEditor::can_drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from) const {
	TreeItem *target = joint_tree->get_item_at_position(p_point);
	if (!target) {
		return false;
	}

	const String path = target->get_metadata(0);
	if (!path.begins_with("bones/")) {
		return false;
	}

	TreeItem *selected = Object::cast_to<TreeItem>(Dictionary(p_data)["node"]);
	if (target == selected) {
		return false;
	}

	const String path2 = target->get_metadata(0);
	if (!path2.begins_with("bones/")) {
		return false;
	}

	return true;
}

void EWBIKSkeleton3DEditor::drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from) {
	if (!can_drop_data_fw(p_point, p_data, p_from)) {
		return;
	}

	TreeItem *target = joint_tree->get_item_at_position(p_point);
	TreeItem *selected = Object::cast_to<TreeItem>(Dictionary(p_data)["node"]);

	const BoneId target_boneidx = String(target->get_metadata(0)).get_slicec('/', 1).to_int();
	const BoneId selected_boneidx = String(selected->get_metadata(0)).get_slicec('/', 1).to_int();

	move_skeleton_bone(skeleton->get_path(), selected_boneidx, target_boneidx);
}

void EWBIKSkeleton3DEditor::move_skeleton_bone(NodePath p_skeleton_path, int32_t p_selected_boneidx, int32_t p_target_boneidx) {
	Node *node = get_node_or_null(p_skeleton_path);
	Skeleton3D *skeleton = Object::cast_to<Skeleton3D>(node);
	ERR_FAIL_NULL(skeleton);
	UndoRedo *ur = EditorNode::get_singleton()->get_undo_redo();
	ur->create_action(TTR("Set Bone Parentage"));
	// If the target is a child of ourselves, we move only *us* and not our children.
	if (skeleton->is_bone_parent_of(p_target_boneidx, p_selected_boneidx)) {
		const BoneId parent_idx = skeleton->get_bone_parent(p_selected_boneidx);
		const int bone_count = skeleton->get_bone_count();
		for (BoneId i = 0; i < bone_count; ++i) {
			if (skeleton->get_bone_parent(i) == p_selected_boneidx) {
				ur->add_undo_method(skeleton, "set_bone_parent", i, skeleton->get_bone_parent(i));
				ur->add_do_method(skeleton, "set_bone_parent", i, parent_idx);
				skeleton->set_bone_parent(i, parent_idx);
			}
		}
	}
	ur->add_undo_method(skeleton, "set_bone_parent", p_selected_boneidx, skeleton->get_bone_parent(p_selected_boneidx));
	ur->add_do_method(skeleton, "set_bone_parent", p_selected_boneidx, p_target_boneidx);
	skeleton->set_bone_parent(p_selected_boneidx, p_target_boneidx);

	update_joint_tree();
	ur->commit_action();
}

void EWBIKSkeleton3DEditor::_joint_tree_selection_changed() {
	TreeItem *selected = joint_tree->get_selected();
	if (selected) {
		const String path = selected->get_metadata(0);
		if (!path.begins_with("bones/")) {
			return;
		}
		const int b_idx = path.get_slicec('/', 1).to_int();
		selected_bone = b_idx;
		if (pose_editor) {
			const String bone_path = "bones/" + itos(b_idx) + "/";
			pose_editor->set_target(bone_path);
			pose_editor->set_keyable(keyable);
		}
	}

	if (pose_editor && pose_editor->is_inside_tree()) {
		pose_editor->set_visible(selected);
	}
	_update_properties();
	_update_gizmo_visible();
}

// May be not used with single select mode.
void EWBIKSkeleton3DEditor::_joint_tree_rmb_select(const Vector2 &p_pos) {
}

void EWBIKSkeleton3DEditor::_update_properties() {
	if (pose_editor) {
		pose_editor->_update_properties();
	}
	Node3DEditor::get_singleton()->update_transform_gizmo();
}

void EWBIKSkeleton3DEditor::update_joint_tree() {
	joint_tree->clear();

	if (!skeleton) {
		return;
	}

	TreeItem *root = joint_tree->create_item();

	Map<int, TreeItem *> items;

	items.insert(-1, root);

	Ref<Texture> bone_icon = get_theme_icon(SNAME("BoneAttachment3D"), SNAME("EditorIcons"));

	Vector<int> bones_to_process = skeleton->get_parentless_bones();
	while (bones_to_process.size() > 0) {
		int current_bone_idx = bones_to_process[0];
		bones_to_process.erase(current_bone_idx);

		const int parent_idx = skeleton->get_bone_parent(current_bone_idx);
		TreeItem *parent_item = items.find(parent_idx)->get();

		TreeItem *joint_item = joint_tree->create_item(parent_item);
		items.insert(current_bone_idx, joint_item);

		joint_item->set_text(0, skeleton->get_bone_name(current_bone_idx));
		joint_item->set_icon(0, bone_icon);
		joint_item->set_selectable(0, true);
		joint_item->set_metadata(0, "bones/" + itos(current_bone_idx));

		// Add the bone's children to the list of bones to be processed.
		Vector<int> current_bone_child_bones = skeleton->get_bone_children(current_bone_idx);
		int child_bone_size = current_bone_child_bones.size();
		for (int i = 0; i < child_bone_size; i++) {
			bones_to_process.push_back(current_bone_child_bones[i]);
		}
	}
}

void EWBIKSkeleton3DEditor::update_editors() {
}

void EWBIKSkeleton3DEditor::create_editors() {
	set_h_size_flags(SIZE_EXPAND_FILL);
	add_theme_constant_override("separation", 0);

	set_focus_mode(FOCUS_ALL);

	Node3DEditor *ne = Node3DEditor::get_singleton();
	AnimationTrackEditor *te = AnimationPlayerEditor::get_singleton()->get_track_editor();

	separator = memnew(VSeparator);
	ne->add_control_to_menu_panel(separator);

	Vector<Variant> button_binds;
	button_binds.resize(1);

	edit_mode_button = memnew(Button);
	ne->add_control_to_menu_panel(edit_mode_button);
	edit_mode_button->set_flat(true);
	edit_mode_button->set_toggle_mode(true);
	edit_mode_button->set_focus_mode(FOCUS_NONE);
	edit_mode_button->set_tooltip(TTR("Constraint Edit Mode\nShow buttons on joint constraints."));
	edit_mode_button->connect("toggled", callable_mp(this, &EWBIKSkeleton3DEditor::edit_mode_toggled));

	edit_mode = false;

	if (skeleton) {
		skeleton->add_child(handles_mesh_instance);
		handles_mesh_instance->set_skeleton_path(NodePath(""));
	}

	// Keying buttons.
	animation_hb = memnew(HBoxContainer);
	ne->add_control_to_menu_panel(animation_hb);
	animation_hb->hide();

	key_loc_button = memnew(Button);
	key_loc_button->set_flat(true);
	key_loc_button->set_toggle_mode(true);
	key_loc_button->set_pressed(false);
	key_loc_button->set_focus_mode(FOCUS_NONE);
	key_loc_button->set_tooltip(TTR("Translation mask for inserting keys."));
	animation_hb->add_child(key_loc_button);

	key_rot_button = memnew(Button);
	key_rot_button->set_flat(true);
	key_rot_button->set_toggle_mode(true);
	key_rot_button->set_pressed(true);
	key_rot_button->set_focus_mode(FOCUS_NONE);
	key_rot_button->set_tooltip(TTR("Rotation mask for inserting keys."));
	animation_hb->add_child(key_rot_button);

	key_scale_button = memnew(Button);
	key_scale_button->set_flat(true);
	key_scale_button->set_toggle_mode(true);
	key_scale_button->set_pressed(false);
	key_scale_button->set_focus_mode(FOCUS_NONE);
	key_scale_button->set_tooltip(TTR("Scale mask for inserting keys."));
	animation_hb->add_child(key_scale_button);

	key_insert_button = memnew(Button);
	key_insert_button->set_flat(true);
	key_insert_button->set_focus_mode(FOCUS_NONE);
	key_insert_button->connect("pressed", callable_mp(this, &EWBIKSkeleton3DEditor::insert_keys), varray(false));
	key_insert_button->set_tooltip(TTR("Insert key of bone poses already exist track."));
	key_insert_button->set_shortcut(ED_SHORTCUT("skeleton_3d_editor/insert_key_to_existing_tracks", TTR("Insert Key (Existing Tracks)"), Key::INSERT));
	animation_hb->add_child(key_insert_button);

	key_insert_all_button = memnew(Button);
	key_insert_all_button->set_flat(true);
	key_insert_all_button->set_focus_mode(FOCUS_NONE);
	key_insert_all_button->connect("pressed", callable_mp(this, &EWBIKSkeleton3DEditor::insert_keys), varray(true));
	key_insert_all_button->set_tooltip(TTR("Insert key of all bone poses."));
	key_insert_all_button->set_shortcut(ED_SHORTCUT("skeleton_3d_editor/insert_key_of_all_bones", TTR("Insert Key (All Bones)"), KeyModifierMask::CMD + Key::INSERT));
	animation_hb->add_child(key_insert_all_button);

	// Bone tree.
	const Color section_color = get_theme_color(SNAME("prop_subsection"), SNAME("Editor"));

	EditorInspectorSection *bones_section = memnew(EditorInspectorSection);
	bones_section->setup("bones", "Bones", skeleton, section_color, true);
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
	joint_tree->set_drag_forwarding(this);
	s_con->add_child(joint_tree);

	pose_editor = memnew(EWBIKBoneTransformEditor(skeleton));
	pose_editor->set_label(TTR("Bone Transform"));
	pose_editor->set_visible(false);
	add_child(pose_editor);

	set_keyable(te->has_keying());
}

void EWBIKSkeleton3DEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			edit_mode_button->set_icon(get_theme_icon(SNAME("ToolRotate"), SNAME("EditorIcons")));
			key_loc_button->set_icon(get_theme_icon(SNAME("KeyPosition"), SNAME("EditorIcons")));
			key_rot_button->set_icon(get_theme_icon(SNAME("KeyRotation"), SNAME("EditorIcons")));
			key_scale_button->set_icon(get_theme_icon(SNAME("KeyScale"), SNAME("EditorIcons")));
			key_insert_button->set_icon(get_theme_icon(SNAME("Key"), SNAME("EditorIcons")));
			key_insert_all_button->set_icon(get_theme_icon(SNAME("NewKey"), SNAME("EditorIcons")));
			get_tree()->connect("node_removed", callable_mp(this, &EWBIKSkeleton3DEditor::_node_removed), Vector<Variant>(), Object::CONNECT_ONESHOT);
			break;
		}
		case NOTIFICATION_ENTER_TREE: {
			create_editors();
			update_joint_tree();
			update_editors();
			joint_tree->connect("item_selected", callable_mp(this, &EWBIKSkeleton3DEditor::_joint_tree_selection_changed));
			joint_tree->connect("item_rmb_selected", callable_mp(this, &EWBIKSkeleton3DEditor::_joint_tree_rmb_select));
#ifdef TOOLS_ENABLED
			skeleton->connect("pose_updated", callable_mp(this, &EWBIKSkeleton3DEditor::_draw_gizmo));
			skeleton->connect("pose_updated", callable_mp(this, &EWBIKSkeleton3DEditor::_update_properties));
			skeleton->connect("bone_enabled_changed", callable_mp(this, &EWBIKSkeleton3DEditor::_bone_enabled_changed));
			skeleton->connect("show_rest_only_changed", callable_mp(this, &EWBIKSkeleton3DEditor::_update_gizmo_visible));
#endif
			break;
		}
	}
}

void EWBIKSkeleton3DEditor::_node_removed(Node *p_node) {
	if (skeleton && p_node == skeleton) {
		skeleton = nullptr;
	}

	_update_properties();
}

void EWBIKSkeleton3DEditor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_node_removed"), &EWBIKSkeleton3DEditor::_node_removed);
	ClassDB::bind_method(D_METHOD("_joint_tree_selection_changed"), &EWBIKSkeleton3DEditor::_joint_tree_selection_changed);
	ClassDB::bind_method(D_METHOD("_joint_tree_rmb_select"), &EWBIKSkeleton3DEditor::_joint_tree_rmb_select);
	ClassDB::bind_method(D_METHOD("_update_properties"), &EWBIKSkeleton3DEditor::_update_properties);

	ClassDB::bind_method(D_METHOD("get_drag_data_fw"), &EWBIKSkeleton3DEditor::get_drag_data_fw);
	ClassDB::bind_method(D_METHOD("can_drop_data_fw"), &EWBIKSkeleton3DEditor::can_drop_data_fw);
	ClassDB::bind_method(D_METHOD("drop_data_fw"), &EWBIKSkeleton3DEditor::drop_data_fw);

	ClassDB::bind_method(D_METHOD("move_skeleton_bone"), &EWBIKSkeleton3DEditor::move_skeleton_bone);

	ClassDB::bind_method(D_METHOD("_draw_gizmo"), &EWBIKSkeleton3DEditor::_draw_gizmo);
}

void EWBIKSkeleton3DEditor::edit_mode_toggled(const bool pressed) {
	edit_mode = pressed;
	_update_gizmo_visible();
}

EWBIKSkeleton3DEditor::EWBIKSkeleton3DEditor(EWBIKEditorInspectorPluginSkeleton *e_plugin, Skeleton3D *p_skeleton) :
		editor_plugin(e_plugin),
		skeleton(p_skeleton) {
	singleton = this;

	// Handle.
	handle_material = Ref<ShaderMaterial>(memnew(ShaderMaterial));
	handle_shader = Ref<Shader>(memnew(Shader));
	handle_shader->set_code(R"(
// Skeleton 3D gizmo handle shader.

shader_type spatial;
render_mode unshaded, depth_test_disabled, cull_back;
uniform sampler2D texture_albedo : hint_albedo;
uniform float point_size : hint_range(0,128) = 32;
void vertex() {
	if (!OUTPUT_IS_SRGB) {
		COLOR.rgb = mix( pow((COLOR.rgb + vec3(0.055)) * (1.0 / (1.0 + 0.055)), vec3(2.4)), COLOR.rgb* (1.0 / 12.92), lessThan(COLOR.rgb,vec3(0.04045)) );
	}
	VERTEX = VERTEX;
	POSITION = PROJECTION_MATRIX * VIEW_MATRIX * MODEL_MATRIX * vec4(VERTEX.xyz, 1.0);
	POSITION.z = mix(POSITION.z, 0, 0.999);
	POINT_SIZE = point_size;
}
void fragment() {
	vec4 albedo_tex = texture(texture_albedo,POINT_COORD);
	vec3 col = albedo_tex.rgb + COLOR.rgb;
	col = vec3(min(col.r,1.0),min(col.g,1.0),min(col.b,1.0));
	ALBEDO = col;
	if (albedo_tex.a < 0.5) { discard; }
	ALPHA = albedo_tex.a;
}
)");
	handle_material->set_shader(handle_shader);
	Ref<Texture2D> handle = EditorNode::get_singleton()->get_gui_base()->get_theme_icon(SNAME("EditorBoneHandle"), SNAME("EditorIcons"));
	handle_material->set_shader_param("point_size", handle->get_width());
	handle_material->set_shader_param("texture_albedo", handle);

	handles_mesh_instance = memnew(MeshInstance3D);
	handles_mesh_instance->set_cast_shadows_setting(GeometryInstance3D::SHADOW_CASTING_SETTING_OFF);
	handles_mesh.instantiate();
	handles_mesh_instance->set_mesh(handles_mesh);
}

void EWBIKSkeleton3DEditor::update_bone_original() {
	if (!skeleton) {
		return;
	}
	if (skeleton->get_bone_count() == 0 || selected_bone == -1) {
		return;
	}
	bone_original_position = skeleton->get_bone_pose_position(selected_bone);
	bone_original_rotation = skeleton->get_bone_pose_rotation(selected_bone);
	bone_original_scale = skeleton->get_bone_pose_scale(selected_bone);
}

void EWBIKSkeleton3DEditor::_hide_handles() {
	handles_mesh_instance->hide();
}

void EWBIKSkeleton3DEditor::_draw_gizmo() {
	if (!skeleton) {
		return;
	}

	// If you call get_bone_global_pose() while drawing the surface, such as toggle rest mode,
	// the skeleton update will be done first and
	// the drawing surface will be interrupted once and an error will occur.
	skeleton->force_update_all_dirty_bones();

	// Handles.
	if (edit_mode) {
		_draw_handles();
	} else {
		_hide_handles();
	}
}

void EWBIKSkeleton3DEditor::_draw_handles() {
	handles_mesh_instance->show();

	const int bone_len = skeleton->get_bone_count();
	handles_mesh->clear_surfaces();
	handles_mesh->surface_begin(Mesh::PRIMITIVE_POINTS);

	for (int i = 0; i < bone_len; i++) {
		Color c;
		if (i == selected_bone) {
			c = Color(1, 1, 0);
		} else {
			c = Color(0.1, 0.25, 0.8);
		}
		Vector3 point = skeleton->get_bone_global_pose(i).origin;
		handles_mesh->surface_set_color(c);
		handles_mesh->surface_add_vertex(point);
	}
	handles_mesh->surface_end();
	handles_mesh->surface_set_material(0, handle_material);
}

TreeItem *EWBIKSkeleton3DEditor::_find(TreeItem *p_node, const NodePath &p_path) {
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

void EWBIKSkeleton3DEditor::_subgizmo_selection_change() {
	if (!skeleton) {
		return;
	}

	// Once validated by subgizmos_intersect_ray, but required if through inspector's bones tree.
	if (!edit_mode) {
		skeleton->clear_subgizmo_selection();
		return;
	}

	int selected = -1;
	EWBIKSkeleton3DEditor *se = EWBIKSkeleton3DEditor::get_singleton();
	if (se) {
		selected = se->get_selected_bone();
	}

	if (selected >= 0) {
		Vector<Ref<Node3DGizmo>> gizmos = skeleton->get_gizmos();
		for (int i = 0; i < gizmos.size(); i++) {
			Ref<EditorNode3DGizmo> gizmo = gizmos[i];
			if (!gizmo.is_valid()) {
				continue;
			}
			Ref<EWBIKSkeleton3DGizmoPlugin> plugin = gizmo->get_plugin();
			if (!plugin.is_valid()) {
				continue;
			}
			skeleton->set_subgizmo_selection(gizmo, selected, skeleton->get_bone_global_pose(selected));
			break;
		}
	} else {
		skeleton->clear_subgizmo_selection();
	}
}

void EWBIKSkeleton3DEditor::select_bone(int p_idx) {
	if (p_idx >= 0) {
		TreeItem *ti = _find(joint_tree->get_root(), "bones/" + itos(p_idx));
		if (ti) {
			// Make visible when it's collapsed.
			TreeItem *node = ti->get_parent();
			while (node && node != joint_tree->get_root()) {
				node->set_collapsed(false);
				node = node->get_parent();
			}
			ti->select(0);
			joint_tree->scroll_to_item(ti);
		}
	} else {
		selected_bone = -1;
		joint_tree->deselect_all();
		_joint_tree_selection_changed();
	}
}

EWBIKSkeleton3DEditor::~EWBIKSkeleton3DEditor() {
	if (skeleton) {
		select_bone(-1);
#ifdef TOOLS_ENABLED
		skeleton->disconnect("show_rest_only_changed", callable_mp(this, &EWBIKSkeleton3DEditor::_update_gizmo_visible));
		skeleton->disconnect("bone_enabled_changed", callable_mp(this, &EWBIKSkeleton3DEditor::_bone_enabled_changed));
		skeleton->disconnect("pose_updated", callable_mp(this, &EWBIKSkeleton3DEditor::_draw_gizmo));
		skeleton->disconnect("pose_updated", callable_mp(this, &EWBIKSkeleton3DEditor::_update_properties));
		skeleton->set_transform_gizmo_visible(true);
#endif
		handles_mesh_instance->get_parent()->remove_child(handles_mesh_instance);
	}
	edit_mode_toggled(false);

	handles_mesh_instance->queue_delete();

	Node3DEditor *ne = Node3DEditor::get_singleton();

	if (animation_hb) {
		ne->remove_control_from_menu_panel(animation_hb);
		memdelete(animation_hb);
	}

	if (separator) {
		ne->remove_control_from_menu_panel(separator);
		memdelete(separator);
	}

	if (edit_mode_button) {
		ne->remove_control_from_menu_panel(edit_mode_button);
		memdelete(edit_mode_button);
	}
}

bool EWBIKEditorInspectorPluginSkeleton::can_handle(Object *p_object) {
	return Object::cast_to<Skeleton3D>(p_object) != nullptr;
}

void EWBIKEditorInspectorPluginSkeleton::parse_begin(Object *p_object) {
	Skeleton3D *skeleton = Object::cast_to<Skeleton3D>(p_object);
	ERR_FAIL_COND(!skeleton);

	skel_editor = memnew(EWBIKSkeleton3DEditor(this, skeleton));
	add_custom_control(skel_editor);
}

EWBIKSkeleton3DEditorPlugin::EWBIKSkeleton3DEditorPlugin() {
	skeleton_plugin = memnew(EWBIKEditorInspectorPluginSkeleton);

	EditorInspector::add_inspector_plugin(skeleton_plugin);

	Ref<EWBIKSkeleton3DGizmoPlugin> gizmo_plugin = Ref<EWBIKSkeleton3DGizmoPlugin>(memnew(EWBIKSkeleton3DGizmoPlugin));
	Node3DEditor::get_singleton()->add_gizmo_plugin(gizmo_plugin);
}

EditorPlugin::AfterGUIInput EWBIKSkeleton3DEditorPlugin::forward_spatial_gui_input(Camera3D *p_camera, const Ref<InputEvent> &p_event) {
	EWBIKSkeleton3DEditor *se = EWBIKSkeleton3DEditor::get_singleton();
	Node3DEditor *ne = Node3DEditor::get_singleton();
	if (se->is_edit_mode()) {
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
		return EditorPlugin::AFTER_GUI_INPUT_DESELECT;
	}
	return EditorPlugin::AFTER_GUI_INPUT_PASS;
}

bool EWBIKSkeleton3DEditorPlugin::handles(Object *p_object) const {
	if (!p_object->is_class("Skeleton3D")) {
		return false;
	}
	Skeleton3D *skeleton = cast_to<Skeleton3D>(p_object);
	Ref<SkeletonModificationStack3D> stack = skeleton->get_modification_stack();
	if (stack.is_null()) {
		return false;
	}
	if (!stack->get_modification_count()) {
		return false;
	}
	for (int32_t count_i = 0; count_i < stack->get_modification_count(); count_i++) {
		Ref<SkeletonModification3D> mod = stack->get_modification(count_i);
		if (mod.is_null()) {
			continue;
		}
		if (!mod->is_class("SkeletonModification3DEWBIK")) {
			return false;
		}
		return true;
	}
	return false;
}

void EWBIKSkeleton3DEditor::_bone_enabled_changed(const int p_bone_id) {
	_update_gizmo_visible();
}

void EWBIKSkeleton3DEditor::_update_gizmo_visible() {
	_subgizmo_selection_change();
#ifdef TOOLS_ENABLED
	skeleton->set_transform_gizmo_visible(false);
#endif
	_draw_gizmo();
}

int EWBIKSkeleton3DEditor::get_selected_bone() const {
	return selected_bone;
}

EWBIKSkeleton3DGizmoPlugin::EWBIKSkeleton3DGizmoPlugin() {
	unselected_mat = Ref<StandardMaterial3D>(memnew(StandardMaterial3D));
	unselected_mat->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
	unselected_mat->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
	unselected_mat->set_flag(StandardMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
	unselected_mat->set_flag(StandardMaterial3D::FLAG_SRGB_VERTEX_COLOR, true);

	selected_mat = Ref<ShaderMaterial>(memnew(ShaderMaterial));
	selected_sh = Ref<Shader>(memnew(Shader));
	selected_sh->set_code(R"(
// Skeleton 3D gizmo bones shader.

shader_type spatial;
render_mode unshaded, shadows_disabled;
void vertex() {
	if (!OUTPUT_IS_SRGB) {
		COLOR.rgb = mix( pow((COLOR.rgb + vec3(0.055)) * (1.0 / (1.0 + 0.055)), vec3(2.4)), COLOR.rgb* (1.0 / 12.92), lessThan(COLOR.rgb,vec3(0.04045)) );
	}
	VERTEX = VERTEX;
	POSITION = PROJECTION_MATRIX * VIEW_MATRIX * MODEL_MATRIX * vec4(VERTEX.xyz, 1.0);
	POSITION.z = mix(POSITION.z, 0, 0.998);
}
void fragment() {
	ALBEDO = COLOR.rgb;
	ALPHA = COLOR.a;
}
)");
	selected_mat->set_shader(selected_sh);

	kusudama_material = Ref<ShaderMaterial>(memnew(ShaderMaterial));
	Ref<Shader> kusudama_shader = Ref<Shader>(memnew(Shader));
	kusudama_shader->set_code(R"(
// Skeleton 3D gizmo kusudama constraint shader.
shader_type spatial;
render_mode depth_draw_always, depth_prepass_alpha, cull_disabled;

uniform vec4 kusudamaColor : hint_color = vec4(0.58039218187332, 0.27058824896812, 0.00784313771874, 1.0);
const int CONE_COUNT_MAX = 30;


// 0,0,0 is the center of the kusudama. The kusudamas have their own bases that automatically get reoriented such that +y points in the direction that is the weighted average of the limitcones on the kusudama.
// But, if you have a kusuduma with just 1 limitcone, then in general that limitcone should be 0,1,0 in the kusudama's basis unless the user has specifically specified otherwise.

uniform vec4 coneSequence[30];

// This shader can display up to 30 cones (represented by 30 4d vectors) 
// Each group of 4 represents the xyz coordinates of the cone direction
// vector in model space and the fourth element represents radius

// TODO: Use a texture to store bone parameters.
// Use the uv to get the row of the bone.

varying vec3 normalModelDir;
varying vec4 vertModelColor;

bool isInInterConePath(in vec3 normalDir, in vec4 tangent1, in vec4 cone1, in vec4 tangent2, in vec4 cone2) {
	vec3 c1xc2 = cross(cone1.xyz, cone2.xyz);		
	float c1c2dir = dot(normalDir, c1xc2);
		
	if (c1c2dir < 0.0) { 
		vec3 c1xt1 = cross(cone1.xyz, tangent1.xyz); 
		vec3 t1xc2 = cross(tangent1.xyz, cone2.xyz);	
		float c1t1dir = dot(normalDir, c1xt1);
		float t1c2dir = dot(normalDir, t1xc2);
		
	 	return (c1t1dir > 0.0 && t1c2dir > 0.0); 
			
	} else {
		vec3 t2xc1 = cross(tangent2.xyz, cone1.xyz);	
		vec3 c2xt2 = cross(cone2.xyz, tangent2.xyz);	
		float t2c1dir = dot(normalDir, t2xc1);
		float c2t2dir = dot(normalDir, c2xt2);
		
		return (c2t2dir > 0.0 && t2c1dir > 0.0);
	}	
	return false;
}

//determines the current draw condition based on the desired draw condition in the setToArgument
// -3 = disallowed entirely; 
// -2 = disallowed and on tangentCone boundary
// -1 = disallowed and on controlCone boundary
// 0 =  allowed and empty; 
// 1 =  allowed and on controlCone boundary
// 2  = allowed and on tangentCone boundary
int getAllowabilityCondition(in int currentCondition, in int setTo) {
	if((currentCondition == -1 || currentCondition == -2)
		&& setTo >= 0) {
		return currentCondition *= -1;
	} else if(currentCondition == 0 && (setTo == -1 || setTo == -2)) {
		return setTo *=-2;
	}  	
	return max(currentCondition, setTo);
}

// returns 1 if normalDir is beyond (cone.a) radians from the cone.rgb
// returns 0 if normalDir is within (cone.a + boundaryWidth) radians from the cone.rgb
// return -1 if normalDir is less than (cone.a) radians from the cone.rgb
int isInCone(in vec3 normalDir, in vec4 cone, in float boundaryWidth) {
	float arcDistToCone = acos(dot(normalDir, cone.rgb));
	if (arcDistToCone > (cone.a+(boundaryWidth/2.))) {
		return 1; 
	}
	if (arcDistToCone < (cone.a-(boundaryWidth/2.))) {
		return -1;
	}
	return 0;
} 

// Returns a color corresponding to the allowability of this region,
// or otherwise the boundaries corresponding 
// to various cones and tangentCone.
vec4 colorAllowed(in vec3 normalDir,  in int coneCounts, in float boundaryWidth) {
	int currentCondition = -3;
	if (coneCounts == 1) {
		vec4 cone = coneSequence[0];
		int inCone = isInCone(normalDir, cone, boundaryWidth);
		bool isInCone = inCone == 0;
		if (isInCone) {
			inCone = -1;
		} else {
			if (inCone < 0) {
				inCone = 0;
			} else {
				inCone = -3;
			}
		}
		currentCondition = getAllowabilityCondition(currentCondition, inCone);
	} else {
		for(int i=0; i < coneCounts-1; i += 3) {
			normalDir = normalize(normalDir);
			int idx = i*3; 
			vec4 cone1 = coneSequence[idx];
			vec4 tangent1 = coneSequence[idx+1];
			vec4 tangent2 = coneSequence[idx+2];
			vec4 cone2 = coneSequence[idx+3];

			int inCone1 = isInCone(normalDir, cone1, boundaryWidth);
			if (inCone1 == 0) {
				inCone1 = -1;
			} else {
				if (inCone1 < 0) {
					inCone1 = 0;
				} else {
					inCone1 = -3;
				}
			}
			currentCondition = getAllowabilityCondition(currentCondition, inCone1);

			int inCone2 = isInCone(normalDir, cone2, boundaryWidth);
			if (inCone2 == 0) {
				inCone2 = -1;
			} else {
				if (inCone2 < 0) {
					inCone2 = 0;
				} else {
					inCone2 = -3;
				}
			}
			currentCondition = getAllowabilityCondition(currentCondition, inCone2);

			int inTan1 = isInCone(normalDir, tangent1, boundaryWidth); 
			int inTan2 = isInCone(normalDir, tangent2, boundaryWidth);
			
			if (float(inTan1) < 1. || float(inTan2) < 1.) {
				inTan1 = inTan1 == 0 ? -2 : -3;
				currentCondition = getAllowabilityCondition(currentCondition, inTan1);
				inTan2 = inTan2 == 0 ? -2 : -3;
				currentCondition = getAllowabilityCondition(currentCondition, inTan2);
			} else {				 
				bool inIntercone = isInInterConePath(normalDir, tangent1, cone1, tangent2, cone2);
				int interconeCondition = inIntercone ? 0 : -3;
				currentCondition = getAllowabilityCondition(currentCondition, interconeCondition);
			}
		}
	}
	vec4 result = vertModelColor;
	if (currentCondition != 0) {
		float onTanBoundary = abs(currentCondition) == 2 ? 0.3 : 0.0; 
		float onConeBoundary = abs(currentCondition) == 1 ? 0.3 : 0.0;
		result += vec4(0.0, onConeBoundary, onTanBoundary, 0.0);
	} else {
		return vec4(0.0, 0.0, 0.0, 0.0);
	}
	return result;
}

void vertex() {
	normalModelDir = NORMAL;
	vertModelColor.rgb = kusudamaColor.rgb;
}

void fragment() {
	vec4 resultColorAllowed = vec4(0.0, 0.0, 0.0, 0.0);
	if (coneSequence.length() == 30) {
		resultColorAllowed = colorAllowed(normalModelDir, CONE_COUNT_MAX, 0.02);
	}
	if (resultColorAllowed.a == 0.0) {
		discard;
	}
	ALBEDO = resultColorAllowed.rgb;
	ALPHA = resultColorAllowed.a;
}
)");
	kusudama_material->set_shader(kusudama_shader);
}

bool EWBIKSkeleton3DGizmoPlugin::has_gizmo(Node3D *p_spatial) {
	if (!Object::cast_to<Skeleton3D>(p_spatial)) {
		return false;
	}
	return true;
}

String EWBIKSkeleton3DGizmoPlugin::get_gizmo_name() const {
	return "Skeleton3D";
}

int EWBIKSkeleton3DGizmoPlugin::subgizmos_intersect_ray(const EditorNode3DGizmo *p_gizmo, Camera3D *p_camera, const Vector2 &p_point) const {
	Skeleton3D *skeleton = Object::cast_to<Skeleton3D>(p_gizmo->get_spatial_node());
	ERR_FAIL_COND_V(!skeleton, -1);

	EWBIKSkeleton3DEditor *se = EWBIKSkeleton3DEditor::get_singleton();

	if (!se->is_edit_mode()) {
		return -1;
	}

	if (Node3DEditor::get_singleton()->get_tool_mode() != Node3DEditor::TOOL_MODE_SELECT) {
		return -1;
	}

	// Select bone.
	real_t grab_threshold = 4 * EDSCALE;
	Vector3 ray_from = p_camera->get_global_transform().origin;
	Transform3D gt = skeleton->get_global_transform();
	int closest_idx = -1;
	real_t closest_dist = 1e10;
	const int bone_len = skeleton->get_bone_count();
	for (int i = 0; i < bone_len; i++) {
		Vector3 joint_pos_3d = gt.xform(skeleton->get_bone_global_pose(i).origin);
		Vector2 joint_pos_2d = p_camera->unproject_position(joint_pos_3d);
		real_t dist_3d = ray_from.distance_to(joint_pos_3d);
		real_t dist_2d = p_point.distance_to(joint_pos_2d);
		if (dist_2d < grab_threshold && dist_3d < closest_dist) {
			closest_dist = dist_3d;
			closest_idx = i;
		}
	}

	if (closest_idx >= 0) {
		WARN_PRINT("ray:");
		WARN_PRINT(itos(closest_idx));
		se->select_bone(closest_idx);
		return closest_idx;
	}

	se->select_bone(-1);
	return -1;
}

Transform3D EWBIKSkeleton3DGizmoPlugin::get_subgizmo_transform(const EditorNode3DGizmo *p_gizmo, int p_id) const {
	Skeleton3D *skeleton = Object::cast_to<Skeleton3D>(p_gizmo->get_spatial_node());
	ERR_FAIL_COND_V(!skeleton, Transform3D());

	return skeleton->get_bone_global_pose(p_id);
}

void EWBIKSkeleton3DGizmoPlugin::set_subgizmo_transform(const EditorNode3DGizmo *p_gizmo, int p_id, Transform3D p_transform) {
	Skeleton3D *skeleton = Object::cast_to<Skeleton3D>(p_gizmo->get_spatial_node());
	ERR_FAIL_COND(!skeleton);

	// Prepare for global to local.
	Transform3D original_to_local = Transform3D();
	int parent_idx = skeleton->get_bone_parent(p_id);
	if (parent_idx >= 0) {
		original_to_local = original_to_local * skeleton->get_bone_global_pose(parent_idx);
	}
	Basis to_local = original_to_local.get_basis().inverse();

	// Prepare transform.
	Transform3D t = Transform3D();

	// Basis.
	t.basis = to_local * p_transform.get_basis();

	// Origin.
	Vector3 orig = skeleton->get_bone_pose(p_id).origin;
	Vector3 sub = p_transform.origin - skeleton->get_bone_global_pose(p_id).origin;
	t.origin = orig + to_local.xform(sub);

	// Apply transform.
	skeleton->set_bone_pose_position(p_id, t.origin);
	skeleton->set_bone_pose_rotation(p_id, t.basis.get_rotation_quaternion());
	skeleton->set_bone_pose_scale(p_id, t.basis.get_scale());
}

void EWBIKSkeleton3DGizmoPlugin::commit_subgizmos(const EditorNode3DGizmo *p_gizmo, const Vector<int> &p_ids, const Vector<Transform3D> &p_restore, bool p_cancel) {
	Skeleton3D *skeleton = Object::cast_to<Skeleton3D>(p_gizmo->get_spatial_node());
	ERR_FAIL_COND(!skeleton);

	EWBIKSkeleton3DEditor *se = EWBIKSkeleton3DEditor::get_singleton();
	Node3DEditor *ne = Node3DEditor::get_singleton();

	UndoRedo *ur = EditorNode::get_singleton()->get_undo_redo();
	ur->create_action(TTR("Set Bone Transform"));
	if (ne->get_tool_mode() == Node3DEditor::TOOL_MODE_SELECT || ne->get_tool_mode() == Node3DEditor::TOOL_MODE_MOVE) {
		for (int i = 0; i < p_ids.size(); i++) {
			ur->add_do_method(skeleton, "set_bone_pose_position", p_ids[i], skeleton->get_bone_pose_position(p_ids[i]));
			ur->add_undo_method(skeleton, "set_bone_pose_position", p_ids[i], se->get_bone_original_position());
		}
	}
	if (ne->get_tool_mode() == Node3DEditor::TOOL_MODE_SELECT || ne->get_tool_mode() == Node3DEditor::TOOL_MODE_ROTATE) {
		for (int i = 0; i < p_ids.size(); i++) {
			ur->add_do_method(skeleton, "set_bone_pose_rotation", p_ids[i], skeleton->get_bone_pose_rotation(p_ids[i]));
			ur->add_undo_method(skeleton, "set_bone_pose_rotation", p_ids[i], se->get_bone_original_rotation());
		}
	}
	if (ne->get_tool_mode() == Node3DEditor::TOOL_MODE_SCALE) {
		for (int i = 0; i < p_ids.size(); i++) {
			// If the axis is swapped by scaling, the rotation can be changed.
			ur->add_do_method(skeleton, "set_bone_pose_rotation", p_ids[i], skeleton->get_bone_pose_rotation(p_ids[i]));
			ur->add_undo_method(skeleton, "set_bone_pose_rotation", p_ids[i], se->get_bone_original_rotation());
			ur->add_do_method(skeleton, "set_bone_pose_scale", p_ids[i], skeleton->get_bone_pose_scale(p_ids[i]));
			ur->add_undo_method(skeleton, "set_bone_pose_scale", p_ids[i], se->get_bone_original_scale());
		}
	}
	ur->commit_action();
}

void EWBIKSkeleton3DGizmoPlugin::redraw(EditorNode3DGizmo *p_gizmo) {
	Skeleton3D *skeleton = Object::cast_to<Skeleton3D>(p_gizmo->get_spatial_node());
	p_gizmo->clear();

	int selected = -1;
	EWBIKSkeleton3DEditor *se = EWBIKSkeleton3DEditor::get_singleton();
	if (se) {
		selected = se->get_selected_bone();
	}

	Color bone_color = EditorSettings::get_singleton()->get("editors/3d_gizmos/gizmo_colors/skeleton");
	Color selected_bone_color = EditorSettings::get_singleton()->get("editors/3d_gizmos/gizmo_colors/selected_bone");
	real_t bone_axis_length = EditorSettings::get_singleton()->get("editors/3d_gizmos/gizmo_settings/bone_axis_length");
	int bone_shape = EditorSettings::get_singleton()->get("editors/3d_gizmos/gizmo_settings/bone_shape");

	LocalVector<Color> axis_colors;
	axis_colors.push_back(Node3DEditor::get_singleton()->get_theme_color(SNAME("axis_x_color"), SNAME("Editor")));
	axis_colors.push_back(Node3DEditor::get_singleton()->get_theme_color(SNAME("axis_y_color"), SNAME("Editor")));
	axis_colors.push_back(Node3DEditor::get_singleton()->get_theme_color(SNAME("axis_z_color"), SNAME("Editor")));

	Ref<SurfaceTool> surface_tool(memnew(SurfaceTool));
	surface_tool->begin(Mesh::PRIMITIVE_LINES);

	if (p_gizmo->is_selected()) {
		surface_tool->set_material(selected_mat);
	} else {
		unselected_mat->set_albedo(bone_color);
		surface_tool->set_material(unselected_mat);
	}

	LocalVector<int> bones;
	LocalVector<float> weights;
	bones.resize(4);
	weights.resize(4);
	for (int i = 0; i < 4; i++) {
		bones[i] = 0;
		weights[i] = 0;
	}
	weights[0] = 1;

	int current_bone_index = 0;
	Vector<int> bones_to_process = skeleton->get_parentless_bones();

	while (bones_to_process.size() > current_bone_index) {
		int current_bone_idx = bones_to_process[current_bone_index];
		current_bone_index++;

		Color current_bone_color = (current_bone_idx == selected) ? selected_bone_color : bone_color;

		Vector<int> child_bones_vector;
		child_bones_vector = skeleton->get_bone_children(current_bone_idx);
		int child_bones_size = child_bones_vector.size();

		for (int i = 0; i < child_bones_size; i++) {
			// Something wrong.
			if (child_bones_vector[i] < 0) {
				continue;
			}

			int child_bone_idx = child_bones_vector[i];

			Vector3 v0 = skeleton->get_bone_global_rest(current_bone_idx).origin;
			Vector3 v1 = skeleton->get_bone_global_rest(child_bone_idx).origin;
			Vector3 d = (v1 - v0).normalized();
			real_t dist = v0.distance_to(v1);

			// Find closest axis.
			int closest = -1;
			real_t closest_d = 0.0;
			for (int j = 0; j < 3; j++) {
				real_t dp = Math::abs(skeleton->get_bone_global_rest(current_bone_idx).basis[j].normalized().dot(d));
				if (j == 0 || dp > closest_d) {
					closest = j;
				}
			}
			// Draw bone.
			switch (bone_shape) {
				case 0: { // Wire shape.
					surface_tool->set_color(current_bone_color);
					bones[0] = current_bone_idx;
					surface_tool->set_bones(bones);
					surface_tool->set_weights(weights);
					surface_tool->add_vertex(v0);
					bones[0] = child_bone_idx;
					surface_tool->set_bones(bones);
					surface_tool->set_weights(weights);
					surface_tool->add_vertex(v1);
				} break;

				case 1: { // Octahedron shape.
					Vector3 first;
					Vector3 points[6];
					int point_idx = 0;
					for (int j = 0; j < 3; j++) {
						Vector3 axis;
						if (first == Vector3()) {
							axis = d.cross(d.cross(skeleton->get_bone_global_rest(current_bone_idx).basis[j])).normalized();
							first = axis;
						} else {
							axis = d.cross(first).normalized();
						}

						surface_tool->set_color(current_bone_color);
						for (int k = 0; k < 2; k++) {
							if (k == 1) {
								axis = -axis;
							}
							Vector3 point = v0 + d * dist * 0.2;
							point += axis * dist * 0.1;

							bones[0] = current_bone_idx;
							surface_tool->set_bones(bones);
							surface_tool->set_weights(weights);
							surface_tool->add_vertex(v0);
							surface_tool->set_bones(bones);
							surface_tool->set_weights(weights);
							surface_tool->add_vertex(point);

							surface_tool->set_bones(bones);
							surface_tool->set_weights(weights);
							surface_tool->add_vertex(point);
							bones[0] = child_bone_idx;
							surface_tool->set_bones(bones);
							surface_tool->set_weights(weights);
							surface_tool->add_vertex(v1);
							points[point_idx++] = point;
						}
					}
					surface_tool->set_color(current_bone_color);
					SWAP(points[1], points[2]);
					bones[0] = current_bone_idx;
					for (int j = 0; j < 6; j++) {
						surface_tool->set_bones(bones);
						surface_tool->set_weights(weights);
						surface_tool->add_vertex(points[j]);
						surface_tool->set_bones(bones);
						surface_tool->set_weights(weights);
						surface_tool->add_vertex(points[(j + 1) % 6]);
					}
				} break;
			}

			// Axis as root of the bone.
			for (int j = 0; j < 3; j++) {
				bones[0] = current_bone_idx;
				surface_tool->set_color(axis_colors[j]);
				surface_tool->set_bones(bones);
				surface_tool->set_weights(weights);
				surface_tool->add_vertex(v0);
				surface_tool->set_bones(bones);
				surface_tool->set_weights(weights);
				surface_tool->add_vertex(v0 + (skeleton->get_bone_global_rest(current_bone_idx).basis.inverse())[j].normalized() * dist * bone_axis_length);

				if (j == closest) {
					continue;
				}
			}

			// Axis at the end of the bone children.
			if (i == child_bones_size - 1) {
				for (int j = 0; j < 3; j++) {
					bones[0] = child_bone_idx;
					surface_tool->set_color(axis_colors[j]);
					surface_tool->set_bones(bones);
					surface_tool->set_weights(weights);
					surface_tool->add_vertex(v1);
					surface_tool->set_bones(bones);
					surface_tool->set_weights(weights);
					surface_tool->add_vertex(v1 + (skeleton->get_bone_global_rest(child_bone_idx).basis.inverse())[j].normalized() * dist * bone_axis_length);

					if (j == closest) {
						continue;
					}
				}
			}

			Ref<SphereMesh> sphere_mesh;
			sphere_mesh.instantiate();
			sphere_mesh->set_radius(dist / 8.0f);
			sphere_mesh->set_height(dist / 4.0f);
			PackedFloat32Array kusudama_limit_cones;
			constexpr int32_t KUSUDAMA_MAX_CONES = 30;
			kusudama_limit_cones.resize(KUSUDAMA_MAX_CONES * 4);
			kusudama_limit_cones.fill(0.0f);
			Vector3 origin = Vector3(0.0f, 1.0f, 0.0f);
			kusudama_limit_cones.write[0] = origin.x;
			kusudama_limit_cones.write[1] = origin.y;
			kusudama_limit_cones.write[2] = origin.z;
			kusudama_limit_cones.write[3] = 1.f;
			kusudama_material->set_shader_param("coneSequence", kusudama_limit_cones);
			kusudama_material->set_shader_param("kusudamaColor", current_bone_color);

			Ref<SurfaceTool> kusudama_surface_tool;
			kusudama_surface_tool.instantiate();
			kusudama_surface_tool->begin(Mesh::PRIMITIVE_TRIANGLES);
			kusudama_surface_tool->create_from(sphere_mesh, 0);
			Array kusudama_array = kusudama_surface_tool->commit_to_arrays();
			kusudama_surface_tool->clear();
			kusudama_surface_tool->begin(Mesh::PRIMITIVE_TRIANGLES);
			Vector<Vector3> vertex_array = kusudama_array[Mesh::ARRAY_VERTEX];
			PackedFloat32Array index_array = kusudama_array[Mesh::ARRAY_INDEX];
			PackedVector2Array uv_array = kusudama_array[Mesh::ARRAY_TEX_UV];
			PackedVector3Array normal_array = kusudama_array[Mesh::ARRAY_NORMAL];
			PackedFloat32Array tangent_array = kusudama_array[Mesh::ARRAY_TANGENT];
			for (int32_t vertex_i = 0; vertex_i < vertex_array.size(); vertex_i++) {
				Vector3 sphere_vertex = vertex_array[vertex_i];
				kusudama_surface_tool->set_color(current_bone_color);
				kusudama_surface_tool->set_bones(bones);
				kusudama_surface_tool->set_weights(weights);
				Vector2 uv_vertex = uv_array[vertex_i];
				kusudama_surface_tool->set_uv(uv_vertex);
				Vector3 normal_vertex = normal_array[vertex_i];
				kusudama_surface_tool->set_normal(normal_vertex);
				Plane tangent_vertex;
				tangent_vertex.normal.x = tangent_array[vertex_i + 0];
				tangent_vertex.normal.y = tangent_array[vertex_i + 1];
				tangent_vertex.normal.z = tangent_array[vertex_i + 2];
				tangent_vertex.d = tangent_array[vertex_i + 3];
				kusudama_surface_tool->set_tangent(tangent_vertex);
				kusudama_surface_tool->add_vertex(skeleton->get_bone_global_rest(child_bone_idx).xform(sphere_vertex));
			}
			for (int32_t index_i = 0; index_i < index_array.size(); index_i++) {
				int32_t index = index_array[index_i];
				kusudama_surface_tool->add_index(index);
			}
			p_gizmo->add_mesh(kusudama_surface_tool->commit(), kusudama_material, Transform3D(), skeleton->register_skin(skeleton->create_skin_from_rest_transforms()));

			// Add the bone's children to the list of bones to be processed.
			bones_to_process.push_back(child_bones_vector[i]);
		}
	}

	// Ref<ArrayMesh> m = surface_tool->commit();
	// p_gizmo->add_mesh(m, Ref<Material>(), Transform3D(), skeleton->register_skin(skeleton->create_skin_from_rest_transforms()));
}