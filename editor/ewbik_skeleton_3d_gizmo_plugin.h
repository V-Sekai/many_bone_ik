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

#include "core/templates/hash_map.h"
#include "core/templates/local_vector.h"
#include "editor/editor_node.h"
#include "editor/editor_properties.h"
#include "editor/plugins/node_3d_editor_plugin.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/label_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/node_3d.h"
#include "scene/3d/skeleton_3d.h"
#include "scene/resources/immediate_mesh.h"

#include "../src/ewbik.h"

class Joint;
class PhysicalBone3D;
class EWBIKSkeleton3DEditorPlugin;
class Button;

class EWBIK3DGizmoPlugin : public EditorNode3DGizmoPlugin {
	GDCLASS(EWBIK3DGizmoPlugin, EditorNode3DGizmoPlugin);

	SkeletonModification3DEWBIK *ewbik = nullptr;

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("_get_gizmo_name"), &EWBIK3DGizmoPlugin::get_gizmo_name);
	}

public:
	const int32_t KUSUDAMA_MAX_CONES = 30;
	bool has_gizmo(Node3D *p_spatial) override;
	String get_gizmo_name() const override;
	void redraw(EditorNode3DGizmo *p_gizmo) override;
	int get_priority() const override {
		return -2;
	}
	EWBIK3DGizmoPlugin() {
		// Enable vertex colors for the materials below as the gizmo color depends on the light color.
		create_material("lines_primary", Color(1, 1, 1), false, false, true);
		create_material("lines_secondary", Color(1, 1, 1, 0.35), false, false, true);
		create_material("lines_billboard", Color(1, 1, 1), true, false, true);

		create_handle_material("handles");
		create_handle_material("handles_billboard", true);
	}
};

class EditorPluginEWBIK : public EditorPlugin {
	GDCLASS(EditorPluginEWBIK, EditorPlugin);

public:
	EditorPluginEWBIK() {
		Ref<EWBIK3DGizmoPlugin> ewbik_gizmo_plugin = Ref<EWBIK3DGizmoPlugin>(memnew(EWBIK3DGizmoPlugin));
		Node3DEditor::get_singleton()->add_gizmo_plugin(ewbik_gizmo_plugin);
	}
};

#endif // SKELETON_3D_EDITOR_PLUGIN_H
