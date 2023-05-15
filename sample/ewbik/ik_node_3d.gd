@tool
extends RefCounted

class_name IKNode3D

enum TransformDirty {
	DIRTY_NONE = 0,
	DIRTY_VECTORS = 1,
	DIRTY_LOCAL = 2,
	DIRTY_GLOBAL = 4
}

var global_transform := Transform3D.IDENTITY
var local_transform := Transform3D.IDENTITY
var rotation := Basis()
var scale := Vector3(1, 1, 1)

var dirty := TransformDirty.DIRTY_NONE

var parent: IKNode3D
var children: Array = []

var disable_scale := false

func _update_local_transform() -> void:
	local_transform.basis = rotation.scaled(scale)
	dirty &= ~TransformDirty.DIRTY_LOCAL

func _propagate_transform_changed() -> void:
	for transform in children:
		if transform == null:
			children.erase(transform)
			continue
		transform._propagate_transform_changed()

	dirty |= TransformDirty.DIRTY_GLOBAL

func rotate_local_with_global(p_basis: Basis, p_propagate: bool = false) -> void:
	if parent == null:
		return
	var new_rot := parent.get_global_transform().basis
	local_transform.basis = (new_rot.inverse() * p_basis * new_rot) * local_transform.basis
	dirty |= TransformDirty.DIRTY_GLOBAL
	if p_propagate:
		_propagate_transform_changed()

func set_transform(p_transform: Transform3D) -> void:
	if local_transform == p_transform:
		return
	local_transform = p_transform
	dirty |= TransformDirty.DIRTY_VECTORS
	_propagate_transform_changed()

func set_global_transform(p_transform: Transform3D) -> void:
	var xform := p_transform
	if parent != null:
		xform = parent.get_global_transform().affine_inverse() * p_transform
	local_transform = xform
	dirty |= TransformDirty.DIRTY_VECTORS
	_propagate_transform_changed()

func get_transform() -> Transform3D:
	if dirty & TransformDirty.DIRTY_LOCAL:
		_update_local_transform()

	return local_transform

func get_global_transform() -> Transform3D:
	if dirty & TransformDirty.DIRTY_GLOBAL:
		if dirty & TransformDirty.DIRTY_LOCAL:
			_update_local_transform()

		if parent != null:
			global_transform = parent.get_global_transform() * local_transform
		else:
			global_transform = local_transform

		if disable_scale:
			global_transform.basis = global_transform.basis.orthogonalized()

		dirty &= ~TransformDirty.DIRTY_GLOBAL

	return global_transform

func set_disable_scale(p_enabled: bool) -> void:
	disable_scale = p_enabled

func is_scale_disabled() -> bool:
	return disable_scale

func set_parent(p_parent: IKNode3D) -> void:
	parent = p_parent
	if p_parent != null:
		parent.children.push_back(self)
	_propagate_transform_changed()

func get_parent() -> IKNode3D:
	return parent

func to_local(p_global: Vector3) -> Vector3:
	return get_global_transform().affine_inverse() * p_global

func to_global(p_local: Vector3) -> Vector3:
	return get_global_transform() * p_local
