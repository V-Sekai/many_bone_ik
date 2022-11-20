@tool
extends EditorScript

func enable_debug_pins(pins, ewbik : NBoneIK):
	for pin_i in range(pins.size()):
		ewbik.set_pin_weight(pin_i, 0)
		if str(pins[pin_i]) == "Root":
			ewbik.set_pin_weight(pin_i, 1)
		ewbik.default_damp = 0.001

func create_pins(ewbik : NBoneIK, skeleton : Skeleton3D):	
	var root : Node3D = get_editor_interface().get_edited_scene_root()
	if root == null:
		return
	var profile : SkeletonProfileHumanoid = SkeletonProfileHumanoid.new()
	var bone_map : BoneMap = BoneMap.new()
	bone_map.profile = profile
	var pins =  [
		"Head",
		"LeftHand", 
		"RightHand", 
		"LeftFoot", 
		"RightFoot",
		"Spine",
		"Root",
	]
	for pin in pins:
		var node = root.find_child(pin)
		if node != null:
			node.free()
	ewbik.set_pin_count(pins.size())
	skeleton.reset_bone_poses()
	for pin_i in range(pins.size()):
		var pin = pins[pin_i]
		var bone_name = pin
		var bone_i = skeleton.find_bone(bone_name)
		if bone_i == -1:
			continue
		if bone_name in ["Hips"]:
			ewbik.set_pin_passthrough_factor(pin_i, 0.1)
		var node_3d : BoneAttachment3D = BoneAttachment3D.new()
		node_3d.name = bone_name
		node_3d.bone_name = bone_name
		node_3d.bone_idx = skeleton.find_bone(bone_name)
		skeleton.add_child(node_3d)
		node_3d.owner = root
		var path_string : String = "../" + str(skeleton.get_path_to(node_3d))
		ewbik.set_pin_nodepath(pin_i, NodePath(path_string))
		ewbik.set_pin_bone_name(pin_i, bone_name)
		ewbik.set_pin_passthrough_factor(pin_i, 1)			
		if bone_name in ["UpperChest"]:
			ewbik.set_pin_weight(pin_i, 0.01)
		var marker_3d : Marker3D = Marker3D.new()
		marker_3d.name = bone_name
		marker_3d.global_transform = node_3d.global_transform
		node_3d.replace_by(marker_3d, true)
		marker_3d.gizmo_extents = 0.1
#	enable_debug_pins(pins, ewbik)
var human_bones: PackedStringArray

func _run():
	# Check if bones roll point forward
	var profile : SkeletonProfileHumanoid = SkeletonProfileHumanoid.new()
	for bone_i in profile.bone_size:
		human_bones.push_back(profile.get_bone_name(bone_i)) 
	var root : Node3D = get_editor_interface().get_edited_scene_root()
	if root == null:
		return
	var skeletons : Array[Node] = root.find_children("*", "Skeleton3D")
	for skeleton in skeletons:
		var iks : Array[Node] = skeleton.find_children("*", "NBoneIK")
		for ik in iks:
			ik.free()
		var new_ik : NBoneIK = NBoneIK.new()
		skeleton.add_child(new_ik, true)
		new_ik.owner = root
		new_ik.set_pin_count(0)
		new_ik.set_constraint_count(human_bones.size())
		for constraint_i in range(human_bones.size()):
			var bone_name = human_bones[constraint_i]
			new_ik.set_constraint_name(constraint_i, bone_name)
			new_ik.set_kusudama_twist(constraint_i, Vector2(-PI, TAU - 0.001))
		create_pins(new_ik, skeleton)
