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
	joint_tree->clear();
	Skeleton3D *skeleton = ik->get_skeleton();
	if (!skeleton) {
		return;
	}
	TreeItem *root = joint_tree->create_item();
	HashMap<int, TreeItem *> items;
	items.insert(-1, root);
	Ref<Texture> bone_icon = get_theme_icon(SNAME("BoneAttachment3D"), SNAME("EditorIcons"));
	Vector<Ref<IKBoneSegment>> bone_segments = ik->get_segmented_skeletons();
	for (Ref<IKBoneSegment> bone_segment : bone_segments) {
		if (bone_segment.is_null()) {
			continue;
		}
		Vector<Ref<IKBone3D>> bone_list = ik->get_bone_list();
		bone_list.reverse();
		for (Ref<IKBone3D> bone : bone_list) {
			int current_bone_idx = bone->get_bone_id();
			Ref<IKBone3D> parent = bone->get_parent();
			int parent_idx = -1;
			if (parent.is_valid()) {
				parent_idx = parent->get_bone_id();
			}
			TreeItem *parent_item = items.find(parent_idx)->value;
			TreeItem *joint_item = joint_tree->create_item(parent_item);
			items.insert(current_bone_idx, joint_item);
			joint_item->set_text(0, skeleton->get_bone_name(current_bone_idx));
			joint_item->set_icon(0, bone_icon);
			joint_item->set_selectable(0, true);
			joint_item->set_metadata(0, "bones/" + itos(current_bone_idx));
		}
	}
}

void ManyBoneIK3DEditor::create_editors() {
	set_h_size_flags(SIZE_EXPAND_FILL);
	set_focus_mode(FOCUS_ALL);

	// Bone tree.
	const Color section_color = get_theme_color(SNAME("prop_subsection"), SNAME("Editor"));

	EditorInspectorSection *bones_section = memnew(EditorInspectorSection);
	ERR_FAIL_NULL(ik->get_skeleton());
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
