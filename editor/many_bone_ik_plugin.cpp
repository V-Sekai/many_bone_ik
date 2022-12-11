#include "editor/plugins/node_3d_editor_plugin.h"
#include "scene/gui/tree.h"

#include "many_bone_ik_plugin.h"

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

ManyBoneIK3DEditorPlugin::ManyBoneIK3DEditorPlugin() {
	skeleton_plugin = memnew(EditorInspectorPluginManyBoneIK);

	EditorInspector::add_inspector_plugin(skeleton_plugin);

	Ref<Skeleton3DGizmoPlugin> gizmo_plugin = Ref<Skeleton3DGizmoPlugin>(memnew(Skeleton3DGizmoPlugin));
	Node3DEditor::get_singleton()->add_gizmo_plugin(gizmo_plugin);
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
			if (joint_tree) {
				update_joint_tree();
				joint_tree->connect("item_selected", callable_mp(this, &ManyBoneIK3DEditor::_joint_tree_selection_changed));
			}
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			update_joint_tree();
		} break;
	}
}

inline ManyBoneIK3DEditor::ManyBoneIK3DEditor(EditorInspectorPluginManyBoneIK *e_plugin, ManyBoneIK3D *p_ik) {
	ik = p_ik;
	create_editors();
}

void ManyBoneIK3DEditor::_update_properties() {
	Node3DEditor::get_singleton()->update_transform_gizmo();
}

void ManyBoneIK3DEditor::update_joint_tree() {
	if (!ik || !ik->get_skeleton() || !joint_tree) {
		return;
	}
	joint_tree->clear();

	Skeleton3D *skeleton = ik->get_skeleton();

	TreeItem *root = joint_tree->create_item();

	HashMap<int, TreeItem *> items;

	items.insert(-1, root);

	Ref<Texture> bone_icon = get_theme_icon(SNAME("BoneAttachment3D"), SNAME("EditorIcons"));

	Vector<int> bones_to_process = skeleton->get_parentless_bones();
	while (bones_to_process.size() > 0) {
		int current_bone_idx = bones_to_process[0];
		bones_to_process.erase(current_bone_idx);

		const int parent_idx = skeleton->get_bone_parent(current_bone_idx);
		TreeItem *parent_item = items.find(parent_idx)->value;

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
	joint_tree->set_drag_forwarding(this);
	s_con->add_child(joint_tree);
	constraint_bone_section = memnew(EditorInspectorSection);
	constraint_bone_section->setup("constraint_bone_properties", TTR("Constraint"), this, section_color, true);
	constraint_bone_section->unfold();
	add_child(constraint_bone_section);

	pin_checkbox = memnew(EditorPropertyCheck());
	pin_checkbox->hide();
	pin_checkbox->set_label(TTR("Pin Enabled"));
	pin_checkbox->set_selectable(false);
	pin_checkbox->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
	constraint_bone_section->get_vbox()->add_child(pin_checkbox);

	target_nodepath = memnew(EditorPropertyNodePath());
	target_nodepath->hide();
	target_nodepath->set_label(TTR("Target NodePath"));
	target_nodepath->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
	constraint_bone_section->get_vbox()->add_child(target_nodepath);

	twist_from_float = memnew(EditorPropertyFloat());
	twist_from_float->hide();
	twist_from_float->setup(0, 360, 0.01, false, false, false, false, "", true);
	twist_from_float->set_label(TTR("Twist From"));
	twist_from_float->set_selectable(false);
	twist_from_float->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
	constraint_bone_section->get_vbox()->add_child(twist_from_float);

	twist_range_float = memnew(EditorPropertyFloat());
	twist_range_float->hide();
	twist_range_float->setup(-360, 360, 0.01, false, false, false, false, "", true);
	twist_range_float->set_label(TTR("Twist Range"));
	twist_range_float->set_selectable(false);
	twist_range_float->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
	constraint_bone_section->get_vbox()->add_child(twist_range_float);

	twist_current_float = memnew(EditorPropertyFloat());
	twist_current_float->hide();
	twist_current_float->setup(0, 360, 0.01, false, false, false, false, "", true);
	twist_current_float->set_label(TTR("Twist Current"));
	twist_current_float->set_selectable(false);
	twist_current_float->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
	constraint_bone_section->get_vbox()->add_child(twist_current_float);

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
		center_vector3[cone_i]->setup(0, 1, 0.01, false, false, "", false);
		center_vector3[cone_i]->set_label(TTR(vformat("Cone Center Point %d", cone_i + 1)));
		center_vector3[cone_i]->set_selectable(false);
		center_vector3[cone_i]->connect("property_changed", callable_mp(this, &ManyBoneIK3DEditor::_value_changed));
		constraint_bone_section->get_vbox()->add_child(center_vector3[cone_i]);

		radius_float[cone_i] = memnew(EditorPropertyFloat());
		radius_float[cone_i]->hide();
		radius_float[cone_i]->setup(0, 180, 0.01, false, false, false, false, "", true);
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
	orientation_constraint_transform->set_label(TTR("Twist Constraint"));
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
	if (selected) {
		const String path = selected->get_metadata(0);
		if (!path.begins_with("bones/")) {
			return;
		}
		const int b_idx = path.get_slicec('/', 1).to_int();
		selected_bone = b_idx;
	}
	select_bone(selected_bone);
	_update_properties();
}

void ManyBoneIK3DEditor::select_bone(int p_idx) {
	if (p_idx < 0) {
		selected_bone = -1;
		joint_tree->deselect_all();
		_joint_tree_selection_changed();
		return;
	}
	TreeItem *ti = _find(joint_tree->get_root(), "bones/" + itos(p_idx));
	if (!ti) {
		return;
	}
	// Make visible when it's collapsed.
	TreeItem *node = ti->get_parent();
	while (node && node != joint_tree->get_root()) {
		node->set_collapsed(false);
		node = node->get_parent();
	}
	ti->select(0);
	joint_tree->scroll_to_item(ti);

	Skeleton3D *skeleton = ik->get_skeleton();
	if (!skeleton) {
		return;
	}
	String bone_name = ik->get_skeleton()->get_bone_name(selected_bone);
	if (bone_name.is_empty()) {
		return;
	}
	int32_t pin_i = ik->find_effector_id(bone_name);
	if (pin_i == -1 || ik->get_pin_bone_name(pin_i) == StringName()) {
		return;
	}
	pin_checkbox->set_object_and_property(ik, vformat("pins/%d/enabled", pin_i));
	pin_checkbox->update_property();
	pin_checkbox->show();
	target_nodepath->set_object_and_property(ik, vformat("pins/%d/target_node", pin_i));
	target_nodepath->update_property();
	target_nodepath->show();

	int32_t constraint_i = ik->find_constraint(bone_name);
	if (constraint_i == -1) {
		return;
	}
	twist_from_float->set_object_and_property(ik, vformat("constraints/%d/twist_from", constraint_i));
	twist_from_float->update_property();
	twist_from_float->show();
	twist_range_float->set_object_and_property(ik, vformat("constraints/%d/twist_range", constraint_i));
	twist_range_float->update_property();
	twist_range_float->show();
	twist_current_float->set_object_and_property(ik, vformat("constraints/%d/twist_current", constraint_i));
	twist_current_float->update_property();
	twist_current_float->show();
	cone_count_float->set_object_and_property(ik, vformat("constraints/%d/kusudama_limit_cone_count", constraint_i));
	cone_count_float->update_property();
	for (int32_t cone_i = 0; cone_i < MAX_KUSUDAMA_CONES; cone_i++) {
		center_vector3[cone_i]->hide();
		radius_float[cone_i]->hide();
	}
	for (int32_t cone_i = 0; cone_i < ik->get_kusudama_limit_cone_count(constraint_i); cone_i++) {
		center_vector3[cone_i]->show();
		center_vector3[cone_i]->set_object_and_property(ik, vformat("constraints/%d/kusudama_limit_cone/%d/center", constraint_i, cone_i));
		center_vector3[cone_i]->update_property();
		radius_float[cone_i]->show();
		radius_float[cone_i]->set_object_and_property(ik, vformat("constraints/%d/kusudama_limit_cone/%d/radius", constraint_i, cone_i));
		radius_float[cone_i]->update_property();
	}
	twist_constraint_transform->set_object_and_property(ik, vformat("constraints/%d/bone_direction", constraint_i));
	twist_constraint_transform->update_property();
	twist_constraint_transform->show();
	orientation_constraint_transform->set_object_and_property(ik, vformat("constraints/%d/kusudama_orientation", constraint_i));
	orientation_constraint_transform->update_property();
	orientation_constraint_transform->show();
	bone_direction_transform->set_object_and_property(ik, vformat("constraints/%d/kusudama_twist", constraint_i));
	bone_direction_transform->update_property();
	bone_direction_transform->show();
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
	if (!is_visible()) {
		return;
	}
	if (ik) {
		Ref<EditorUndoRedoManager> &undo_redo = EditorNode::get_undo_redo();
		undo_redo->create_action(TTR("Set ManyBoneIK Property"), UndoRedo::MERGE_ENDS);
		undo_redo->add_undo_property(ik, p_property, ik->get(p_property));
		undo_redo->add_do_property(ik, p_property, p_value);
		undo_redo->commit_action();
	}
}
