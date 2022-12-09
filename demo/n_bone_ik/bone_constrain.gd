@tool
extends EditorScript

func enable_debug_pins(pins, new_ik : ManyBoneIK3D) -> void:
	for pin_i in range(pins.size()):
		new_ik.set_pin_weight(pin_i, 0)
		if str(pins[pin_i]) == "Root":
			new_ik.set_pin_weight(pin_i, 1)
		new_ik.default_damp = 0.001

func _run():
	var root : Node3D = get_editor_interface().get_edited_scene_root()
	if root == null:
		return
	var skeletons : Array[Node] = root.find_children("*", "Skeleton3D")
	for skeleton in skeletons:
		if not skeleton.get_bone_count():
			continue
		var iks : Array[Node] = skeleton.find_children("*", "ManyBoneIK3D")
		var edit_mode = false
		for ik in iks:
			edit_mode = ik.edit_constraints
			ik.free()
		var new_ik : ManyBoneIK3D = ManyBoneIK3D.new()
		skeleton.add_child(new_ik, true)
		new_ik.owner = root
		new_ik.visible = false
		new_ik.default_damp = deg_to_rad(270)
		new_ik.set_pin_count(0)
		new_ik.set_constraint_count(0)
		var pins : Array
		for bone_i in skeleton.get_bone_count():
			var bone_name = skeleton.get_bone_name(bone_i)
			if bone_i == -1:
				continue
			if pins.has(bone_name):
				continue
			pins.push_back(bone_name)
		for pin in pins:
			var node = root.find_child(pin)
			if node != null:
				node.free()
		skeleton.reset_bone_poses()
		new_ik.set_pin_count(pins.size())

		var constraints : Array
		for bone_i in skeleton.get_bone_count():
			var bone_name = skeleton.get_bone_name(bone_i)
			if bone_i == -1:
				continue
			constraints.push_back(bone_name)
		new_ik.set_constraint_count(constraints.size())

		for constraint_i in constraints.size():
			var bone_name = constraints[constraint_i]
			new_ik.set_constraint_name(constraint_i, bone_name)

		for constraint_i in new_ik.get_constraint_count():
			var bone_name : String = new_ik.get_constraint_name(constraint_i)
			var twist_min = new_ik.get_kusudama_twist(constraint_i).x

		for bone_i in new_ik.get_pin_count():
			var bone_name : String = new_ik.get_pin_bone_name(bone_i)
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
		new_ik.edit_constraints = edit_mode
