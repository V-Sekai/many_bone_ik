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
#include "scene/gui/tree.h"

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

void ManyBoneIK3DEditor::_update_properties() {
	Node3DEditor::get_singleton()->update_transform_gizmo();
}

void ManyBoneIK3DEditor::update_joint_tree() {
	if (!ik || !ik->get_skeleton() || !joint_tree) {
		return;
	}
	joint_tree->clear();

	Skeleton3D *skeleton = ik->get_skeleton();
	if (!skeleton) {
		return;
	}

	TreeItem *root = joint_tree->create_item();

	HashMap<int, TreeItem *> items;

	items.insert(-1, root);

	Ref<Texture> bone_icon = get_theme_icon(SNAME("BoneAttachment3D"), SNAME("EditorIcons"));
	TypedArray<StringName> filter_bones = ik->get_filter_bones();

	Vector<int> bones_to_process = skeleton->get_parentless_bones();
	while (bones_to_process.size() > 0) {
		int current_bone_idx = bones_to_process[0];
		bones_to_process.erase(current_bone_idx);
		StringName bone_name = skeleton->get_bone_name(current_bone_idx);
		if (filter_bones.has(bone_name)) {
			continue;
		}
		const int parent_idx = skeleton->get_bone_parent(current_bone_idx);
		if (!items.find(parent_idx)) {
			continue;
		}
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
	ik->set_ui_selected_bone(selected_bone);
	ik->notify_property_list_changed();
	selected->select(0);
}

void ManyBoneIK3DEditor::select_bone(int p_idx) {
	if (p_idx < 0) {
		selected_bone = -1;
		return;
	}
	selected_bone = p_idx;
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
	if (!ik) {
		return;
	}
	Ref<EditorUndoRedoManager> &undo_redo = EditorNode::get_undo_redo();
	undo_redo->create_action(TTR("Set ManyBoneIK Property"), UndoRedo::MERGE_ENDS);
	undo_redo->add_undo_property(ik, p_property, ik->get(p_property));
	undo_redo->add_do_property(ik, p_property, p_value);
	undo_redo->commit_action();
}
