var chirality : float = node_global_transform.basis.x.dot(node_global_transform.basis.y.cross(node_global_transform.basis.z))
print("%s %f" % [bone_name, chirality])
