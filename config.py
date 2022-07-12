def can_build(env, platform):
    return True


def configure(env):
    pass


def get_doc_classes():
    return [
        "EWBIK",
        "IKBone3D",
        "IKEffector3D",
        "IKBoneSegment",
        "IKEffectorTemplate",
        "IKKusudama",
    ]


def get_doc_path():
    return "doc_classes"
