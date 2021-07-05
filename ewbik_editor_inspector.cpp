#include "ewbik_editor_inspector.h"
#include "editor/editor_node.h"

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
	String label;
    EditorInspectorSection *section = memnew(EditorInspectorSection);
	section->setup("ewbik_properties", label, this, section_color, true);
	add_child(section);
}
