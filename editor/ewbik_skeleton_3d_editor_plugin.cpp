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
#include "core/io/resource_saver.h"
#include "editor/editor_file_dialog.h"
#include "editor/editor_node.h"
#include "editor/editor_properties.h"
#include "editor/editor_scale.h"
#include "editor/plugins/animation_player_editor_plugin.h"
#include "editor/plugins/node_3d_editor_plugin.h"
#include "ewbik/skeleton_modification_3d_ewbik.h"
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
			break;
		}
	}
	p_gizmo->clear();

	Color bone_color = EditorSettings::get_singleton()->get("editors/3d_gizmos/gizmo_colors/skeleton");
	int bone_shape = EditorSettings::get_singleton()->get("editors/3d_gizmos/gizmo_settings/bone_shape");

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
					bones[0] = current_bone_idx;
					bones[0] = child_bone_idx;
				} break;

				case 1: { // Octahedron shape.
					Vector3 first;
					for (int j = 0; j < 3; j++) {
						Vector3 axis;
						if (first == Vector3()) {
							axis = d.cross(d.cross(skeleton->get_bone_global_rest(current_bone_idx).basis[j])).normalized();
							first = axis;
						} else {
							axis = d.cross(first).normalized();
						}
						for (int k = 0; k < 2; k++) {
							if (k == 1) {
								axis = -axis;
							}
							bones[0] = current_bone_idx;
							bones[0] = child_bone_idx;
						}
					}
					bones[0] = current_bone_idx;
				} break;
			}

			// Axis as root of the bone.
			for (int j = 0; j < 3; j++) {
				bones[0] = current_bone_idx;
				if (j == closest) {
					continue;
				}
			}

			// Axis at the end of the bone children.
			if (i == child_bones_size - 1) {
				for (int j = 0; j < 3; j++) {
					bones[0] = child_bone_idx;
					if (j == closest) {
						continue;
					}
				}
			}
			// Todo: Try some other modification;
			HashMap<int32_t, Vector<float>> current_kusudama_constraint = modification_kusudama_constraint[0];
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
			}
			// Add the bone's children to the list of bones to be processed.
			bones_to_process.push_back(child_bones_vector[i]);
		}
	}
}