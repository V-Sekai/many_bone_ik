@tool
extends Object

class_name IKBone

var parent_armature: AbstractArmature
var tag: String

var last_rotation: Quat
var previous_orientation: IKNode3D
var local_axes: IKNode3D
var major_rotation_axes: IKNode3D

var bone_height: float
var parent: IKBone

var children: Array = []
var free_children: Array = []
var effectored_children: Array = []

var constraints: IKKusudama
var pin: IKPin = null
var orientation_lock: bool = false
var stiffness_scalar: float = 0.0
var ancestor_count: int = 0

func _init(par: IKBone, tip_heading: Vector3, roll_heading: Vector3, input_tag: String, input_bone_height: float, coordinate_type: FrameType) -> void:
	last_rotation = Quat()

	if par != null:
		tag = str(hash(self)) if input_tag == null or input_tag == "" else input_tag
		bone_height = input_bone_height

		var tip_heading_ray: IKRay3D = IKRay3D.new(par.get_tip(), tip_heading)
		var roll_heading_ray: IKRay3D = IKRay3D.new(par.get_tip(), roll_heading)
		var temp_tip: Vector3 = tip_heading.copy()
		var temp_roll: Vector3 = roll_heading.copy()
		var temp_x: Vector3 = temp_roll.copy()

		if coordinate_type == FrameType.GLOBAL:
			temp_tip = tip_heading_ray.heading()
			temp_roll = roll_heading_ray.heading()
		elif coordinate_type == FrameType.RELATIVE:
			temp_tip = par.local_axes.get_global_of(tip_heading_ray.heading())
			temp_roll = par.local_axes.get_global_of(roll_heading_ray.heading())
		else:
			print("WOAH WOAH WOAH")

		temp_x = temp_tip.cross(temp_roll)
		temp_roll = temp_x.cross(temp_tip)

		temp_x.normalize()
		temp_tip.normalize()
		temp_roll.normalize()

		self.parent = par
		self.parent_armature = self.parent.parent_armature
		parent_armature.add_to_bone_list(self)

		generate_axes(parent.get_tip(), temp_x, temp_tip, temp_roll)
		local_axes.set_parent(parent.local_axes)

		previous_orientation = local_axes.attached_copy(true)

		major_rotation_axes = parent.local_axes.get_global_copy()
		major_rotation_axes.translate_to(parent.get_tip())
		major_rotation_axes.set_parent(parent.local_axes)

		self.parent.add_free_child(self)
		self.parent.add_child(self)
		self.update_ancestor_count()
	else:
		raise NullParentForBoneException.new()


private void updateAncestorCount() {
	int countedAncestors = 0;
	IKBone currentBone = this.parent;
	while (currentBone != null) {
		countedAncestors++;
		currentBone = currentBone.parent;
	}
	setAncestorCount(countedAncestors);
}

/**
	* updates the ancestor count for this bone, and
	* sets the ancestor count of all child bones
	* to this bone's ancestor count +1;
	* 
	* @param count
	*/
private void setAncestorCount(int count) {
	this.ancestorCount = count;
	for (IKBone b : this.children) {
		b.setAncestorCount(this.ancestorCount + 1);
	}
}

public IKBone(IKBone par,
		Vector3 tipHeading,
		Vector3 rollHeading,
		String inputTag,
		frameType coordinateType)
		throws NullParentForBoneException {

	this.lastRotation = new Quaternion();
	if (par != null) {
		if (inputTag == null || inputTag == "") {
			this.tag = Integer.toString(System.identityHashCode(this));
		} else
			this.tag = inputTag;

		IKRay3D tipHeadingRay = new IKRay3D(par.getTip_(), tipHeading);
		IKRay3D rollHeadingRay = new IKRay3D(par.getTip_(), rollHeading);
		Vector3 tempTip = tipHeading.copy();
		Vector3 tempRoll = rollHeading.copy();
		Vector3 tempX = new Vector3();

		if (coordinateType == frameType.GLOBAL) {
			tempTip = tipHeadingRay.heading();
			tempRoll = rollHeadingRay.heading();
		} else if (coordinateType == frameType.RELATIVE) {
			tempTip = par.localAxes().getGlobalOf(tipHeadingRay.heading());
			tempRoll = par.localAxes().getGlobalOf(rollHeadingRay.heading());
		} else {
			System.out.println("WOAH WOAH WOAH");
		}

		tempX = tempTip.crossCopy(tempRoll);
		tempRoll = tempX.crossCopy(tempTip);

		tempX.normalize();
		tempTip.normalize();
		tempRoll.normalize();

		this.boneHeight = tipHeadingRay.mag();
		this.parent = par;
		this.parentArmature = this.parent.parentArmature;
		parentArmature.addToBoneList(this);

		generateAxes(parent.getTip_(), tempX, tempTip, tempRoll);

		localAxes.setParent(parent.localAxes);
		previousOrientation = localAxes.attachedCopy(true);

		majorRotationAxes = parent.localAxes().getGlobalCopy();
		majorRotationAxes.translateTo(parent.getTip_());
		majorRotationAxes.setParent(parent.localAxes);

		this.parent.addChild(this);
		this.parent.addFreeChild(this);
		this.updateAncestorCount();
	} else {
		throw new NullParentForBoneException();
	}
}

public IKBone(
		AbstractArmature parArma,
		Vector3 tipHeading,
		Vector3 rollHeading,
		String inputTag,
		double inputBoneHeight,
		frameType coordinateType)
		throws NullParentForBoneException {

	this.lastRotation = new Quaternion();
	if (parArma != null) {
		if (inputTag == null || inputTag == "") {
			this.tag = Integer.toString(System.identityHashCode(this));
		} else
			this.tag = inputTag;

		IKRay3D tipHeadingRay = new IKRay3D(parArma.localAxes.origin_(), tipHeading);
		tipHeadingRay.getRayScaledTo(inputBoneHeight);
		IKRay3D rollHeadingRay = new IKRay3D(parArma.localAxes.origin_(), rollHeading);
		Vector3 tempTip = tipHeading.copy();
		Vector3 tempRoll = rollHeading.copy();
		Vector3 tempX = tempTip.copy();

		if (coordinateType == frameType.GLOBAL) {
			tempTip = tipHeadingRay.heading();
			tempRoll = rollHeadingRay.heading();
		} else if (coordinateType == frameType.RELATIVE) {
			tempTip = parArma.localAxes.getGlobalOf(tipHeadingRay.heading());
			tempRoll = parArma.localAxes.getGlobalOf(rollHeadingRay.heading());
		} else {
			System.out.println("WOAH WOAH WOAH");
		}

		tempX = tempTip.crossCopy(tempRoll);
		tempRoll = tempX.crossCopy(tempTip);

		tempX.normalize();
		tempTip.normalize();
		tempRoll.normalize();
		this.parentArmature = parArma;
		generateAxes(parentArmature.localAxes.origin_(), tempX, tempTip, tempRoll);
		localAxes.setParent(parentArmature.localAxes);
		majorRotationAxes = parentArmature.localAxes().getGlobalCopy();
		majorRotationAxes.setParent(parentArmature.localAxes());
		parentArmature.addToBoneList(this);
		previousOrientation = localAxes.attachedCopy(true);
		this.boneHeight = inputBoneHeight;
		this.updateAncestorCount();
	} else {
		throw new NullParentForBoneException();
	}

}

public IKBone(AbstractArmature parArma,
		Vector3 tipHeading,
		Vector3 rollHeading,
		String inputTag,
		frameType coordinateType)
		throws NullParentForBoneException {

	this.lastRotation = new Quaternion();
	if (parArma != null) {
		if (inputTag == null || inputTag == "") {
			this.tag = Integer.toString(System.identityHashCode(this));
		} else
			this.tag = inputTag;

		IKRay3D tipHeadingRay = new IKRay3D(parArma.localAxes.origin_(), tipHeading);
		IKRay3D rollHeadingRay = new IKRay3D(parArma.localAxes.origin_(), rollHeading);
		Vector3 tempTip = tipHeading.copy();
		Vector3 tempRoll = rollHeading.copy();
		Vector3 tempX = rollHeading.copy();

		if (coordinateType == frameType.GLOBAL) {
			tempTip = tipHeadingRay.heading();
			tempRoll = rollHeadingRay.heading();
		} else if (coordinateType == frameType.RELATIVE) {
			tempTip = parArma.localAxes.getGlobalOf(tipHeadingRay.heading());
			tempRoll = parArma.localAxes.getGlobalOf(rollHeadingRay.heading());
		} else {
			System.out.println("WOAH WOAH WOAH");
		}

		tempX = tempTip.crossCopy(tempRoll);
		tempRoll = tempX.crossCopy(tempTip);

		tempX.normalize();
		tempTip.normalize();
		tempRoll.normalize();

		this.boneHeight = tipHeading.mag();
		this.parentArmature = parArma;

		generateAxes(parentArmature.localAxes.origin_(), tempX, tempTip, tempRoll);
		localAxes.setParent(parentArmature.localAxes);
		previousOrientation = localAxes.attachedCopy(true);

		majorRotationAxes = parentArmature.localAxes().getGlobalCopy();
		majorRotationAxes.setParent(parent.localAxes);

		parentArmature.addToBoneList(this);
		this.updateAncestorCount();
		this.updateSegmentedArmature();

	} else {
		throw new NullParentForBoneException();
	}

}

public IKBone() {
	this.lastRotation = new Quaternion();
}

public IKBone getParent() {
	return this.parent;
}

public void attachToParent(IKBone inputParent) {
	inputParent.addChild(this);
	this.parent = inputParent;
	this.updateAncestorCount();
}

public void solveIKFromHere() {
	this.parentArmature.IKSolver(this);
}

public void setAxesToReturnfulled(IKNode3D toSet, IKNode3D swingAxes, IKNode3D twistAxes,
		double cosHalfAngleDampen, double angleDampen) {
	if (constraints != null && IKKusudama.class.isAssignableFrom(constraints.getClass())) {
		((IKKusudama) constraints).setAxesToReturnfulled(toSet, swingAxes, twistAxes, cosHalfAngleDampen,
				angleDampen);
	}
}

public void setPin_(Vector3 pin) {
	if (this.pin == null) {
		this.enablePin_(pin);
	} else {
		this.pin.translateTo_(pin);
	}
}

/**
	* @param newConstraint a constraint Object to add to this bone
	* @return the constraintObject that was just added
	*/
public IKKusudama addConstraint(IKKusudama newConstraint) {
	constraints = newConstraint;
	this.parentArmature.regenerateShadowSkeleton();
	return constraints;
}

/**
	* 
	* @return this bone's constraint object.
	*/
public IKKusudama getConstraint() {
	return constraints;
}

/**
	* Rotate the bone about its frame of reference by a custom Apache Commons
	* Rotation object
	* 
	* @param rot
	*/
public void rotateBy(Quaternion rot) {
	this.previousOrientation.alignLocalsTo(localAxes);
	this.localAxes.rotateBy(rot);

	this.lastRotation.set(rot);
}

/**
	* @param rotationFrameCoordinates the Axes around which rotAboutFrameX,
	*                                 rotAboutFrameY, and rotAboutFrameZ will
	*                                 rotate,
	*                                 and against which getXZYAngle() will be
	*                                 computed.
	*                                 The input is expected to be in RELATIVE
	*                                 coordinates.
	*                                 so specifying these axes as having an <br>
	*                                 x component heading of (1,0,0) and <br>
	*                                 y component heading of (0,1,0) and<br>
	*                                 z component heading of (0,0,1) <br>
	*                                 <br>
	*                                 is equivalent to specifying the
	*                                 frameOfRotation of this bone as being
	*                                 perfectly aligned
	*                                 with the localAxes of its parent bone.<br>
	*                                 <br>
	* 
	*                                 The physical intuition of this is maybe
	*                                 something like
	*                                 "at what angle did I place the servos on this
	*                                 joint"<br>
	*                                 <br>
	* 
	*                                 It doesn't necessarily determine where the
	*                                 bone can rotate, but it does determine *how*
	*                                 the bone would rotate to get there.<br>
	*                                 <br>
	* 
	*                                 This is also used to set constraints. For
	*                                 example, euler constraints are computed
	*                                 against these axes
	*                                 and the limitCones and axial twist limits of
	*                                 KusudamaExample are specified relative to
	*                                 these Axes.<br>
	*                                 <br>
	* 
	*                                 Changing these axes is essentially the
	*                                 equivalent of rotating the joint on which
	*                                 this bone rests,
	*                                 while keeping the bone in globally in
	*                                 place.<br>
	*                                 <br>
	* 
	*                                 You don't need to change this unless you
	*                                 start wishing you could change this.
	*/
public void setFrameofRotation(IKNode3D rotationFrameCoordinates) {
	majorRotationAxes.alignLocalsTo(rotationFrameCoordinates);
	if (parent != null) {
		majorRotationAxes.translateTo(parent.getTip_());
	}
}

/**
	* Get the Axes relative to which this bone's rotations are defined. (If the
	* bone has constraints, this will be the
	* constraint Axes)
	* 
	* @return
	*/
public IKNode3D getMajorRotationAxes() {
	return this.majorRotationAxes;
}

/**
	* Disables the pin for this bone so that it no longer interests the IK Solver.
	* However, all information abut the pin is maintained, so the pin can be turned
	* on again with enablePin().
	*/
public void disablePin() {
	pin.disable();
	if (this.effectoredChildren.size() == 0) {
		notifyAncestorsOfUnpin();
	}
	this.updateSegmentedArmature();
}

/**
	* Entirely removes the pin from this bone. Any child pins attached to it are
	* reparented to this
	* pin's parent.
	*/
public void removePin() {
	pin.disable();
	if (this.effectoredChildren.size() == 0) {
		notifyAncestorsOfUnpin();
	}
	pin.removalNotification();
	this.updateSegmentedArmature();
}

/**
	* Enables an existing pin for this BoneExample. Or creates a pin for this bone
	* at the bone's tip.
	*/
public void enablePin() {
	// System.out.println("pinning");
	if (pin == null) {
		IKNode3D pinAxes = this.localAxes().getGlobalCopy();
		pinAxes.setParent(this.parentArmature.localAxes().getParentAxes());
		pin = createAndReturnPinOnAxes(pinAxes);
	}
	pin.enable();
	// System.out.println("clearing children");
	freeChildren.clear();
	// System.out.println("adding children");
	for (IKBone child : getChildren()) {
		if (child.pin != null && !child.pin.isEnabled()) {
			addFreeChild(child);
			// System.out.println("childAdd");
		}
	}
	// System.out.println("notifying ancestors");
	notifyAncestorsOfPin(false);
	// System.out.println("updating segment armature");
	this.updateSegmentedArmature();
	// System.out.println("segment armature updated");
}

/**
	* Creates a pin for this bone
	* 
	* @param pinTo the position of the pin in the coordinateFrame of the
	*              parentArmature.
	*/

public void enablePin_(Vector3 pinTo) {
	if (pin == null) {
		IKNode3D pinAxes = this.localAxes().getGlobalCopy();
		pinAxes.setParent(this.parentArmature.localAxes().getParentAxes());
		pinAxes.translateTo(pinTo);
		pin = createAndReturnPinOnAxes(pinAxes);
	} else
		pin.translateTo_(pinTo);
	pin.enable();
	freeChildren.clear();
	for (IKBone child : getChildren()) {
		if (!child.pin.isEnabled()) {
			addFreeChild(child);
		}
	}
	notifyAncestorsOfPin();
	// this.updateSegmentedArmature();
}

/**
	* @return true if the bone has a pin enabled, false otherwise.
	*/
public boolean isPinned() {
	if (pin == null || !pin.isEnabled()) {
		return false;
	} else {
		return true;
	}

}

/**
	* Creates / enables a pin if there no pin is active, disables the pin if it is
	* active.
	*/
public void togglePin() {
	if (this.pin == null)
		this.enablePin();
	this.pin.toggle();
	updateSegmentedArmature();
}

public ArrayList<IKBone> returnChildrenWithPinnedDescendants() {
	ArrayList<IKBone> childrenWithPinned = new ArrayList<IKBone>();
	for (IKBone c : getChildren()) {
		if (c.hasPinnedDescendant())
			childrenWithPinned.add(c);
	}
	return childrenWithPinned;
}

public ArrayList<? extends IKBone> getMostImmediatelyPinnedDescendants() {
	ArrayList<IKBone> mostImmediatePinnedDescendants = new ArrayList<IKBone>();
	this.addSelfIfPinned(mostImmediatePinnedDescendants);
	return mostImmediatePinnedDescendants;
}

public IKNode3D getPinnedAxes() {
	if (this.pin == null)
		return null;
	return this.pin.getAxes();
}

public void addSelfIfPinned(ArrayList<IKBone> pinnedBones2) {
	if (this.isPinned()) {
		pinnedBones2.add(this);
	} else {
		for (IKBone child : getChildren()) {
			child.addSelfIfPinned(pinnedBones2);
		}
	}
}

void notifyAncestorsOfPin(boolean updateSegments) {
	if (this.parent != null) {
		parent.addToEffectored(this);
	}
	if (updateSegments)
		parentArmature.regenerateShadowSkeleton();
}

public void notifyAncestorsOfPin() {
	notifyAncestorsOfPin(true);
}

public void notifyAncestorsOfUnpin() {
	if (this.parent != null) {
		parent.removeFromEffectored(this);
	}
	parentArmature.regenerateShadowSkeleton();
}

public void addToEffectored(IKBone abstractBone) {
	int freeIndex = freeChildren.indexOf(abstractBone);
	if (freeIndex != -1)
		freeChildren.remove(freeIndex);

	if (effectoredChildren.contains(abstractBone)) {
	} else {
		effectoredChildren.add(abstractBone);
	}
	if (this.parent != null) {
		parent.addToEffectored(this);
	}
}

public void removeFromEffectored(IKBone abstractBone) {
	int effectoredIndex = effectoredChildren.indexOf(abstractBone);
	if (effectoredIndex != -1)
		effectoredChildren.remove(effectoredIndex);

	if (freeChildren.contains(abstractBone)) {
	} else {
		addFreeChild(abstractBone);
	}
	if (this.parent != null && this.effectoredChildren.size() == 0 && this.pin != null && this.pin.isEnabled()) {
		parent.removeFromEffectored(this);
	}
}

public IKBone getPinnedRootBone() {
	IKBone rootBone = this;
	while (rootBone.parent != null && !rootBone.parent.pin.isEnabled()) {
		rootBone = rootBone.parent;
	}
	return rootBone;
}

public void updateSegmentedArmature() {
	this.parentArmature.regenerateShadowSkeleton();
}

public void setTag(String newTag) {
	parentArmature.updateBoneTag(this, this.tag, newTag);
	this.tag = newTag;
}

public String getTag() {
	return this.tag;
}

public Vector3 getBase_() {
	return localAxes.origin_().copy();
}

public Vector3 getTip_() {
	return localAxes.y_().getScaledTo(boneHeight);
}

public void setBoneHeight(double inBoneHeight) {
	this.boneHeight = inBoneHeight;
	for (IKBone child : this.getChildren()) {
		child.localAxes().translateTo(this.getTip_());
		child.majorRotationAxes.translateTo(this.getTip_());
	}
}

/**
	* removes this BoneExample and any of its children from the armature.
	*/
public void deleteBone() {
	ArrayList<IKBone> bones = new ArrayList<>();
	IKBone root = parentArmature.rootBone;
	root.hasChild(bones, this);
	for (IKBone p : bones) {
		System.out.println("removing from" + p);
		p.removeFromEffectored(this);
		for (IKBone ab : this.effectoredChildren) {
			p.removeFromEffectored(ab);
		}
		p.getChildren().remove(this);
		p.freeChildren.remove(this);
	}
	this.parentArmature.removeFromBoneList(this);
}

/* adds this bone to the arrayList if inputBone is among its children */
private void hasChild(ArrayList<IKBone> list, IKBone query) {
	if (getChildren().contains(query))
		list.add(this);
	for (IKBone c : getChildren()) {
		c.hasChild(list, query);
	}
}

public IKNode3D localAxes() {
	return this.localAxes;
}

public double getBoneHeight() {
	return this.boneHeight;
}

public boolean hasPinnedDescendant() {
	if (this.isPinned())
		return true;
	else {
		boolean result = false;
		for (IKBone c : getChildren()) {
			if (c.hasPinnedDescendant()) {
				result = true;
				break;
			}
		}
		return result;
	}

}

public IKPin getIKPin() {
	return this.pin;
}

/**
	* if set to true, the IK system will not rotate this bone
	* as it solves the IK chain.
	* 
	* @param val
	*/
public void setIKOrientationLock(boolean val) {
	this.orientationLock = val;
	this.parentArmature.updateShadowSkelRateInfo();
}

public boolean getIKOrientationLock() {
	return this.orientationLock;
}

public void addChild(IKBone bone) {
	if (this.getChildren().indexOf(bone) == -1) {
		((ArrayList<IKBone>) getChildren()).add(bone);
	}
}

public void addFreeChild(IKBone bone) {
	if (this.freeChildren.indexOf(bone) == -1) {
		freeChildren.add(bone);
	}
}

public void addEffectoredChild(IKBone bone) {
	if (this.effectoredChildren.indexOf(bone) == -1) {
		this.effectoredChildren.add(bone);
	}
	parentArmature.regenerateShadowSkeleton();
}

public void addDescendantsToArmature() {
	for (IKBone b : getChildren()) {
		parentArmature.addToBoneList(b);
		b.addDescendantsToArmature();
	}
}

public ArrayList<? extends IKBone> getChildren() {
	return children;
}

public void setChildren(ArrayList<? extends IKBone> children) {
	this.children = (ArrayList<IKBone>) children;
}

/**
	* The stiffness of a bone determines how much the IK solver should
	* prefer to avoid rotating it if it can. A value of 0 means the solver will
	* rotate this bone as much as the overall dampening parameter will
	* allow it to per iteration. A value of 0.5 means the solver will
	* rotate it half as much as the dampening parameter will allow,
	* and a value of 1 effectively means the solver is not allowed
	* to rotate this bone at all.
	* 
	* @return a value between 1 and 0.
	*/
public double getStiffness() {
	if (this.getIKOrientationLock())
		return 1d;
	return stiffnessScalar;
}

/**
	* The stiffness of a bone determines how much the IK solver should
	* prefer to avoid rotating it if it can. A value of 0 means the solver will
	* rotate this bone as much as the overall dampening parameter will
	* allow it to per iteration. A value of 0.5 means the solver will
	* rotate it half as much as the dampening parameter will allow,
	* and a value of 1 effectively means the solver is not allowed
	* to rotate this bone at all.
	*/
public void setStiffness(double stiffness) {
	stiffnessScalar = stiffness;
	if (parentArmature != null) {
		parentArmature.updateShadowSkelRateInfo();
	}
}

@Override
public void notifyOfLoadCompletion() {
	this.setBoneHeight(boneHeight);
	for (IKBone b : this.children) {
		b.attachToParent(this);
	}
	this.parentArmature.addToBoneList(this);
}

@Override
public int compareTo(IKBone i) {
	return this.ancestorCount - i.ancestorCount;
}

public String print() {
	return this.getTag();
}
