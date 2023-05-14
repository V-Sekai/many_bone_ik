@tool
extends Object

class_name ShadowSkeleton

var for_armature: AbstractArmature
var sim_transforms: Array
var shadow_space: Transform3D
var traversal_array: Array
var root_segment: IKArmatureSegment
var skel_state: SkeletonState

func _init(skel_state: SkeletonState, for_armature: AbstractArmature):
	self.for_armature = for_armature
	self.skel_state = skel_state

	# Build simTransforms hierarchy
	var transform_count = skel_state.get_transform_count()
	if transform_count != 0:
		sim_transforms = []
		shadow_space = for_armature.local_axes().free_copy()
		shadow_space = Transform3D()

		for i in range(transform_count):
			var ts = skel_state.get_transform_state(i)
			var new_transform: Transform3D = shadow_space
			new_transform = Transform3D(ts.rotation.scaled(ts.scale), ts.translation)
			sim_transforms.append(new_transform)

		for i in range(transform_count):
			var ts = skel_state.get_transform_state(i)
			var par_ts_idx = ts.get_parent_index()
			var sim_t = sim_transforms[i]
			if par_ts_idx == -1:
				sim_t.set_relative_to_parent(shadow_space)
			else:
				sim_t.set_relative_to_parent(sim_transforms[par_ts_idx])

	# Build armature segment hierarchy
	var root_bone = skel_state.get_root_bone_state()
	if root_bone != null:
		root_segment = IKArmatureSegment.new(self, root_bone, null, false)

	# Build traversal array
	if root_segment != null:
		var segment_traversal_array = root_segment.get_descendant_segments()
		var reversed_traversal_array = []
		for segment in segment_traversal_array:
			reversed_traversal_array.extend(segment.reversed_traversal_array)
		traversal_array = []
		for i in range(reversed_traversal_array.size() - 1, -1, -1):
			traversal_array.append(reversed_traversal_array[i])

func solve(dampening: float, iterations: int, stabilization_passes: int, notifier: Callable) -> void:
	var end_on_index = traversal_array.size() - 1
	if iterations == -1:
		iterations = for_armature.get_default_iterations()
	if stabilization_passes == -1:
		stabilization_passes = for_armature.default_stabilizing_pass_count
	var transforms = skel_state.get_transforms_array()
	for i in range(transforms.size()):
		sim_transforms[i].get_local_m_basis().set(transforms[i].translation, transforms[i].rotation,
													transforms[i].scale)
		sim_transforms[i]._exclusive_mark_dirty()
	for i in range(iterations):
		for j in range(end_on_index + 1):
			traversal_array[j].pull_back_toward_allowable_region(i, iterations)
		for j in range(end_on_index + 1):
			traversal_array[j].fast_update_optimal_rotation_to_pinned_descendants(dampening,
																					j == end_on_index and end_on_index == traversal_array.size() - 1)
	for wb in traversal_array:
		align_bone(wb)

func update_rates() -> void:
	for wb in traversal_array:
		wb.update_cos_dampening()

func align_bone(wb: WorkingBone) -> void:
	var bs = wb.for_bone
	var ts = bs.get_transform()
	ts.translation = wb.sim_local_axes.local_m_basis.translate.get()
	ts.rotation = wb.sim_local_axes.local_m_basis.rotation.to_array()

private boolean dirtySkelState = false;
private boolean dirtyRate = false;
SkeletonState skelState;

/**
	* a list of the bones used by the solver, in the same order they appear in the skelState after validation.
	* This is to very quickly update the scene with the solver's results, without incurring hashmap lookup penalty. 
	***/
protected AbstractBone[] skelStateBoneList = new AbstractBone[0];

ShadowSkeleton shadowSkel;
private void _regenerateShadowSkeleton() {
	skelState = new SkeletonState();
	for(AbstractBone b: bones) {
		registerBoneWithShadowSkeleton(b);
	}
	skelState.validate();
	shadowSkel = new ShadowSkeleton(skelState, this);
	skelStateBoneList = new AbstractBone[skelState.getBoneCount()];
	for(int i=0; i<bones.size(); i++) {
		BoneState bonestate = skelState.getBoneStateById(bones.get(i).getIdentityHash());
		if(bonestate != null)
			skelStateBoneList[bonestate.getIndex()] = bones.get(i);
	}
	dirtySkelState = false;
}


/**
	* This method should be called whenever a structural change has been made to the armature prior to calling the solver.
	* A structural change is basically any change other than a rotation/translation/scale of a bone or a target. 
	* Structural changes include things like, 
	*		1. reparenting / adding / removing bones. 
	* 	2. marking a bone as an effector (aka "pinning / unpinning a bone")
	* 	3. adding / removing a constraint on a bone.
	* 	4. modifying a pin's fallOff to non-zero if it was zero, or zero if it was non-zero
	* 
	* You should NOT call this function if you have only modified a translation/rotation/scale of some transform on the armature
	* 
	* For skeletal modifications that are likely to effect the solver behavior but do not fall 
	* under any of the above (generally things like changing bone stiffness, depth falloff, targetweight, etc) to intermediary values, 
	* you should (but don't have to) call updateShadowSkelRateInfo() for maximum efficiency.
	*/
public void regenerateShadowSkeleton() {
	this.regenerateShadowSkeleton(false);
}

/**
	* @param force by default, calling this function sets a flag notifying the solver that it needs to regenerate the shadow skeleton before
	* attempting a solve. If you set this to "true", the shadow skeleton will be regenerated immediately. 
	* (useful if you do solves in a separate thread from structure updates)
	*/
public void regenerateShadowSkeleton(boolean force) {
	dirtySkelState = true;
	if(force) 
		this._regenerateShadowSkeleton();
	dirtyRate = true;
}

public void updateShadowSkelRateInfo() {
	dirtyRate = true;
}

private void _updateShadowSkelRateInfo() {
	BoneState[] bonestates = skelState.getBonesArray();
	for(int i=0; i<skelStateBoneList.length; i++) {
		AbstractBone b = skelStateBoneList[i];
		BoneState bs = bonestates[i];
		bs.setStiffness(b.getStiffness());
	}
}

private void registerBoneWithShadowSkeleton(AbstractBone bone) { 
	String parBoneId = (bone.getParent() == null) ? null : bone.getParent().getIdentityHash(); 
	Constraint constraint = bone.getConstraint();
	String constraintId = (constraint == null) ? null : constraint.getIdentityHash(); 
	AbstractIKPin target = bone.getIKPin();
	String targetId = (target == null || target.getPinWeight() == 0 || target.isEnabled() == false) ? null : target.getIdentityHash();
	skelState.addBone(
			bone.getIdentityHash(), 
			bone.localAxes().getIdentityHash(), 
			parBoneId, 
			constraintId, 
			bone.getStiffness(),
			targetId);
	registerAxesWithShadowSkeleton(bone.localAxes(), bone.getParent() == null);
	if(targetId != null) registerTargetWithShadowSkeleton(target);
	if(constraintId != null) registerConstraintWithShadowSkeleton(constraint);
	
}
private void registerTargetWithShadowSkeleton(AbstractIKPin ikPin) {
	skelState.addTarget(ikPin.getIdentityHash(), 
			ikPin.getAxes().getIdentityHash(), 
			ikPin.forBone().getIdentityHash(),
			new double[] {ikPin.getXPriority(), ikPin.getYPriority(), ikPin.getZPriority()}, 
			ikPin.getDepthFalloff(),
			ikPin.getPinWeight());
	registerAxesWithShadowSkeleton(ikPin.getAxes(), true);
}
private void registerConstraintWithShadowSkeleton(Constraint constraint) {
	AbstractAxes twistAxes = constraint.twistOrientationAxes() == null ? null : constraint.twistOrientationAxes();
	skelState.addConstraint(
			constraint.getIdentityHash(),
			constraint.attachedTo().getIdentityHash(),
			constraint.swingOrientationAxes().getIdentityHash(),
			twistAxes == null ? null : twistAxes.getIdentityHash(),
			constraint);
	registerAxesWithShadowSkeleton(constraint.swingOrientationAxes(), false);
	if(twistAxes != null)
		registerAxesWithShadowSkeleton(twistAxes, false);
	
}

/**
	* @param axes
	* @param rebase if true, this function will not provide a parent_id for these axes.
	* This is mostly usefu l for ensuring that targetAxes are always implicitly defined in skeleton space when calling the solver.
	* You should always set this to true when giving the axes of an IKPin, as well as when giving the axes of the root bone. 
	* see the skelState.addTransform documentation for more info. 
	*/
private void registerAxesWithShadowSkeleton(AbstractAxes axes, boolean unparent) {
	String parent_id  = unparent || axes.getParentAxes() == null ? null : axes.getParentAxes().getIdentityHash();
	AbstractBasis basis = getSkelStateRelativeBasis(axes, unparent);
	Vec3d<?> translate = basis.translate;
	Rot rotation =basis.rotation;
	skelState.addTransform(
			axes.getIdentityHash(), 
			new double[]{translate.getX(), translate.getY(), translate.getZ()}, 
			rotation.toArray(), 
			new double[]{1.0,1.0,1.0}, 
			parent_id, axes);
}

/**
	* @param axes
	* @param unparent if true, will return a COPY of the basis in Armature space, otherwise, will return a reference to axes.localMBasis
	* @return
	*/
private AbstractBasis getSkelStateRelativeBasis(AbstractAxes axes, boolean unparent) {
	AbstractBasis basis = axes.getLocalMBasis(); 
	if(unparent) {
		basis = basis.copy();
		this.localAxes().getGlobalMBasis().setToLocalOf(axes.getGlobalMBasis(), basis);
	}
	return basis;
}