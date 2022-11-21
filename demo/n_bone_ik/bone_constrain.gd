@tool
extends EditorScript

func enable_debug_pins(pins, new_ik : NBoneIK) -> void:
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
		var iks : Array[Node] = skeleton.find_children("*", "NBoneIK")
		for ik in iks:
			ik.free()
		var new_ik : NBoneIK = NBoneIK.new()
		skeleton.add_child(new_ik, true)
		new_ik.owner = root
		new_ik.visible = false
		new_ik.set_pin_count(0)
		new_ik.set_constraint_count(0)
		var pins =  [
			skeleton.get_bone_name(skeleton.get_parentless_bones()[0])
		]
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
		
		for pin_i in range(pins.size()):
			var pin = pins[pin_i]
			var bone_name = pin
			var bone_i = skeleton.find_bone(bone_name)
			if bone_i == -1:
				continue
			if bone_name in ["Hips"]:
				new_ik.set_pin_passthrough_factor(pin_i, 0.1)
			var node_3d : BoneAttachment3D = BoneAttachment3D.new()
			node_3d.name = bone_name
			node_3d.bone_name = bone_name
			node_3d.bone_idx = skeleton.find_bone(bone_name)
			skeleton.add_child(node_3d)
			node_3d.owner = root
			var path_string : String = "../" + str(skeleton.get_path_to(node_3d))
			new_ik.set_pin_nodepath(pin_i, NodePath(path_string))
			new_ik.set_pin_bone_name(pin_i, bone_name)
			new_ik.set_pin_passthrough_factor(pin_i, 1)			
			if bone_name in ["UpperChest"]:
				new_ik.set_pin_weight(pin_i, 0.01)
			var marker_3d : Marker3D = Marker3D.new()
			marker_3d.name = bone_name
			marker_3d.global_transform = node_3d.global_transform
			node_3d.replace_by(marker_3d, true)
			marker_3d.gizmo_extents = 0.1

		new_ik.visible = true
