def can_build(env, platform) :
	return True

def configure(env) :
	pass

def get_doc_classes():
    return [
        "SkeletonModification3DEWBIK",
        "IKBone3D",
		"IKEffector3D",
		"IKBoneChain",
    ]

def get_doc_path():
    return "doc_classes"
	