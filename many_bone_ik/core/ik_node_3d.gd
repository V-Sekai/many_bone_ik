extends RefCounted
class_name IKNode3D

enum TransformDirty {
	DIRTY_NONE = 0,
	DIRTY_VECTORS = 1,
	DIRTY_LOCAL = 2,
	DIRTY_GLOBAL = 4
}

var global_transform : Transform3D
var local_transform: Transform3D
var rotation : Basis
var scale: Vector3 = Vector3(1, 1, 1)
var dirty: int = TransformDirty.DIRTY_NONE

var disable_scale: bool = false:
	set(value):
		disable_scale = value
	get:
		return disable_scale

var parent: IKNode3D = null:
	set(value):
		parent = value
	get:
		return parent

var children: Array = []

## Propagate transform changes to child nodes
func _propagate_transform_changed() -> void:
	var to_remove = []
	for child in children:
		if child == null:
			to_remove.append(child)
		else:
			child._propagate_transform_changed()
	for child in to_remove:
		children.erase(child)

## Update local transform
func _update_local_transform() -> void:
	local_transform.basis = rotation.scaled(scale) 
	
## Rotate local with global
func rotate_local_with_global(p_basis: Basis, p_propagate: bool = false) -> void:
	if parent == null:
		return
	var new_rot = parent.get_global_transform().basis
	local_transform.basis = new_rot * p_basis * new_rot.inverse() * local_transform.basis

## Set transform
func set_transform(p_transform: Transform3D) -> void:
	if local_transform != p_transform:
		local_transform = p_transform
		_propagate_transform_changed()


func set_global_transform(p_transform: Transform3D) -> void:
	var ik_node = parent as IKNode3D
	if ik_node != null:
		var parent_inverse = ik_node.get_global_transform().affine_inverse()
		local_transform = parent_inverse * p_transform
	else:
		local_transform = p_transform
	dirty |= TransformDirty.DIRTY_LOCAL | TransformDirty.DIRTY_GLOBAL


## Get transform
func get_transform() -> Transform3D:
	if dirty & TransformDirty.DIRTY_LOCAL:
		_update_local_transform()
	return local_transform

func get_global_transform() -> Transform3D:
	if dirty & TransformDirty.DIRTY_GLOBAL:
		if dirty & TransformDirty.DIRTY_LOCAL:
			_update_local_transform()
		var ik_node = parent as IKNode3D
		if ik_node != null:
			global_transform = ik_node.get_global_transform() * local_transform
		else:
			global_transform = local_transform
		if disable_scale:
			var scale = global_transform.basis.get_scale()
			global_transform.basis = global_transform.basis.orthonormalized()
			global_transform = global_transform.scaled(scale)
		dirty &= ~TransformDirty.DIRTY_GLOBAL
	return global_transform

## Set disable scale
func set_disable_scale(p_enabled: bool) -> void:
	disable_scale = p_enabled

## Check if scale is disabled
func is_scale_disabled() -> bool:
	return disable_scale

## Set parent node
func set_parent(p_parent: IKNode3D) -> void:
	if parent != null:
		parent.children.erase(self)
	parent = p_parent
	if parent != null:
		parent.children.append(self)
	_propagate_transform_changed()

## Get parent node
func get_parent() -> IKNode3D:
	return parent

## Convert global coordinates to local
func to_local(p_global: Vector3) -> Vector3:
	return get_global_transform().affine_inverse() * p_global

## Convert local coordinates to global
func to_global(p_local: Vector3) -> Vector3:
	return get_global_transform() * p_local

## Cleanup before deletion
func cleanup() -> void:
	for child in children:
		child.set_parent(null)
