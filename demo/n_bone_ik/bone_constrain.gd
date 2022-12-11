@tool
extends EditorScript

func _run():
	var root : Node3D = get_editor_interface().get_edited_scene_root()
	if root == null:
		return
	var iks : Array[Node] = root.find_children("*", "ManyBoneIK3D")
	for ik in iks:
		ik.free()
	var new_ik : ManyBoneIK3D = ManyBoneIK3D.new()
	var skeletons : Array[Node] = root.find_children("*", "Skeleton3D")
	var skeleton : Skeleton3D = skeletons[0]
	root.add_child(new_ik, true)
	new_ik.skeleton_node_path = "../" + str(root.get_path_to(skeleton))
	new_ik.owner = root
	new_ik.default_damp = deg_to_rad(270)
	new_ik.visible = false
	skeleton.reset_bone_poses()

	for bone_i in new_ik.get_pin_count():
		var bone_name : String = new_ik.get_pin_bone_name(bone_i)
		if bone_name.is_empty():
			print("Can't find bone id %s" % bone_i)
			continue
		var node_3d : BoneAttachment3D = BoneAttachment3D.new()
		node_3d.name = bone_name
		node_3d.bone_name = bone_name
		node_3d.bone_idx = skeleton.find_bone(bone_name)
		skeleton.add_child(node_3d, true)
		node_3d.owner = root
		var node_global_transform = node_3d.global_transform
		var path_string : String = "../" + str(skeleton.get_path_to(node_3d))
		new_ik.set_pin_nodepath(bone_i, NodePath(path_string))
		var marker_3d : Marker3D = Marker3D.new()
		marker_3d.name = bone_name
		marker_3d.global_transform = node_global_transform
		node_3d.replace_by(marker_3d, true)
	new_ik.visible = true
