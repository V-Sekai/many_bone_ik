/*************************************************************************/
/*  many_bone_ik_skeleton_3d_gizmo_plugin.cpp                                   */
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

#include "many_bone_ik_3d_gizmo_plugin.h"

#include "core/io/resource_saver.h"
#include "core/math/transform_3d.h"
#include "editor/editor_file_dialog.h"
#include "editor/editor_node.h"
#include "editor/editor_properties.h"
#include "editor/editor_scale.h"
#include "editor/plugins/animation_player_editor_plugin.h"
#include "editor/plugins/node_3d_editor_gizmos.h"
#include "editor/plugins/node_3d_editor_plugin.h"
#include "scene/3d/collision_shape_3d.h"
#include "scene/3d/joint_3d.h"
#include "scene/3d/label_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/physics_body_3d.h"
#include "scene/3d/skeleton_3d.h"
#include "scene/resources/capsule_shape_3d.h"
#include "scene/resources/primitive_meshes.h"
#include "scene/resources/sphere_shape_3d.h"
#include "scene/resources/surface_tool.h"
#include "scene/scene_string_names.h"

#include "../src/ik_kusudama.h"

bool ManyBoneIK3DGizmoPlugin::has_gizmo(Node3D *p_spatial) {
	return cast_to<Node3D>(p_spatial) && !cast_to<Skeleton3D>(p_spatial);
}

String ManyBoneIK3DGizmoPlugin::get_gizmo_name() const {
	return "ManyBoneIK3D";
}

void ManyBoneIK3DGizmoPlugin::redraw(EditorNode3DGizmo *p_gizmo) {
	if (!p_gizmo) {
		return;
	}
	p_gizmo->clear();
	if (!p_gizmo->is_selected()) {
		return;
	}
	Node3D *node_3d = p_gizmo->get_node_3d();
	if (!node_3d) {
		return;
	}
	if (!node_3d->is_visible_in_tree()) {
		return;
	}
	Node *root = node_3d->get_tree()->get_edited_scene_root();
	TypedArray<Node> nodes = root->find_children("*", "ManyBoneIK3D");
	for (int32_t node_i = 0; node_i < nodes.size(); node_i++) {
		ManyBoneIK3D *many_bone_ik = cast_to<ManyBoneIK3D>(nodes[node_i]);
		if (!many_bone_ik) {
			return;
		}
		Skeleton3D *many_bone_ik_skeleton = many_bone_ik->get_skeleton();
		if (!many_bone_ik_skeleton) {
			return;
		}
		if (!many_bone_ik_skeleton->is_connected(SceneStringNames::get_singleton()->pose_updated, callable_mp(node_3d, &Node3D::update_gizmos))) {
			many_bone_ik_skeleton->connect(SceneStringNames::get_singleton()->pose_updated, callable_mp(node_3d, &Node3D::update_gizmos));
		}
		Vector<int> bones_to_process = many_bone_ik_skeleton->get_parentless_bones();
		int bones_to_process_i = 0;
		Vector<BoneId> processing_bones;
		Vector<Ref<IKBoneSegment>> bone_segments = many_bone_ik->get_segmented_skeletons();
		for (Ref<IKBoneSegment> bone_segment : bone_segments) {
			if (bone_segment.is_null()) {
				continue;
			}
			while (bones_to_process_i < bones_to_process.size()) {
				int current_bone_idx = bones_to_process[bones_to_process_i];
				processing_bones.push_back(current_bone_idx);
				Vector<int> child_bones_vector = many_bone_ik_skeleton->get_bone_children(current_bone_idx);
				for (int child_bone_idx : child_bones_vector) {
					bones_to_process.push_back(child_bone_idx);
				}
				bones_to_process_i++;
			}
			Color current_bone_color = bone_color;
			for (BoneId bone_i : bones_to_process) {
				Ref<IKBone3D> ik_bone = bone_segment->get_ik_bone(bone_i);
				if (ik_bone.is_null()) {
					continue;
				}
				create_gizmo_mesh(bone_i, ik_bone, p_gizmo, current_bone_color, many_bone_ik_skeleton, many_bone_ik);
				create_gizmo_handles(bone_i, ik_bone, p_gizmo, current_bone_color, many_bone_ik_skeleton, many_bone_ik);
			}
		}
	}
}

void ManyBoneIK3DGizmoPlugin::create_gizmo_mesh(BoneId current_bone_idx, Ref<IKBone3D> ik_bone, EditorNode3DGizmo *p_gizmo, Color current_bone_color, Skeleton3D *many_bone_ik_skeleton, ManyBoneIK3D *p_many_bone_ik) {
	Ref<IKKusudama> ik_kusudama = ik_bone->get_constraint();
	if (ik_kusudama.is_null()) {
		return;
	}
	BoneId parent_idx = many_bone_ik_skeleton->get_bone_parent(current_bone_idx);
	Vector<Vector3> handles;
	LocalVector<int> bones;
	LocalVector<float> weights;
	bones.resize(4);
	weights.resize(4);
	for (int i = 0; i < 4; i++) {
		bones[i] = 0;
		weights[i] = 0;
	}
	bones[0] = parent_idx;
	weights[0] = 1;

	Transform3D constraint_relative_to_the_skeleton = p_gizmo->get_node_3d()->get_global_transform().affine_inverse() * many_bone_ik_skeleton->get_global_transform() * ik_bone->get_constraint_transform()->get_global_transform();
	PackedFloat32Array kusudama_limit_cones;
	Ref<IKKusudama> kusudama = ik_bone->get_constraint();
	kusudama_limit_cones.resize(KUSUDAMA_MAX_CONES * 4);
	kusudama_limit_cones.fill(0.0f);
	int out_idx = 0;
	const TypedArray<IKLimitCone> &limit_cones = ik_kusudama->get_limit_cones();
	for (int32_t cone_i = 0; cone_i < limit_cones.size(); cone_i++) {
		Ref<IKLimitCone> limit_cone = limit_cones[cone_i];
		Vector3 control_point = limit_cone->get_control_point();
		kusudama_limit_cones.write[out_idx + 0] = control_point.x;
		kusudama_limit_cones.write[out_idx + 1] = control_point.y;
		kusudama_limit_cones.write[out_idx + 2] = control_point.z;
		float radius = limit_cone->get_radius();
		kusudama_limit_cones.write[out_idx + 3] = radius;
		out_idx += 4;

		Vector3 tangent_center_1 = limit_cone->get_tangent_circle_center_next_1();
		kusudama_limit_cones.write[out_idx + 0] = tangent_center_1.x;
		kusudama_limit_cones.write[out_idx + 1] = tangent_center_1.y;
		kusudama_limit_cones.write[out_idx + 2] = tangent_center_1.z;
		float tangent_radius = limit_cone->get_tangent_circle_radius_next();
		kusudama_limit_cones.write[out_idx + 3] = tangent_radius;
		out_idx += 4;

		Vector3 tangent_center_2 = limit_cone->get_tangent_circle_center_next_2();
		kusudama_limit_cones.write[out_idx + 0] = tangent_center_2.x;
		kusudama_limit_cones.write[out_idx + 1] = tangent_center_2.y;
		kusudama_limit_cones.write[out_idx + 2] = tangent_center_2.z;
		kusudama_limit_cones.write[out_idx + 3] = tangent_radius;
		out_idx += 4;
	}
	if (current_bone_idx >= many_bone_ik_skeleton->get_bone_count()) {
		return;
	}
	if (current_bone_idx <= -1) {
		return;
	}
	if (parent_idx >= many_bone_ik_skeleton->get_bone_count()) {
		return;
	}
	if (parent_idx <= -1) {
		return;
	}
	Vector3 v0 = many_bone_ik_skeleton->get_bone_global_rest(current_bone_idx).origin;
	Vector3 v1 = many_bone_ik_skeleton->get_bone_global_rest(parent_idx).origin;
	real_t dist = v0.distance_to(v1);
	float radius = dist / 5.0;
	// Code copied from the SphereMesh.
	float height = dist / 2.5;
	int rings = 32;

	int i, j, prevrow, thisrow, point;
	float x, y, z;

	float scale = height * 0.5;

	Vector<Vector3> points;
	Vector<Vector3> normals;
	Vector<int> indices;
	point = 0;

	thisrow = 0;
	prevrow = 0;
	for (j = 0; j <= (rings + 1); j++) {
		int radial_segments = 32;
		float v = j;
		float w;

		v /= (rings + 1);
		w = sin(Math_PI * v);
		y = scale * cos(Math_PI * v);

		for (i = 0; i <= radial_segments; i++) {
			float u = i;
			u /= radial_segments;

			x = sin(u * Math_TAU);
			z = cos(u * Math_TAU);

			Vector3 p = Vector3(x * scale * w, y, z * scale * w);
			points.push_back(p);
			Vector3 normal = Vector3(x * w * scale, radius * (y / scale), z * w * scale);
			normals.push_back(normal.normalized());
			point++;

			if (i > 0 && j > 0) {
				indices.push_back(prevrow + i - 1);
				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i - 1);

				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i);
				indices.push_back(thisrow + i - 1);
			};
		};

		prevrow = thisrow;
		thisrow = point;
	}
	if (!indices.size()) {
		return;
	}
	Ref<SurfaceTool> surface_tool;
	surface_tool.instantiate();
	surface_tool->begin(Mesh::PRIMITIVE_TRIANGLES);
	const int32_t MESH_CUSTOM_0 = 0;
	surface_tool->set_custom_format(MESH_CUSTOM_0, SurfaceTool::CustomFormat::CUSTOM_RGBA_HALF);
	for (int32_t point_i = 0; point_i < points.size(); point_i++) {
		surface_tool->set_bones(bones);
		surface_tool->set_weights(weights);
		Color c;
		c.r = normals[point_i].x;
		c.g = normals[point_i].y;
		c.b = normals[point_i].z;
		c.a = 0;
		surface_tool->set_custom(MESH_CUSTOM_0, c);
		surface_tool->set_normal(normals[point_i]);
		surface_tool->add_vertex(points[point_i]);
	}
	for (int32_t index_i : indices) {
		surface_tool->add_index(index_i);
	}
	Ref<ShaderMaterial> kusudama_material;
	kusudama_material.instantiate();
	kusudama_material->set_shader(kusudama_shader);
	kusudama_material->set_shader_parameter("cone_sequence", kusudama_limit_cones);
	int32_t cone_count = kusudama->get_limit_cones().size();
	kusudama_material->set_shader_parameter("cone_count", cone_count);
	kusudama_material->set_shader_parameter("kusudama_color", current_bone_color);
	p_gizmo->add_mesh(
			surface_tool->commit(Ref<Mesh>(), RS::ARRAY_CUSTOM_RGBA_HALF << RS::ARRAY_FORMAT_CUSTOM0_SHIFT),
			kusudama_material, constraint_relative_to_the_skeleton);
}

ManyBoneIK3DGizmoPlugin::ManyBoneIK3DGizmoPlugin() {
	create_material("lines_primary", Color(0.93725490570068, 0.19215686619282, 0.22352941334248), true, true, true);
	Ref<Texture2D> handle_center = Node3DEditor::get_singleton()->get_theme_icon(SNAME("EditorPivot"), SNAME("EditorIcons"));
	create_handle_material("handles", false, handle_center);
	Ref<Texture2D> handle_radius = Node3DEditor::get_singleton()->get_theme_icon(SNAME("Editor3DHandle"), SNAME("EditorIcons"));
	create_handle_material("handles_radius", false, handle_radius);
	create_handle_material("handles_billboard", true);
	Ref<Texture2D> handle_axial_from = Node3DEditor::get_singleton()->get_theme_icon(SNAME("SpringArm3D"), SNAME("EditorIcons"));
	create_handle_material("handles_axial_from", false, handle_axial_from);
	Ref<Texture2D> handle_axial_middle = Node3DEditor::get_singleton()->get_theme_icon(SNAME("Node"), SNAME("EditorIcons"));
	create_handle_material("handles_axial_middle", false, handle_axial_middle);
	Ref<Texture2D> handle_axial_to = Node3DEditor::get_singleton()->get_theme_icon(SNAME("Node"), SNAME("EditorIcons"));
	create_handle_material("handles_axial_to", false, handle_axial_to);
	Ref<Texture2D> handle_axial_current = Node3DEditor::get_singleton()->get_theme_icon(SNAME("Node2D"), SNAME("EditorIcons"));
	create_handle_material("handles_axial_current", false, handle_axial_current);
	kusudama_shader.instantiate();
	kusudama_shader->set_code(MANY_BONE_IKKUSUDAMA_SHADER);
}

void ManyBoneIK3DGizmoPlugin::create_gizmo_handles(BoneId current_bone_idx, Ref<IKBone3D> ik_bone, EditorNode3DGizmo *p_gizmo, Color current_bone_color, Skeleton3D *many_bone_ik_skeleton, ManyBoneIK3D *p_many_bone_ik) {
	// TEST PLAN:
	// You will also want to make sure it's robust to translations of the skeleton node and root bone
	Ref<IKKusudama> ik_kusudama = ik_bone->get_constraint();
	if (ik_kusudama.is_null()) {
		return;
	}
	BoneId parent_idx = many_bone_ik_skeleton->get_bone_parent(current_bone_idx);
	LocalVector<int> bones;
	LocalVector<float> weights;
	bones.resize(4);
	weights.resize(4);
	for (int i = 0; i < 4; i++) {
		bones[i] = 0;
		weights[i] = 0;
	}
	bones[0] = parent_idx;
	weights[0] = 1;
	Transform3D constraint_relative_to_the_universe = p_gizmo->get_node_3d()->get_global_transform().affine_inverse() * many_bone_ik_skeleton->get_global_transform() * ik_bone->get_constraint_transform()->get_global_transform();
	Transform3D handle_transform;
	if (p_gizmo->get_node_3d()->get_parent()) {
		Node3D *node = cast_to<Node3D>(p_gizmo->get_node_3d()->get_parent());
		if (node) {
			handle_transform = node->get_global_transform();
		}
	}
	PackedFloat32Array kusudama_limit_cones;
	Ref<IKKusudama> kusudama = ik_bone->get_constraint();
	if (current_bone_idx >= many_bone_ik_skeleton->get_bone_count()) {
		return;
	}
	if (current_bone_idx <= -1) {
		return;
	}
	if (parent_idx >= many_bone_ik_skeleton->get_bone_count()) {
		return;
	}
	if (parent_idx <= -1) {
		return;
	}
	Vector3 v0 = many_bone_ik_skeleton->get_bone_global_rest(current_bone_idx).origin;
	Vector3 v1 = many_bone_ik_skeleton->get_bone_global_rest(parent_idx).origin;
	real_t dist = v0.distance_to(v1);
	float radius = dist / 5.0;
	float r = radius;
	Vector<Vector3> center_handles;
	Vector<Vector3> radius_handles;
	Vector<Vector3> handles_current;
	Vector<Vector3> axial_from_handles;
	TypedArray<Vector3> axial_middle_handles;
	TypedArray<Vector3> axial_current_handles;
	Vector<Vector3> axial_to_handles;
	kusudama_limit_cones.resize(KUSUDAMA_MAX_CONES * 4);
	kusudama_limit_cones.fill(0.0f);
	int out_idx = 0;
	const TypedArray<IKLimitCone> &limit_cones = ik_kusudama->get_limit_cones();
	for (int32_t cone_i = 0; cone_i < limit_cones.size(); cone_i++) {
		Ref<IKLimitCone> limit_cone = limit_cones[cone_i];
		Vector3 control_point = limit_cone->get_control_point();
		kusudama_limit_cones.write[out_idx + 0] = control_point.x;
		kusudama_limit_cones.write[out_idx + 1] = control_point.y;
		kusudama_limit_cones.write[out_idx + 2] = control_point.z;
		float radius = limit_cone->get_radius();
		kusudama_limit_cones.write[out_idx + 3] = radius;
		out_idx += 4;

		Vector3 tangent_center_1 = limit_cone->get_tangent_circle_center_next_1();
		kusudama_limit_cones.write[out_idx + 0] = tangent_center_1.x;
		kusudama_limit_cones.write[out_idx + 1] = tangent_center_1.y;
		kusudama_limit_cones.write[out_idx + 2] = tangent_center_1.z;
		float tangent_radius = limit_cone->get_tangent_circle_radius_next();
		kusudama_limit_cones.write[out_idx + 3] = tangent_radius;
		out_idx += 4;

		Vector3 tangent_center_2 = limit_cone->get_tangent_circle_center_next_2();
		kusudama_limit_cones.write[out_idx + 0] = tangent_center_2.x;
		kusudama_limit_cones.write[out_idx + 1] = tangent_center_2.y;
		kusudama_limit_cones.write[out_idx + 2] = tangent_center_2.z;
		kusudama_limit_cones.write[out_idx + 3] = tangent_radius;
		out_idx += 4;
	}
	Ref<SurfaceTool> surface_tool;
	surface_tool.instantiate();
	surface_tool->begin(Mesh::PRIMITIVE_LINES);
	for (int32_t cone_i = 0; cone_i < kusudama->get_limit_cones().size() * (3 * 4); cone_i = cone_i + (3 * 4)) {
		Vector3 center = Vector3(kusudama_limit_cones[cone_i + 0], kusudama_limit_cones[cone_i + 1], kusudama_limit_cones[cone_i + 2]);
		float cone_radius = kusudama_limit_cones[cone_i + 3];
		if (Math::is_zero_approx(center.length_squared())) {
			center = Vector3(0, 1, 0);
			cone_radius = 0.0;
		}
		{
			Transform3D handle_relative_to_mesh;
			handle_relative_to_mesh.origin = center * radius;
			Transform3D handle_relative_to_universe = handle_transform.affine_inverse() * constraint_relative_to_the_universe * handle_relative_to_mesh;
			center_handles.push_back(handle_relative_to_universe.origin);
		}
		{
			float w = r * Math::sin(cone_radius);
			float d = r * Math::cos(cone_radius);
			const float ra = (float)(0 * 3);
			const Point2 a = Vector2(Math::sin(ra), Math::cos(ra)) * w;
			Transform3D handle_border_relative_to_mesh;
			Transform3D center_relative_to_mesh = Transform3D(Quaternion(Vector3(0, 1, 0), center));
			handle_border_relative_to_mesh.origin = center_relative_to_mesh.xform(Vector3(a.x, a.y, -d));
			Transform3D handle_border_relative_to_universe = constraint_relative_to_the_universe * handle_border_relative_to_mesh;
			radius_handles.push_back((handle_transform.affine_inverse() * handle_border_relative_to_universe).origin);
		}
	}

	Transform3D twist_constraint_relative_to_the_universe = p_gizmo->get_node_3d()->get_global_transform().affine_inverse() * many_bone_ik_skeleton->get_global_transform() * ik_bone->get_constraint_twist_transform()->get_global_transform();
	const Vector3 axial_center = Vector3(0, 1, 0);
	float cone_radius = Math::deg_to_rad(90.0f);
	float w = r * Math::sin(cone_radius);
	float d = r * Math::cos(cone_radius);
	{
		const float ra = (float)kusudama->get_min_axial_angle();
		const Point2 a = Vector2(Math::sin(ra), Math::cos(ra)) * w;
		Transform3D axial_from_relative_to_mesh;
		Transform3D center_relative_to_mesh = Transform3D(Quaternion(Vector3(0, 1, 0), axial_center));
		axial_from_relative_to_mesh.origin = center_relative_to_mesh.xform(Vector3(a.x, a.y, -d));
		Transform3D axial_relative_to_universe = twist_constraint_relative_to_the_universe * axial_from_relative_to_mesh;
		axial_from_handles.push_back((handle_transform.affine_inverse() * axial_relative_to_universe).origin);
	}
	if (center_handles.size() && radius_handles.size()) {
		p_gizmo->add_handles(center_handles, get_material("handles"), Vector<int>(), false, true);
		p_gizmo->add_handles(radius_handles, get_material("handles_radius"), Vector<int>(), false, true);
	}
	float start_angle = kusudama->get_min_axial_angle();
	float end_angle = start_angle + kusudama->get_range_angle();
	float gaps = Math::deg_to_rad(20.0f);
	for (float theta = start_angle; theta < end_angle; theta += gaps) {
		const float ra = theta;
		const Point2 a = Vector2(Math::sin(ra), Math::cos(ra)) * w;
		Transform3D axial_from_relative_to_mesh;
		Transform3D center_relative_to_mesh = Transform3D(Quaternion(Vector3(0, 1, 0), axial_center));
		axial_from_relative_to_mesh.origin = center_relative_to_mesh.xform(Vector3(a.x, a.y, -d));
		Transform3D axial_relative_to_universe = twist_constraint_relative_to_the_universe * axial_from_relative_to_mesh;
		axial_from_relative_to_mesh.origin = center_relative_to_mesh.xform(Vector3(a.x, a.y, -d));
		axial_middle_handles.push_back((handle_transform.affine_inverse() * axial_relative_to_universe).origin);
	}
	axial_middle_handles.pop_front();
	axial_middle_handles.pop_back();
	{
		const float ra = kusudama->get_min_axial_angle() + (float)(kusudama->get_range_angle());
		const Point2 a = Vector2(Math::sin(ra), Math::cos(ra)) * w;
		Transform3D axial_from_relative_to_mesh;
		Transform3D center_relative_to_mesh = Transform3D(Quaternion(Vector3(0, 1, 0), axial_center));
		axial_from_relative_to_mesh.origin = center_relative_to_mesh.xform(Vector3(a.x, a.y, -d));
		Transform3D axial_relative_to_universe = twist_constraint_relative_to_the_universe * axial_from_relative_to_mesh;
		axial_to_handles.push_back((handle_transform.affine_inverse() * axial_relative_to_universe).origin);
	}
	if (axial_from_handles.size() && axial_to_handles.size()) {
		p_gizmo->add_handles(axial_from_handles, get_material("handles_axial_from"), Vector<int>(), true, false);
		p_gizmo->add_handles(axial_to_handles, get_material("handles_axial_to"), Vector<int>(), true, false);
	}
	if (axial_middle_handles.size()) {
		Vector<Vector3> handles;
		for (int32_t handle_i = 0; handle_i < axial_middle_handles.size(); handle_i++) {
			handles.push_back(axial_middle_handles[handle_i]);
		}
		p_gizmo->add_handles(handles, get_material("handles_axial_middle"), Vector<int>(), true, true);
	}
	{
		float current_angle = kusudama->get_min_axial_angle() + (ik_kusudama->get_current_twist_rotation(ik_bone) * ik_kusudama->get_range_angle());
		const Point2 a = Vector2(Math::sin(current_angle), Math::cos(current_angle)) * w;
		Transform3D center_relative_to_mesh = Transform3D(Quaternion(Vector3(0, 1, 0), axial_center));
		Transform3D axial_from_relative_to_mesh;
		axial_from_relative_to_mesh.origin = center_relative_to_mesh.xform(Vector3(a.x, a.y, -d));
		Transform3D axial_relative_to_universe = twist_constraint_relative_to_the_universe * axial_from_relative_to_mesh;
		Vector3 handle_position = (handle_transform.affine_inverse() * axial_relative_to_universe).origin;
		handles_current.push_back(handle_position);
	}
	if (handles_current.size()) {
		p_gizmo->add_handles(handles_current, get_material("handles_axial_current"), Vector<int>(), true, true);
	}
}

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
			update_joint_tree();

			// joint_tree->connect("item_selected", callable_mp(this, &Skeleton3DEditor::_joint_tree_selection_changed));
			// joint_tree->connect("item_mouse_selected", callable_mp(this, &Skeleton3DEditor::_joint_tree_rmb_select));
			// #ifdef TOOLS_ENABLED
			// 				skeleton->connect("pose_updated", callable_mp(this, &Skeleton3DEditor::_draw_gizmo));
			// 				skeleton->connect("pose_updated", callable_mp(this, &Skeleton3DEditor::_update_properties));
			// 				skeleton->connect("bone_enabled_changed", callable_mp(this, &Skeleton3DEditor::_bone_enabled_changed));
			// 				skeleton->connect("show_rest_only_changed", callable_mp(this, &Skeleton3DEditor::_update_gizmo_visible));
			// #endif
			// get_tree()->connect("node_removed", callable_mp(this, &Skeleton3DEditor::_node_removed), Object::CONNECT_ONE_SHOT);
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			update_joint_tree();
		} break;
		case NOTIFICATION_PREDELETE: {
			if (ik->get_skeleton()) {
				// select_bone(-1); // Requires that the joint_tree has not been deleted.
				// #ifdef TOOLS_ENABLED
				// 					skeleton->disconnect("show_rest_only_changed", callable_mp(this, &Skeleton3DEditor::_update_gizmo_visible));
				// 					skeleton->disconnect("bone_enabled_changed", callable_mp(this, &Skeleton3DEditor::_bone_enabled_changed));
				// 					skeleton->disconnect("pose_updated", callable_mp(this, &Skeleton3DEditor::_draw_gizmo));
				// 					skeleton->disconnect("pose_updated", callable_mp(this, &Skeleton3DEditor::_update_properties));
				// 					skeleton->set_transform_gizmo_visible(true);
				// #endif
			}
		} break;
	}
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
