@tool
extends GutTest

var IKNode3D = preload("res://many_bone_ik/core/ik_node_3d.gd")

func test_transform_operations():
	## Instantiate IKNode3D
	var node = IKNode3D.new()

	## Test set_transform and get_transform
	var t = Transform3D()
	t.origin = Vector3(1, 2, 3)
	
	node.set_transform(t)
	assert_eq(node.get_transform(), t)

	## Test set_global_transform and get_global_transform
	var gt = Transform3D()
	gt.origin = Vector3(4, 5, 6)
	node.set_global_transform(gt)
	assert_eq(node.get_global_transform(), gt)


func test_scale_operations():
	## Instantiate IKNode3D
	var node = IKNode3D.new()

	## Test set_disable_scale and is_scale_disabled
	node.set_disable_scale(true)
	assert_true(node.is_scale_disabled())


func test_parent_operations():
	## Instantiate IKNode3D
	var node = IKNode3D.new()
	var parent = IKNode3D.new()

	## Test set_parent and get_parent
	node.set_parent(parent)
	assert_eq(node.get_parent(), parent)


func test_coordinate_transformations():
	## Instantiate IKNode3D
	var node = IKNode3D.new()

	## Test to_local and to_global
	var global = Vector3(1, 2, 3)
	var local = node.to_local(global)
	assert_eq(node.to_global(local), global)


func test_local_transform_calculation():
	## Instantiate IKNode3D
	var node = IKNode3D.new()

	var node_transform = Transform3D()
	node_transform.origin = Vector3(1.0, 2.0, 3.0) ## Translation by (1, 2, 3)
	node.set_global_transform(node_transform)

	var parent_node = IKNode3D.new()

	var parent_transform = Transform3D()
	parent_transform.origin = Vector3(4.0, 5.0, 6.0) ## Translation by (4, 5, 6)
	parent_node.set_global_transform(parent_transform)

	node.set_parent(parent_node)

	var expected_local_transform = parent_node.get_global_transform().affine_inverse() * node.get_global_transform()

	gut.p("Set parent global transform: %s" % parent_transform)
	gut.p("Got parent global transform: %s" % parent_node.get_global_transform())
	gut.p("Set node global transform: %s" % node_transform)
	gut.p("Got node global transform: %s" % node.get_global_transform())
	gut.p("Calculated local transform: %s" % expected_local_transform)

	assert_eq(node.get_transform(), expected_local_transform)
