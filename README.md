# IK Cyclic Mean Deviation Descent (CMDD)

## Effectors section
* name
* effector budget(total budget / per item budget)
* target node (Defaults to empty nodepath)
* target transform (Defaults to identity transform)

Can do both target node and target transform. If both target node and target transform exists both the offset transform from the node and the target transform are concatenated. If the node doesn't exist, use the target transform. If the node exists, use the node transform.

Clicking on an effector bone name will show:

* target transform
* target node
* effector budget

## Bone section
* name
* add effector
* remove effector
* twist (0 - 1)
* direction count (0)
* directions (0 - *)

Clicking on an effector bone name will show:

* add effector
* remove effector (hidden)
* twist min_twist_angle
* twist range
* direction count
    * direction radius
    * control point

## Global options

* budget for the entire solver (ms)
* max distance
* max change per iterations

## Skeleton
* bone name
* bone pose
* bone custom pose
* bone rest
* bone parent

## Notes

Transform orientating the kusudama wants the y axis to be point in the oppositing direction of most disallowed. Do not allow 360 rotation because of singularity.

Name it constraint axes. (Limiting Axes in Java code.)