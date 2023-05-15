@tool
extends Object

class_name BoneState

func _init(id, transform_id, parent_id, target_id, constraint_id, stiffness):
	pass

String id = null;
String parent_id = null;
String transform_id = null;
String target_id = null;
String constraint_id = null;
private double stiffness = 0.0;
private HashMap<String, Integer> childMap = new HashMap<>();
private int index;
private int parentIdx = -1;
private int[] childIndices = null;
private int transformIdx = -1;
private int constraintIdx = -1;
private int targetIdx = -1;

private BoneState(String id, String transform_id, String parent_id, String target_id, String constraint_id,  double stiffness) {
	this.id = id;
	this.parent_id = parent_id;
	this.transform_id  = transform_id;
	this.target_id = target_id; 
	this.constraint_id = constraint_id; 
	this.stiffness = stiffness;
}		
public TransformState getTransform() {
	return transforms[this.transformIdx];
}
public TargetState getTarget() {
	if(this.targetIdx ==-1) return null;
	else return targets[this.targetIdx];
}
public double getStiffness() {
	return stiffness;
}
public BoneState getParent() {
	if(this.parentIdx >= 0)
		return bones[this.parentIdx];
	return null;
}
public BoneState getChild(String id) {
	return bones[this.childMap.get(id)];
}
public BoneState getChild(int index) {
	return bones[this.childIndices[index]];
}
public void clearChildList() {
	this.childMap.clear();
	this.childIndices = new int[] {};
}
public int getChildCount() {
	return childIndices.length;
}

private int getTempChildCount() {
	return this.childMap.values().size();
}
public ConstraintState getConstraint() {
	if(this.constraintIdx >= 0)
		return constraints[this.constraintIdx];
	return null;
}

private void prune() {
	bonesList.set(this.index, null);
	boneMap.remove(this.id);
	this.getTransform().prune();
	if(this.getConstraint() != null) this.getConstraint().prune();
	if(this.parent_id != null) {
		boneMap.get(this.parent_id).childMap.remove(this.id);
	}
	if(rootBoneState == this)
		rootBoneState = null;
}		
private void setIndex(int index) {
	this.index = index;
	if(this.parent_id != null) {
		BoneState parentBone = boneMap.get(this.parent_id);
		parentBone.addChild(this.id, this.index);
	}
}
private void  addChild(String id, int childIndex) {
	this.childMap.put(id, childIndex);
}
private void optimize() {
	Integer[] tempChildren = childMap.values().toArray(new Integer[0]);
	this.childIndices = new int[tempChildren.length];
	int j = 0;
	for(int i=0; i<tempChildren.length; i++) childIndices[i] = tempChildren[i];
	if(this.parent_id != null)
		this.parentIdx = boneMap.get(this.parent_id).index;
	this.transformIdx = transformMap.get(this.transform_id).getIndex();
	if(this.constraint_id != null)
		this.constraintIdx = constraintMap.get(this.constraint_id).getIndex();
	if(this.target_id != null)
		this.targetIdx = targetMap.get(this.target_id).getIndex();
}

private void validate() {
	if(assumeValid) return;
	TransformState transform = transformMap.get(this.transform_id); 
	if(transform == null) //check that the bone has a transform
		throw new RuntimeException("Bone '"+this.id+"' references transform with id '"+this.transform_id+"', but '"+this.transform_id+"' has not been registered with the SkeletonState.");
	if(this.parent_id != null) { //if this isn't a root bone, ensure the following
		BoneState parent = boneMap.get(this.parent_id);
		if(parent == null) //check that the bone listed as its parent has been registered.
			throw new RuntimeException("Bone '"+this.id+"' references parent bone with id '"+this.parent_id+"', but '"+this.parent_id+"' has not been registered with the SkeletonState.");
		TransformState parentBonesTransform = transformMap.get(parent.transform_id);
		TransformState transformsParent = transformMap.get(transform.parent_id);
		if(parentBonesTransform != transformsParent) { //check that the parent transform of this bones transform 
			// is the same as the transform as the ransform of the bone's parent
			throw new RuntimeException("Bone '"+this.id+"' has listed bone with id '"+this.parent_id+"' as its parent, which has a transform_id of '"+parent.transform_id+
					"' but the parent transform of this bone's transform is listed as "+transform.parent_id+"'. A bone's transform must have the parent bone's transform as its parent");
		}
		//avoid grandfather paradoxes
		BoneState ancestor = parent;
		while(ancestor != null) {
			if(ancestor == this) {
				throw new RuntimeException("Bone '"+this.id+"' is listed as being both a descendant and an ancestor of itself.");
			}
			if(ancestor.parent_id == null) 
				ancestor = null;
			else {
				BoneState curr = ancestor;
				ancestor = boneMap.get(ancestor.parent_id);
				if(ancestor == null) {
					throw new RuntimeException("bone with id `"+curr.id+"` lists its parent bone as having id `"+curr.parent_id+"', but no such bone has been registered with the SkeletonState");  
				}
			}
		}
	} else {
		if(this.constraint_id != null) {
			throw new RuntimeException("Bone '"+this.id+"' has been determined to be a root bone. However, root bones may not be constrained."
					+ "If you wish to constrain the root bone anyway, please insert a fake unconstrained root bone prior to this bone. Give that bone's transform values equal to this bone's, and set this "
					+ "bone's transformsto identity. ");
		}
	}	      		
	if(this.constraint_id != null) { //if this bone has a constraint, ensure the following:
		ConstraintState constraint = constraintMap.get(this.constraint_id) ;
		if(constraint == null)//check that the constraint has been registered
			throw new RuntimeException("Bone '"+this.id+"' claims to be constrained by '"+this.constraint_id+"', but no such constraint has been registered with this SkeletonState");
		if(!constraint.forBone_id.equals(this.id)) {
			throw new RuntimeException("Bone '"+this.id+"' claims to be constrained by '"+constraint.id+"', but constraint of id '"+constraint.id+"' claims to be constraining bone with id '"+constraint.forBone_id+"'");
		}
	}
}
public int getIndex() {
	return index;
}
public String getIdString() {
	return this.id;
}
public void setStiffness(double stiffness) {
	this.stiffness = stiffness;
}