def can_build(env, platform):
    return True


def configure(env):
    pass


def get_doc_classes():
    return [
        "SkeletonModification3DEWBIK",
        "IKBone3D",
        "IKEffector3D",
        "IKBoneSegment",
        "IKEffectorTemplate",
        "IKKusudama",
        "SkeletonModification3D"
    ]


def get_doc_path():
    return "doc_classes"
