/*************************************************************************/
/*  skeleton_3d_editor_plugin.h                                          */
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

#ifndef EWBIK_SKELETON_3D_EDITOR_PLUGIN_H
#define EWBIK_SKELETON_3D_EDITOR_PLUGIN_H

#include "editor/editor_node.h"
#include "editor/editor_properties.h"
#include "editor/plugins/node_3d_editor_plugin.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/label_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/skeleton_3d.h"
#include "scene/resources/immediate_mesh.h"

class Joint;
class PhysicalBone3D;
class EWBIKSkeleton3DEditorPlugin;
class Button;

class EWBIKSkeleton3DEditor : public VBoxContainer {
	GDCLASS(EWBIKSkeleton3DEditor, VBoxContainer);

	friend class EWBIKSkeleton3DEditorPlugin;

	struct BoneInfo {
		PhysicalBone3D *physical_bone = nullptr;
		Transform3D relative_rest; // Relative to skeleton node.
	};
	Skeleton3D *skeleton = nullptr;
	static EWBIKSkeleton3DEditor *singleton;

	void _file_selected(const String &p_file);

	EditorFileDialog *file_export_lib = nullptr;

	Vector3 bone_original_position;
	Quaternion bone_original_rotation;
	Vector3 bone_original_scale;

	void _subgizmo_selection_change();

protected:
	void _notification(int p_what);
	void _node_removed(Node *p_node);
	static void _bind_methods();

public:
	static EWBIKSkeleton3DEditor *get_singleton() { return singleton; }
	Skeleton3D *get_skeleton() const { return skeleton; };

	~EWBIKSkeleton3DEditor();
};

class EWBIKSkeleton3DEditorPlugin : public EditorPlugin {
	GDCLASS(EWBIKSkeleton3DEditorPlugin, EditorPlugin);

public:
	virtual EditorPlugin::AfterGUIInput forward_spatial_gui_input(Camera3D *p_camera, const Ref<InputEvent> &p_event) override;

	bool has_main_screen() const override { return false; }
	virtual bool handles(Object *p_object) const override;
	virtual String get_name() const override { return "Skeleton3D"; }

	EWBIKSkeleton3DEditorPlugin();
};

class EWBIKSkeleton3DGizmoPlugin : public EditorNode3DGizmoPlugin {
	GDCLASS(EWBIKSkeleton3DGizmoPlugin, EditorNode3DGizmoPlugin);

public:
	const int32_t KUSUDAMA_MAX_CONES = 30;
	bool has_gizmo(Node3D *p_spatial) override;
	String get_gizmo_name() const override;

	int subgizmos_intersect_ray(const EditorNode3DGizmo *p_gizmo, Camera3D *p_camera, const Vector2 &p_point) const override;
	Transform3D get_subgizmo_transform(const EditorNode3DGizmo *p_gizmo, int p_id) const override;
	void set_subgizmo_transform(const EditorNode3DGizmo *p_gizmo, int p_id, Transform3D p_transform) override;

	void redraw(EditorNode3DGizmo *p_gizmo) override;

	int get_priority() const override {
		return -2;
	}
	EWBIKSkeleton3DGizmoPlugin();
};

#endif // SKELETON_3D_EDITOR_PLUGIN_H
