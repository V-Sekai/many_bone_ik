#pragma once
#include "core/reference.h"
#include "ray.h"

class AxisDependency: public Reference {
public:
	virtual void emancipate() = 0;
	virtual void axisSlipWarning(Ref<AxisDependency> globalPriorToSlipping, Ref<AxisDependency> globalAfterSlipping, Ref<AxisDependency> thisAxis) = 0;
	virtual void axisSlipCompletionNnotice(Ref<AxisDependency> globalPriorToSlipping, Ref<AxisDependency> globalAfterSlipping, Ref<AxisDependency> thisAxis) = 0;
	virtual void parentChangeWarning(Ref<AxisDependency> warningBy, Ref<AxisDependency> oldParent, Ref<AxisDependency> intendedParent, Object requestedBy) = 0;
	virtual void parentChangeCompletionNotice(Ref<AxisDependency> warningBy, Ref<AxisDependency> oldParent, Ref<AxisDependency> intendedParent, Object requestedBy) = 0; 
	virtual void markDirty() = 0;
	virtual void markDependentsDirty() = 0;
	virtual Ref<AxisDependency> getParentAxes() = 0;
};

class AbstractAxes : public AxisDependency {
private:
	Ref<AbstractAxes> parent;
	int slipType = 0;
	List<Ref<AbstractAxes>> dependentsRegistry;

public:
	bool debug = false;
	Transform localMBasis;
	Transform globalMBasis;
	bool dirty = true;
	const int NORMAL = 0, IGNORE = 1, FORWARD = 2;
	const int RIGHT = 1, LEFT = -1;
	const int X = 0, Y = 1, Z = 2;

protected:
	Vector3 workingVector;
	bool areGlobal = true;

public:
	Ref<AxisDependency> getParentAxes() {
		if(parent.is_null()) 
			return Ref<AbstractAxes>();
		else 
			return parent;
	}
	void axis_slip_warning(Ref<AbstractAxes> p_globalPriorToSlipping, Ref<AbstractAxes> p_globalAfterSlipping, Ref<AbstractAxes> thisAxis);
	void axisSlipCompletionNotice(Ref<AbstractAxes> p_globalPriorToSlipping, Ref<AbstractAxes> p_globalAfterSlipping, Ref<AbstractAxes> p_thisAxis);
	void parentChangeWarning(Ref<AbstractAxes> p_warningBy, Ref<AbstractAxes> p_oldParent, Ref<AbstractAxes> p_intendedParent, Object p_requestedBy);
	void parentChangeCompletionNotice(Ref<AbstractAxes> p_warningBy, Ref<AbstractAxes> p_oldParent, Ref<AbstractAxes> p_intendedParent, Object p_requestedBy);
	void markDirty();
	void createTempVars(Vector3 type);

	/**
	 * @param globalMBasis a Basis object for this Axes to adopt the values of
	 * @param customBases set to true if you intend to use a custom Bases class, in which case, this constructor will not initialize them.
	 */
	AbstractAxes(Transform p_globalBasis, Ref<AbstractAxes> p_parent);

	/**
	 * @param origin the center of this axes basis. The basis vector parameters will be automatically ADDED to the origin in order to create this basis vector.
	 * @param inX the direction of the X basis vector in global coordinates, given as an offset from this base's origin in global coordinates.   
	 * @param inY the direction of the Y basis vector in global coordinates, given as an offset from this base's origin in global coordinates.
	 * @param inZ the direction of the Z basis vector in global coordinates, given as an offset from this base's origin in global coordinates.
	 * @param forceOrthoNormality
	 * @param customBases set to true if you intend to use a custom Bases class, in which case, this constructor will not initialize them.
	 */
	AbstractAxes(Vector3 origin, Vector3 inX, Vector3 inY, Vector3 inZ, Ref<AbstractAxes> p_parent, bool customBases);

	void updateGlobal();

	Vector3 tempOrigin;
	Vector3 origin_();

	/**
	 * Make a GlobalCopy of these Axes. 
	 * @return
	 */
	Ref<AbstractAxes> getGlobalCopy();

	/**
	 * Sets the parentAxes for this axis globally.  
	 * in other words, globalX, globalY, and globalZ remain unchanged, but lx, ly, and lz 
	 * change.
	 * 
	 *   @param par the new parent Axes
	 **/
	void setParent(Ref<AbstractAxes> par);

	/**
	 * Sets the parentAxes for this axis globally.  
	 * in other words, globalX, globalY, and globalZ remain unchanged, but lx, ly, and lz 
	 * change.
	 * 
	 *   @param intendedParent the new parent Axes
	 *   @param requestedBy the object making thisRequest, will be passed on to parentChangeWarning 
	 *   for any AxisDependancy objects registered with this Ref<AbstractAxes>  (can be null if not important)
	 **/
	void setParent(Ref<AbstractAxes> intendedParent, Object requestedBy);

	/**
	 * runs the given runnable on each dependent axis,
	 * taking advantage of the call to remove entirely any 
	 * weak references to elements that have been cleaned up by the garbage collector. 
	 * @param r
	 */
	void forEachDependent(Ref<AbstractAxes> action);

	int getGlobalChirality();

	int getLocalChirality();

	/**
	 * True if the input axis of this Axes object in global coordinates should be multiplied by negative one after rotation. 
	 * By default, this always returns false. But can be overridden for more advanced implementations
	 * allowing for reflection transformations. 
	 * @param axis
	 * @return true if axis should be flipped, false otherwise. Default is false. 
	 */
	bool isGlobalAxisFlipped(int axis);

	/**
	 * True if the input axis of this Axes object in local coordinates should be multiplied by negative one after rotation. 
	 * By default, this always returns false. But can be overridden for more advanced implementations
	 * allowing for reflection transformations. 
	 * @param axis
	 * @return true if axis should be flipped, false otherwise. Default is false. 
	 */
	bool isLocalAxisFlipped(int axis);

	/**
	 * Sets the parentAxes for this axis locally. 
	 * in other words, lx,ly,lz remain unchanged, but globalX, globalY, and globalZ 
	 * change.  
	 * 
	 * if setting this parent would result in a dependency loop, then the input Axes 
	 * parent is set to this Axes' parent, prior to this axes setting the input axes
	 * as its parent.   
	 **/
	void setRelativeToParent(Ref<AbstractAxes> par);

	bool needsUpdate();

	/**
	 * Given a vector in this axes local coordinates, returns the vector's position in global coordinates.
	 * @param in
	 * @return
	 */
	Vector3 getGlobalOf(Vector3 in);

	/**
	 *  Given a vector in this axes local coordinates, modifies the vector's values to represent its position global coordinates.
	 * @param in
	 * @return a reference to this the @param in object.
	 */
	Vector3 setToGlobalOf(Vector3 in);

	/**
	 *  Given an input vector in this axes local coordinates, modifies the output vector's values to represent the input's position in global coordinates.
	 * @param in
	 */
	void setToGlobalOf(Vector3 input, Vector3 output);

	/**
	 *  Given an input sgRay in this axes local coordinates, modifies the output Rays's values to represent the input's in global coordinates.
	 * @param in
	 */
	void setToGlobalOf(Ray input, Ray output);

	Ray getGlobalOf(Ray in);

	/**
	 * returns an axis representing the global location of this axis if the input axis were its parent.
	 * @param in
	 * @return
	 */
	Ref<AbstractAxes> relativeTo(Ref<AbstractAxes> in);

	Vector3 getLocalOf(Vector3 in);

	/**
	 *  Given a vector in global coordinates, modifies the vector's values to represent its position in theseAxes local coordinates.
	 * @param in
	 * @return a reference to the @param in object. 
	 */

	Vector3 setToLocalOf(Vector3 in);

	/**
	 *  Given a vector in global coordinates, modifies the vector's values to represent its position in theseAxes local coordinates.
	 * @param in
	 */

	void setToLocalOf(Vector3 in, Vector3 out);

	/**
	 *  Given a sgRay in global coordinates, modifies the sgRay's values to represent its position in theseAxes local coordinates.
	 * @param in
	 */

	void setToLocalOf(Ray in, Ray out);

	void setToLocalOf(Transform input, Transform output);

	Ray getLocalOf(Ray in) {
		Ray result = in;
		result.normal = getLocalOf(in.normal);
		result.position = getLocalOf(in.position);
		return result;
	}

	Ref<AbstractAxes> getLocalOf(Ref<AbstractAxes> input);

	Transform getLocalOf(Transform input);

	void translateByLocal(Vector3 translate) {
		updateGlobal();
		getLocalMBasis().translate(translate);
		markDirty();
	}
	void translateByGlobal(Vector3 translate);

	void translateTo(Vector3 translate, bool slip);

	void translateTo(Vector3 translate);

	/**
	 * @return a copy of these axes that does not refer to any parent. 
	 * Because there is no parent, the copy's global coordinates and local coordinates will be equivalent to each other. 
	 * The copy's local coordinates will also be equivalent to the original's local coordinates. However, the copy's
	 * global coordinates will likely be drastically different from the original's global coordinates. 
	 *   
	 */
	Ref<AbstractAxes> freeCopy();

	/**
	 *  return a ray / segment representing this Axes global x basis position and direction and magnitude
	 * @return a ray / segment representing this Axes global x basis position and direction and magnitude
	 */
	Ray x_();

	/**
	 *  return a ray / segment representing this Axes global y basis position and direction and magnitude
	 * @return a ray / segment representing this Axes global y basis position and direction and magnitude
	 */
	Ray y_();

	/**
	 *  return a ray / segment representing this Axes global z basis position and direction and magnitude
	 * @return a ray / segment representing this Axes global z basis position and direction and magnitude
	 */
	Ray z_();

	/**
 * Creates an exact copy of this Axes object. Attached to the same parent as this Axes object
 * @param slipAware
 * @return
 */
	Ref<AbstractAxes> attachedCopy(bool slipAware);

	void setSlipType(int type);

	int getSlipType();

	void rotateAboutX(float angle, bool orthonormalized);

	void rotateAboutY(float angle, bool orthonormalized);

	void rotateAboutZ(float angle, bool orthonormalized);

	void rotateBy(Basis apply);

	/**
	 * Rotates the bases around their origin in global coordinates
	 * @param rotation
	 */
	void rotateBy(Quat apply);

	/**
	 * rotates the bases around their origin in Local coordinates
	 * @param rotation
	 */
	void rotateByLocal(Quat apply);

	/**
	 * sets these axes to have the same orientation and location relative to their parent
	 * axes as the input's axes do to the input's parent axes.
	 * 
	 * If the axes on which this function is called are orthonormal, 
	 * this function normalizes and orthogonalized them regardless of whether the targetAxes are orthonormal.
	 * 
	 * @param targetAxes the Axes to make this Axis identical to
	 */
	void alignLocalsTo(Ref<AbstractAxes> targetAxes);

	/**
	 * sets the bases to the Identity basis and Identity rotation relative to its parent, and translates 
	 * its origin to the parent's origin. 
	 * 
	 * be careful calling this method, as it destroys any shear / scale information. 
	 */
	void alignToParent();

	/**
	 * rotates and translates the axes back to its parent, but maintains 
	 * its shear, translate and scale attributes.
	 */
	void rotateToParent();

	/**
	 * sets these axes to have the same global orientation as the input Axes. 
	 * these Axes lx, ly, and lz headings will differ from the target ages,
	 * but its gx, gy, and gz headings should be identical unless this 
	 * axis is orthonormalized and the target axes are not.
	 * @param targetAxes
	 */
	void alignGlobalsTo(Ref<AbstractAxes> targetAxes);

	void alignOrientationTo(Ref<AbstractAxes> targetAxes);

	/**
	 * updates the axes object such that its global orientation 
	 * matches the given Quat object. 
	 * @param rotation
	 */
	void setGlobalOrientationTo(Quat rotation);

	void registerDependent(Ref<AxisDependency> newDependent);

	bool isAncestorOf(Ref<AbstractAxes> potentialDescendent);

	/**
	 * unregisters this Ref<AbstractAxes> from its current parent and 
	 * registers it to a new parent without changing its global position or orientation 
	 * when doing so.
	 * @param newParent
	 */

	void transferToParent(Ref<AbstractAxes> newParent);

	void disown(Ref<AxisDependency> child);

	Transform getGlobalMBasis() const;

	Transform getLocalMBasis() const;

	void axisSlipWarning(Ref<AbstractAxes> p_globalPriorToSlipping, Ref<AbstractAxes> p_globalAfterSlipping, Ref<AbstractAxes> actualAxis, List<Object> dontWarn);

	/**
	 * if the input axes have have the same global
	 * values as these axes, returns true, otherwise, returns false.
	 * 
	 * This function is orthonormality aware. Meaning, if the orthonormality 
	 * constraint is enabled on either axes, that axes' orthonormal version
	 * will be used in the comparison. 
	 * @param ax
	 */
	bool equals(Ref<AbstractAxes> ax);
	void axisSlipWarning(Ref<AbstractAxes> p_globalPriorToSlipping, Ref<AbstractAxes> p_globalAfterSlipping, Ref<AbstractAxes> actualAxis);
	void slipTo(Ref<AbstractAxes> newAxisGlobal);
	void slipTo(Ref<AbstractAxes> newAxisGlobal, List<Object> dontWarn);
	void notifyDependentsOfSlip(Ref<AbstractAxes> newAxisGlobal, List<Object> dontWarn);
	void notifyDependentsOfSlipCompletion(Ref<AbstractAxes> globalAxisPriorToSlipping, List<Object> dontWarn);
	void notifyDependentsOfSlip(Ref<AbstractAxes> newAxisGlobal);
	void notifyDependentsOfSlipCompletion(Ref<AbstractAxes> globalAxisPriorToSlipping);
	void markDependentsDirty();
	String toString();
};

class CartesianAxes : public Ref<AbstractAxes> {
	CartesianAxes(Transform p_globalBasis, Ref<AbstractAxes> p_parent);
	CartesianAxes(Vector3 origin, Vector3 inX, Vector3 inY, Vector3 inZ,
			Ref<AbstractAxes> p_parent);

	Ray x_();

	Ray y_();

	Ray z_();

	bool equals(Ref<AbstractAxes> ax);

	CartesianAxes getGlobalCopy();

	Ref<AbstractAxes> relativeTo(Ref<AbstractAxes> in);

	Ref<AbstractAxes> getLocalOf(Ref<AbstractAxes> input);

	Ref<AbstractAxes> freeCopy();

	/**
	 * Creates an exact copy of this Axes object. Attached to the same parent as this Axes object
	 * @param slipAware
	 * @return
	 */
	CartesianAxes attachedCopy(bool slipAware);

	Transform getLocalOf(Transform input);
};
