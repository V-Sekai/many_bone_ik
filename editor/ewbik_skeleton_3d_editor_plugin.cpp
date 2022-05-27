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

#include "../kusudama.h"
#include "../skeleton_modification_3d_ewbik.h"
#include "core/io/resource_saver.h"
#include "editor/editor_file_dialog.h"
#include "editor/editor_node.h"
#include "editor/editor_properties.h"
#include "editor/editor_scale.h"
#include "editor/plugins/animation_player_editor_plugin.h"
#include "editor/plugins/node_3d_editor_plugin.h"
#include "scene/3d/collision_shape_3d.h"
#include "scene/3d/joint_3d.h"
#include "scene/3d/label_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/physics_body_3d.h"
#include "scene/resources/capsule_shape_3d.h"
#include "scene/resources/primitive_meshes.h"
#include "scene/resources/skeleton_modification_3d.h"
#include "scene/resources/sphere_shape_3d.h"
#include "scene/resources/surface_tool.h"

EWBIKSkeleton3DEditor *EWBIKSkeleton3DEditor::singleton = nullptr;

void EWBIKSkeleton3DEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			get_tree()->connect("node_removed", callable_mp(this, &EWBIKSkeleton3DEditor::_node_removed), Vector<Variant>(), Object::CONNECT_ONESHOT);
			break;
		}
	}
}

void EWBIKSkeleton3DEditor::_node_removed(Node *p_node) {
	if (skeleton && p_node == skeleton) {
		skeleton = nullptr;
	}
}

void EWBIKSkeleton3DEditor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_node_removed"), &EWBIKSkeleton3DEditor::_node_removed);
}

void EWBIKSkeleton3DEditor::_subgizmo_selection_change() {
	if (!skeleton) {
		return;
	}
	skeleton->clear_subgizmo_selection();
}

EWBIKSkeleton3DEditor::~EWBIKSkeleton3DEditor() {
}

EWBIKSkeleton3DEditorPlugin::EWBIKSkeleton3DEditorPlugin() {
	Ref<EWBIKSkeleton3DGizmoPlugin> gizmo_plugin = Ref<EWBIKSkeleton3DGizmoPlugin>(memnew(EWBIKSkeleton3DGizmoPlugin));
	Node3DEditor::get_singleton()->add_gizmo_plugin(gizmo_plugin);
}

EditorPlugin::AfterGUIInput EWBIKSkeleton3DEditorPlugin::forward_spatial_gui_input(Camera3D *p_camera, const Ref<InputEvent> &p_event) {
	return EditorPlugin::AFTER_GUI_INPUT_PASS;
}

bool EWBIKSkeleton3DEditorPlugin::handles(Object *p_object) const {
	Skeleton3D *skeleton = cast_to<Skeleton3D>(p_object);
	Ref<SkeletonModificationStack3D> stack;
	if (skeleton) {
		stack = skeleton->get_modification_stack();
	}
	if (stack.is_null()) {
		return false;
	}
	if (!stack->get_modification_count()) {
		return false;
	}
	bool found = false;
	for (int32_t count_i = 0; count_i < stack->get_modification_count(); count_i++) {
		Ref<SkeletonModification3D> mod = stack->get_modification(count_i);
		if (mod.is_null()) {
			continue;
		}
		if (!mod->is_class("SkeletonModification3DEWBIK")) {
			continue;
		}
		found = true;
		break;
	}
	return found;
}

EWBIKSkeleton3DGizmoPlugin::EWBIKSkeleton3DGizmoPlugin() {
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

void EWBIKSkeleton3DGizmoPlugin::redraw(EditorNode3DGizmo *p_gizmo) {
	HashMap<int32_t, HashMap<int32_t, Vector<float>>> modification_kusudama_constraint;
	Ref<SkeletonModificationStack3D> stack;
	Skeleton3D *skeleton = Object::cast_to<Skeleton3D>(p_gizmo->get_spatial_node());
	if (skeleton) {
		stack = skeleton->get_modification_stack();
	}
	if (stack.is_valid()) {
		for (int32_t modification_i = 0; modification_i < stack->get_modification_count(); modification_i++) {
			Ref<SkeletonModification3D> modification = stack->get_modification(modification_i);
			if (modification.is_null()) {
				continue;
			}
			Ref<SkeletonModification3DEWBIK> ewbik_modification = modification;
			if (ewbik_modification.is_null()) {
				continue;
			}
			int32_t constraint_count = ewbik_modification->get_constraint_count();
			HashMap<int32_t, Vector<float>> kusudama_constraint;
			for (int32_t constraint_i = 0; constraint_i < constraint_count; constraint_i++) {
				Vector<float> cone_constraint;
				for (int32_t cone_i = 0; cone_i < ewbik_modification->get_kusudama_limit_cone_count(constraint_i); cone_i++) {
					Vector3 center = ewbik_modification->get_kusudama_limit_cone_center(constraint_i, cone_i);
					Vector<float> cone;
					cone.resize(4);
					cone.write[0] = center.x;
					cone.write[1] = center.y;
					cone.write[2] = center.z;
					float radius = ewbik_modification->get_kusudama_limit_cone_radius(constraint_i, cone_i);
					cone.write[3] = radius;
					cone_constraint.append_array(cone);
				}
				kusudama_constraint[constraint_i] = cone_constraint;
			}
			modification_kusudama_constraint[modification_i] = kusudama_constraint;
		}
	}
	p_gizmo->clear();

	Color bone_color = EditorSettings::get_singleton()->get("editors/3d_gizmos/gizmo_colors/skeleton");

	Ref<SurfaceTool> surface_tool(memnew(SurfaceTool));
	surface_tool->begin(Mesh::PRIMITIVE_LINES);

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

		Color current_bone_color = bone_color;

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
			real_t dist = v0.distance_to(v1);
			bones[0] = current_bone_idx;

			if (stack.is_valid()) {
				for (int32_t modification_i = 0; modification_i < stack->get_modification_count(); modification_i++) {
					HashMap<int32_t, Vector<float>> current_kusudama_constraint = modification_kusudama_constraint[modification_i];
					Ref<SkeletonModification3DEWBIK> modification = stack->get_modification(0);
					String bone_name = skeleton->get_bone_name(current_bone_idx);
					BoneId constraint_id = modification->find_effector_id(bone_name);
					if (modification.is_valid() && current_kusudama_constraint.has(constraint_id)) {
						Ref<SphereMesh> sphere_mesh;
						sphere_mesh.instantiate();
						sphere_mesh->set_radius(dist / 8.0f);
						sphere_mesh->set_height(dist / 4.0f);
						PackedFloat32Array kusudama_limit_cones;
						kusudama_limit_cones.resize(KUSUDAMA_MAX_CONES * 4);
						kusudama_limit_cones.fill(0.0f);
						Vector<float> limit_cones = current_kusudama_constraint[constraint_id];
						for (int32_t value_i = 0; value_i < limit_cones.size(); value_i++) {
							kusudama_limit_cones.write[value_i] = limit_cones[value_i];
						}
						Ref<ShaderMaterial> kusudama_material = Ref<ShaderMaterial>(memnew(ShaderMaterial));
						Ref<Shader> kusudama_shader = Ref<Shader>(memnew(Shader));
						kusudama_shader->set_code(R"(
// Skeleton 3D gizmo kusudama constraint shader.
shader_type spatial;
render_mode depth_prepass_alpha, cull_disabled;

uniform vec4 kusudama_color : source_color = vec4(0.58039218187332, 0.27058824896812, 0.00784313771874, 1.0);
const int CONE_COUNT_MAX = 30;

// 0,0,0 is the center of the kusudama. The kusudamas have their own bases that automatically get reoriented such that +y points in the direction that is the weighted average of the limitcones on the kusudama.
// But, if you have a kusuduma with just 1 limit_cone, then in general that limit_cone should be 0,1,0 in the kusudama's basis unless the user has specifically specified otherwise.

uniform vec4 cone_sequence[30];

// This shader can display up to 30 cones (represented by 30 4d vectors) 
// Each group of 4 represents the xyz coordinates of the cone direction
// vector in model space and the fourth element represents radius

// TODO: fire 2022-05-26
// Use a texture to store bone parameters.
// Use the uv to get the row of the bone.

varying vec3 normal_model_dir;
varying vec4 vert_model_color;

bool is_in_inter_cone_path(in vec3 normal_dir, in vec4 tangent_1, in vec4 cone_1, in vec4 tangent_2, in vec4 cone_2) {
	vec3 c1xc2 = cross(cone_1.xyz, cone_2.xyz);		
	float c1c2dir = dot(normal_dir, c1xc2);
		
	if (c1c2dir < 0.0) { 
		vec3 c1xt1 = cross(cone_1.xyz, tangent_1.xyz); 
		vec3 t1xc2 = cross(tangent_1.xyz, cone_2.xyz);	
		float c1t1dir = dot(normal_dir, c1xt1);
		float t1c2dir = dot(normal_dir, t1xc2);
		
	 	return (c1t1dir > 0.0 && t1c2dir > 0.0); 
			
	} else {
		vec3 t2xc1 = cross(tangent_2.xyz, cone_1.xyz);	
		vec3 c2xt2 = cross(cone_2.xyz, tangent_2.xyz);	
		float t2c1dir = dot(normal_dir, t2xc1);
		float c2t2dir = dot(normal_dir, c2xt2);
		
		return (c2t2dir > 0.0 && t2c1dir > 0.0);
	}	
	return false;
}

//determines the current draw condition based on the desired draw condition in the setToArgument
// -3 = disallowed entirely; 
// -2 = disallowed and on tangent_cone boundary
// -1 = disallowed and on control_cone boundary
// 0 =  allowed and empty; 
// 1 =  allowed and on control_cone boundary
// 2  = allowed and on tangent_cone boundary
int get_allowability_condition(in int current_condition, in int set_to) {
	if((current_condition == -1 || current_condition == -2)
		&& set_to >= 0) {
		return current_condition *= -1;
	} else if(current_condition == 0 && (set_to == -1 || set_to == -2)) {
		return set_to *=-2;
	}  	
	return max(current_condition, set_to);
}

// returns 1 if normal_dir is beyond (cone.a) radians from the cone.rgb
// returns 0 if normal_dir is within (cone.a + boundary_width) radians from the cone.rgb
// return -1 if normal_dir is less than (cone.a) radians from the cone.rgb
int is_in_cone(in vec3 normal_dir, in vec4 cone, in float boundary_width) {
	float arc_dist_to_cone = acos(dot(normal_dir, cone.rgb));
	if (arc_dist_to_cone > (cone.a+(boundary_width/2.))) {
		return 1; 
	}
	if (arc_dist_to_cone < (cone.a-(boundary_width/2.))) {
		return -1;
	}
	return 0;
} 

// Returns a color corresponding to the allowability of this region,
// or otherwise the boundaries corresponding 
// to various cones and tangent_cone.
vec4 color_allowed(in vec3 normal_dir,  in int cone_counts, in float boundary_width) {
	int current_condition = -3;
	if (cone_counts == 1) {
		vec4 cone = cone_sequence[0];
		int in_cone = is_in_cone(normal_dir, cone, boundary_width);
		bool is_in_cone = in_cone == 0;
		if (is_in_cone) {
			in_cone = -1;
		} else {
			if (in_cone < 0) {
				in_cone = 0;
			} else {
				in_cone = -3;
			}
		}
		current_condition = get_allowability_condition(current_condition, in_cone);
	} else {
		for(int i=0; i < cone_counts-1; i += 3) {
			normal_dir = normalize(normal_dir);
			int idx = i*3; 
			vec4 cone_1 = cone_sequence[idx];
			vec4 tangent_1 = cone_sequence[idx+1];
			vec4 tangent_2 = cone_sequence[idx+2];
			vec4 cone_2 = cone_sequence[idx+3];

			int inCone1 = is_in_cone(normal_dir, cone_1, boundary_width);
			if (inCone1 == 0) {
				inCone1 = -1;
			} else {
				if (inCone1 < 0) {
					inCone1 = 0;
				} else {
					inCone1 = -3;
				}
			}
			current_condition = get_allowability_condition(current_condition, inCone1);

			int inCone2 = is_in_cone(normal_dir, cone_2, boundary_width);
			if (inCone2 == 0) {
				inCone2 = -1;
			} else {
				if (inCone2 < 0) {
					inCone2 = 0;
				} else {
					inCone2 = -3;
				}
			}
			current_condition = get_allowability_condition(current_condition, inCone2);

			int in_tan_1 = is_in_cone(normal_dir, tangent_1, boundary_width); 
			int in_tan_2 = is_in_cone(normal_dir, tangent_2, boundary_width);

			if (float(in_tan_1) < 1. || float(in_tan_2) < 1.) {
				in_tan_1 = in_tan_1 == 0 ? -2 : -3;
				current_condition = get_allowability_condition(current_condition, in_tan_1);
				in_tan_2 = in_tan_2 == 0 ? -2 : -3;
				current_condition = get_allowability_condition(current_condition, in_tan_2);
			} else {
				bool in_intercone = is_in_inter_cone_path(normal_dir, tangent_1, cone_1, tangent_2, cone_2);
				int intercone_condition = in_intercone ? 0 : -3;
				current_condition = get_allowability_condition(current_condition, intercone_condition);
			}
		}
	}
	vec4 result = vert_model_color;
	if (current_condition != 0) {
		float on_tan_boundary = abs(current_condition) == 2 ? 0.3 : 0.0; 
		float on_cone_boundary = abs(current_condition) == 1 ? 0.3 : 0.0;
		result += vec4(0.0, on_cone_boundary, on_tan_boundary, 0.0);
	} else {
		return vec4(0.0, 0.0, 0.0, 0.0);
	}
	return result;
}

void vertex() {
	normal_model_dir = NORMAL;
	vert_model_color.rgb = kusudama_color.rgb;
}

void fragment() {
	vec4 result_color_allowed = vec4(0.0, 0.0, 0.0, 0.0);
	if (cone_sequence.length() == 30) {
		result_color_allowed = color_allowed(normal_model_dir, CONE_COUNT_MAX, 0.02);
	}
	if (result_color_allowed.a == 0.0) {
		discard;
	}
	ALBEDO = result_color_allowed.rgb;
	ALPHA = result_color_allowed.a;
}
)");
						kusudama_material->set_shader(kusudama_shader);
						kusudama_material->set_shader_param("cone_sequence", kusudama_limit_cones);
						kusudama_material->set_shader_param("kusudama_color", current_bone_color);
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
						Transform3D transform = skeleton->get_bone_global_rest(current_bone_idx);
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
							kusudama_surface_tool->add_vertex(transform.xform(sphere_vertex));
						}
						for (int32_t index_i = 0; index_i < index_array.size(); index_i++) {
							int32_t index = index_array[index_i];
							kusudama_surface_tool->add_index(index);
						}

						p_gizmo->add_mesh(kusudama_surface_tool->commit(), kusudama_material, Transform3D(), skeleton->register_skin(skeleton->create_skin_from_rest_transforms()));
					}
				}
			}
			// Add the bone's children to the list of bones to be processed.
			bones_to_process.push_back(child_bones_vector[i]);
		}
	}
}