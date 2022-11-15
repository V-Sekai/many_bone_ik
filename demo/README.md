# README.md

N Bone IK is a full skeleton inverse kinematic solver.

This is the demo project for N Bone IK.

## Godot Engine editor settings for Windows 11

Use godot with an external editor.

Point to the Visual Studio code `example_absolute_path/code.cmd` absolute path  and use `{project} --goto {file}:{line}:{col}` as the exec setting.

Change the language server port to `6008`.

## Visual Studio Code

Install Visual Studio Code.

Install gut extension.

## Explosions

Do you know what the physical quanity where the larger the bone the less movement it can be pulled by muscle?

because currently larger bones can move more and they do

You mean the IRL physical quantity or the parameter a user should change in ewbik?

in the can't reach case it explodes

The exploding is likely largely a bug in your implementation energy conserving is the general idea but not sure.

You should fix it instead of trying to work around it.

You're probably thinking of angular momentum and/or torque.

In terms of making bones less likely to move in ewbik, you would lower their dampening parameter.

But the human body doesn't care about this that much. Your legs are much larger than any of the bones in your spine, but move around much more
Anyway, the situations where you will encounter actual non-convergence are pretty rare, and the way to handle them is by ensuring that an iteration after each constraint decreases target deviation, and skipping the iteration if it doesn't
