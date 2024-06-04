extends Node

var many_bone_ik : ManyBoneIK3D
var skeleton : Skeleton3D
var selected : int = -1
var se : Skeleton3DEditor
var selected_bone_color : Color
var current_bone_index : int = 0
var bones_to_process : Array
var current_bone_idx : int
var current_bone_color : Color
var ik_bone : IKBone3D
var ik_kusudama : IKKusudama3D
var open_cones : Array
var parent_idx : int
var bones : Array
var weights : Array
var constraint_relative_to_the_skeleton : Transform3D
var kusudama_open_cones : Array
var kusudama : IKKusudama3D
var control_point : Vector3
var new_kusudama_open_cones : Array
var radius : float
var tangent_center_1 : Vector3
var tangent_radius : float
var tangent_center_2 : Vector3
var rings : int = 8
var i : int = 0
var j : int = 0
var prevrow : int = 0
var thisrow : int = 0
var point : int = 0
var x : float
var y : float
var z : float
var points : Array
var normals : Array
var indices : Array
var surface_tool : SurfaceTool
var kusudama_material : ShaderMaterial
var kusudama_shader : Shader
var many_bone_ik_gizmo_plugin : ManyBoneIK3DGizmoPlugin
var edit_mode : bool = false
var handle_material : ShaderMaterial
var handle_shader : Shader
var handles_mesh_instance : MeshInstance3D
var edit_mode_button : Button
var unselected_mat : StandardMaterial3D
var selected_mat : ShaderMaterial
var selected_sh : Shader

func _process(delta):
    many_bone_ik = p_gizmo.get_node_3d() as ManyBoneIK3D
    skeleton = p_gizmo.get_node_3d().get_skeleton()
    p_gizmo.clear()
    if not skeleton or skeleton.get_bone_count() == 0:
        return
    if handles_mesh_instance and not handles_mesh_instance.is_inside_tree():
        skeleton.call_deferred("add_child", handles_mesh_instance)
        handles_mesh_instance.set_skeleton_path(NodePath(""))
    se = Skeleton3DEditor.get_singleton()
    if se:
        selected = se.get_selected_bone()
    selected_bone_color = EDITOR_GET("editors/3d_gizmos/gizmo_colors/selected_bone")
    bones_to_process = skeleton.get_parentless_bones()
    while bones_to_process.size() > current_bone_index:
        current_bone_idx = bones_to_process[current_bone_index]
        current_bone_color = selected_bone_color if current_bone_idx == selected else bone_color
        for segmented_skeleton in many_bone_ik.get_segmented_skeletons():
            if segmented_skeleton.is_null():
                continue
            ik_bone = segmented_skeleton.get_ik_bone(current_bone_idx)
            if ik_bone.is_null() or ik_bone.get_constraint().is_null():
                continue
            create_gizmo_mesh(current_bone_idx, ik_bone, p_gizmo, current_bone_color, skeleton, many_bone_ik)
        current_bone_index += 1
        var child_bones_vector = skeleton.get_bone_children(current_bone_idx)
        var child_bones_size = child_bones_vector.size()
        for i in range(child_bones_size):
            if child_bones_vector[i] < 0:
                continue
            bones_to_process.push_back(child_bones_vector[i])
    ik_kusudama = ik_bone.get_constraint()
    if ik_kusudama.is_null():
        return
    open_cones = ik_kusudama.get_open_cones()
    if not open_cones.size():
        return
    parent_idx = many_bone_ik_skeleton.get_bone_parent(current_bone_idx)
    bones = []
    weights = []
    bones.resize(4)
    weights.resize(4)
    for i in range(4):
        bones[i] = 0
        weights[i] = 0
    bones[0] = parent_idx
    weights[0] = 1
    constraint_relative_to_the_skeleton = p_many_bone_ik.get_relative_transform(p_many_bone_ik.get_owner()).affine_inverse() * many_bone_ik_skeleton.get_relative_transform(many_bone_ik_skeleton.get_owner()) * p_many_bone_ik.get_godot_skeleton_transform_inverse() * ik_bone.get_constraint_orientation_transform().get_global_transform()
    kusudama_open_cones = []
    kusudama = ik_bone.get_constraint()
    for cone_i in range(open_cones.size()):
        var open_cone = open_cones[cone_i]
        control_point = open_cone.get_control_point()
        new_kusudama_open_cones = []
        new_kusudama_open_cones.resize(4 * 3)
        new_kusudama_open_cones.fill(0.0)
        new_kusudama_open_cones[0] = control_point.x
        new_kusudama_open_cones[1] = control_point.y
        new_kusudama_open_cones[2] = control_point.z
        radius = open_cone.get_radius()
        new_kusudama_open_cones[3] = radius
        tangent_center_1 = open_cone.get_tangent_circle_center_next_1()
        new_kusudama_open_cones[4] = tangent_center_1.x
        new_kusudama_open_cones[5] = tangent_center_1.y
        new_kusudama_open_cones[6] = tangent_center_1.z
        tangent_radius = open_cone.get_tangent_circle_radius_next()
        new_kusudama_open_cones[7] = tangent_radius
        tangent_center_2 = open_cone.get_tangent_circle_center_next_2()
        new_kusudama_open_cones[8] = tangent_center_2.x
        new_kusudama_open_cones[9] = tangent_center_2.y
        new_kusudama_open_cones[10] = tangent_center_2.z
        new_kusudama_open_cones[11] = tangent_radius
        kusudama_open_cones.append_array(new_kusudama_open_cones)
    if current_bone_idx >= many_bone_ik_skeleton.get_bone_count():
        return
    if current_bone_idx == -1:
        return
    if parent_idx >= many_bone_ik_skeleton.get_bone_count():
        return
    if parent_idx <= -1:
        return
    rings = 8
    i = 0
    j = 0
    prevrow = 0
    thisrow = 0
    point = 0
    points = []
    normals = []
    indices = []
    point = 0
    thisrow = 0
    prevrow = 0
    for j in range(rings + 2):
        var radial_segments = 8
        var v = j
        var w : float
        v /= (rings + 1)
        w = sin(PI * v)
        y = cos(PI * v)
        for i in range(radial_segments + 1):
            var u = i
            u /= radial_segments
            x = sin(u * TAU)
            z = cos(u * TAU)
            var p = Vector3(x * w, y, z * w) * 0.02
            points.append(p)
            var normal = Vector3(x * w, y, z * w)
            normals.append(normal.normalized())
            point += 1
            if i > 0 and j > 0:
                indices.append(prevrow + i - 1)
                indices.append(prevrow + i)
                indices.append(thisrow + i - 1)
                indices.append(prevrow + i)
                indices.append(thisrow + i)
                indices.append(thisrow + i - 1)
        prevrow = thisrow
        thisrow = point
    if not indices.size():
        return
    surface_tool = SurfaceTool.new()
    surface_tool.begin(Mesh.PRIMITIVE_TRIANGLES)
    const MESH_CUSTOM_0 = 0
    surface_tool.set_custom_format(MESH_CUSTOM_0, SurfaceTool.CustomFormat.CUSTOM_RGBA_HALF)
    for point_i in range(points.size()):
        surface_tool.set_bones(bones)
        surface_tool.set_weights(weights)
        var c : Color
        c.r = normals[point_i].x
        c.g = normals[point_i].y
        c.b = normals[point_i].z
        c.a = 0
        surface_tool.set_custom(MESH_CUSTOM_0, c)
        surface_tool.set_normal(normals[point_i])
        surface_tool.add_vertex(points[point_i])
    for index_i in indices:
        surface_tool.add_index(index_i)
    kusudama_material = ShaderMaterial.new()
    kusudama_material.set_shader(kusudama_shader)
    kusudama_material.set_shader_parameter("cone_sequence", kusudama_open_cones)
    var cone_count = kusudama.get_open_cones().size()
    kusudama_material.set_shader_parameter("cone_count", cone_count) 
