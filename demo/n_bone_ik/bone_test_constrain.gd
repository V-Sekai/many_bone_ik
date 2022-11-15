@tool
extends EditorScript

var pins =  [
	"Bone_01",
	"Bone_03",
]

func create_pins(ewbik : NBoneIK, skeleton : Skeleton3D):	
	var root : Node3D = get_editor_interface().get_edited_scene_root()
	if root == null:
		return
	var profile : SkeletonProfileHumanoid = SkeletonProfileHumanoid.new()
	var bone_map : BoneMap = BoneMap.new()
	bone_map.profile = profile
	for pin in pins:
		var node = root.find_child(pin)
		if node != null:
			node.free()
	ewbik.set_pin_count(pins.size())
	ewbik.enable = false
	skeleton.reset_bone_poses()
	for pin_i in range(pins.size()):
		var pin = pins[pin_i]
		var bone_name = pin
		var bone_i = skeleton.find_bone(bone_name)
		if bone_i == -1:
			continue
		var node_3d : BoneAttachment3D = BoneAttachment3D.new()
		node_3d.name = bone_name
		node_3d.bone_name = bone_name
		node_3d.bone_idx = skeleton.find_bone(bone_name)
		node_3d.set_use_external_skeleton(true)
		node_3d.set_external_skeleton(str("../") + str(root.get_path_to(skeleton)))
		root.add_child(node_3d)
		node_3d.owner = root
		var path_string : String = str(ewbik.get_path_to(root)) + "/" + str(root.get_path_to(node_3d))
		ewbik.set_pin_nodepath(pin_i, NodePath(path_string))
		ewbik.set_pin_bone_name(pin_i, bone_name)
		ewbik.set_pin_depth_falloff(pin_i, 1)
		ewbik.set_pin_weight(pin_i, 0.2)
		if bone_name in ["Head"]:
			ewbik.set_pin_weight(pin_i, 1)
			ewbik.set_pin_depth_falloff(pin_i, 0)
		if bone_name in ["LeftFoot", "RightFoot"]:
			ewbik.set_pin_weight(pin_i, 0.4)
		var marker_3d : Marker3D = Marker3D.new()
		marker_3d.name = bone_name
		marker_3d.global_transform = node_3d.global_transform
		node_3d.replace_by(marker_3d, true)
		marker_3d.gizmo_extents = 0.5
	ewbik.enable = true
	
var bones = [
	"Bone_01",
	"Bone_02",
	"Bone_03",
]

func create_constraints(ewbik):
	ewbik.set_constraint_count(bones.size())
	for constraint_i in range(bones.size()):
		var bone_name = bones[constraint_i]
		ewbik.set_constraint_name(constraint_i, bone_name)
		ewbik.set_kusudama_limit_cone_count(constraint_i, 1)
		ewbik.set_kusudama_limit_cone_center(constraint_i, 0, Vector3(0, 1, 0))
		ewbik.set_kusudama_limit_cone_radius(constraint_i, 0, deg_to_rad(150))
		ewbik.set_kusudama_twist(constraint_i, Vector2(deg_to_rad(0), deg_to_rad(360)))
		
func _run():
	var root : Node3D = get_editor_interface().get_edited_scene_root()
	if root == null:
		return
	var skeletons : Array[Node] = root.find_children("*", "Skeleton3D")
	var skeleton : Skeleton3D = null
	for new_skeleton in skeletons:
		skeleton = new_skeleton
		break
	var iks : Array[Node] = root.find_children("*", "NBoneIK")
	var ik : NBoneIK = null
	for new_ik in iks:
		ik = new_ik
		break
	if skeleton == null:
		return
	if ik == null:
		ik = NBoneIK.new()
		root.add_child(ik, true)
		ik.owner = root
	ik.set_pin_count(0)
	ik.set_constraint_count(0)
	ik.skeleton_node_path = "../" + str(root.get_path_to(skeleton))
	ik.visible = true
	create_constraints(ik)
	create_pins(ik, skeleton)
	ik.queue_print_skeleton()
