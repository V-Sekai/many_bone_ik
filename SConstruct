#!/usr/bin/env python
import os
import sys

env = SConscript("thirdparty/godot-cpp/SConstruct")


sources = [Glob("*.cpp"), Glob("src/*.cpp"), Glob("src/math/*.cpp")]

if env["platform"] == "macos":
	library = env.SharedLibrary(
		"bin/addons/godot_many_bones_ik/bin/libgodot_many_bones_ik{}.framework/libgodot_many_bones_ik{}".format(
			env["suffix"], env["suffix"]
		),
		source=sources,
	)
elif env["platform"] == "ios":
	library = env.SharedLibrary(
		"bin/addons/godot_many_bones_ik/bin/libgodot_many_bones_ik{}.framework/libgodot_many_bones_ik{}".format(
			env["suffix"], env["suffix"]
		),
		source=sources,
	)
else:
	library = env.SharedLibrary(
		"bin/addons/godot_many_bones_ik/bin/libgodot_many_bones_ik{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
		source=sources,
	)
Default(library)
