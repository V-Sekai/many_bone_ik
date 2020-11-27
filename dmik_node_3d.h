#pragma once
#include "core/math/vector3.h"
#include "core/object/reference.h"
#include "dmik_axis_dependency.h"
#include "dmik_transform.h"

class DMIKAxisDependency;

class DMIKNode3D : public DMIKAxisDependency {
private:
	Ref<DMIKAxisDependency> parent;
	int slipType = 0;

protected:
	Vector3 workingVector;
	bool areGlobal = true;

public:
	int NORMAL = 0, IGNORE = 1, FORWARD = 2;
	int RIGHT = 1, LEFT = -1;
	int X = 0, Y = 1, Z = 2;
	bool debug = false;
	DMIKTransform localMBasis;
	DMIKTransform globalMBasis;
	bool dirty = true;

	Vector3 tempOrigin;

	List<Ref<DMIKAxisDependency>> dependentsRegistry;
	void createTempVars(Vector3 type);

	Ref<DMIKAxisDependency> get_parent_axes();

	void updateGlobal();

	/**
     * Sets the parentAxes for this axis globally.
     * in other words, globalX, globalY, and globalZ remain unchanged, but lx, ly, and lz
     * change.
     *
     *   @param intendedParent the new parent Axes
     *   @param requestedBy the object making thisRequest, will be passed on to parentChangeWarning
     *   for any AxisDependancy objects registered with this Ref<AbstractAxes>  (can be nullptr if not important)
     **/
	void setParent(Ref<DMIKNode3D> intendedParent, Ref<DMIKNode3D> requestedBy);

	/**
     * Sets the parentAxes for this axis globally.
     * in other words, globalX, globalY, and globalZ remain unchanged, but lx, ly, and lz
     * change.
     *
     *   @param par the new parent Axes
     **/
	void setParent(Ref<DMIKNode3D> par);

	Vector3 origin_();

	/**
		 * Make a GlobalCopy of these Axes.
		 * @return
		 */
	Ref<DMIKNode3D> getGlobalCopy();

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
		 * in other words, lx, ly, lz remain unchanged, but globalX, globalY, and globalZ
		 * change.
		 *
		 * if setting this parent would result in a dependency loop, then the input Axes
		 * parent is set to this Axes' parent, prior to this axes setting the input axes
		 * as its parent.
		 **/
	void setRelativeToParent(Ref<DMIKNode3D> par);

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
	Vector3 setToGlobalOf(
			Vector3 in);

	/**
		 *  Given an input vector in this axes local coordinates, modifies the output vector's values to represent the input's position in global coordinates.
		 * @param in
		 */
	void setToGlobalOf(Vector3 input, Vector3 &output);

	/**
		 *  Given an input sgRay in this axes local coordinates, modifies the output Rays's values to represent the input's in global coordinates.
		 * @param in
		 */
	void setToGlobalOf(Ray input, Ray &output);

	Ray getGlobalOf(Ray in);

	/**
		 * returns an axis representing the global location of this axis if the input axis were its parent.
		 * @param in
		 * @return
		 */
	virtual Ref<DMIKNode3D> relativeTo(Ref<DMIKNode3D> in);

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
	void setToLocalOf(Vector3 in, Vector3 &out);

	/**
		 *  Given a sgRay in global coordinates, modifies the sgRay's values to represent its position in theseAxes local coordinates.
		 * @param in
		 */
	void setToLocalOf(Ray in, Ray &out);
	void setToLocalOf(DMIKTransform input, DMIKTransform &output);

	void setToLocalOf(DMIKNode3D input, DMIKNode3D &output);

	Ray getLocalOf(Ray in);
	virtual Ref<DMIKNode3D> getLocalOf(Ref<DMIKNode3D> input);
	;

	virtual DMIKTransform getLocalOf(DMIKNode3D input);
	;

	void translateByLocal(Vector3 translate);
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
	virtual Ref<DMIKNode3D> freeCopy();
	;

	/**
		 *  return a ray / segment representing this Axes global x basis position and direction and magnitude
		 * @return a ray / segment representing this Axes global x basis position and direction and magnitude
		 */
	virtual Ray x_();
	;

	/**
		 *  return a ray / segment representing this Axes global y basis position and direction and magnitude
		 * @return a ray / segment representing this Axes global y basis position and direction and magnitude
		 */
	virtual Ray y_();
	;

	/**
		 *  return a ray / segment representing this Axes global z basis position and direction and magnitude
		 * @return a ray / segment representing this Axes global z basis position and direction and magnitude
		 */
	virtual Ray z_();
	;

	/**
	 * Creates an exact copy of this Axes object. Attached to the same parent as this Axes object
	 * @param slipAware
	 * @return
	 */
	virtual Ref<DMIKNode3D> attachedCopy(bool slipAware);
	;
	void setSlipType(int type);
	int getSlipType();
	void rotateAboutX(float angle, bool orthonormalized);
	void rotateAboutY(float angle, bool orthonormalized);
	void rotateAboutZ(float angle, bool orthonormalized);

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
		 * this function normalizes and orthogonalizes them regardless of whether the targetAxes are orthonormal.
		 *
		 * @param targetAxes the Axes to make this Axis identical to
		 */
	void alignLocalsTo(Ref<DMIKNode3D> targetAxes);

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
	void alignGlobalsTo(Ref<DMIKNode3D> targetAxes);
	void alignOrientationTo(Ref<DMIKNode3D> targetAxes);

	/**
		 * updates the axes object such that its global orientation
		 * matches the given Quat object.
		 * @param rotation
		 */
	void setGlobalOrientationTo(Quat rotation);
	void registerDependent(Ref<DMIKAxisDependency> newDependent);
	bool isAncestorOf(Ref<DMIKNode3D> potentialDescendent);

	/**
		 * unregisters this Ref<AbstractAxes> from its current parent and
		 * registers it to a new parent without changing its global position or orientation
		 * when doing so.
		 * @param newParent
		 */

	void transferToParent(Ref<DMIKNode3D> newParent);

	/**
		 * unregisters this Ref<AbstractAxes> from its parent,
		 * but keeps its global position the same.
		 */
	void emancipate();
	void disown(Ref<DMIKAxisDependency> child);
	DMIKTransform getGlobalMBasis();
	DMIKTransform getLocalMBasis();
	virtual void axis_slip_warning(Ref<DMIKAxisDependency> globalPriorToSlipping, Ref<DMIKAxisDependency> globalAfterSlipping, Ref<DMIKAxisDependency> actualAxis, List<Object> dontWarn);

	/**
		 * if the input axes have have the same global
		 * values as these axes, returns true, otherwise, returns false.
		 *
		 * This function is orthonormality aware. Meaning, if the orthonormality
		 * constraint is enabled on either axes, that axes' orthonormal version
		 * will be used in the comparison.
		 * @param ax
		 */
	bool equals(Ref<DMIKNode3D> ax);
	virtual void axis_slip_warning(Ref<DMIKAxisDependency> globalPriorToSlipping, Ref<DMIKAxisDependency> globalAfterSlipping, Ref<DMIKAxisDependency> actualAxis);
	virtual void axisSlipCompletionNotice(Ref<DMIKAxisDependency> globalPriorToSlipping, Ref<DMIKAxisDependency> globalAfterSlipping, Ref<DMIKAxisDependency> thisAxis);
	void slipTo(Ref<DMIKNode3D> newAxisGlobal);
	void slipTo(Ref<DMIKNode3D> newAxisGlobal, List<Object> dontWarn);
	// void notifyDependentsOfSlip(Ref<DMIKNode3D> newAxisGlobal, List<Object> dontWarn);
	// void notifyDependentsOfSlipCompletion(Ref<DMIKNode3D> globalAxisPriorToSlipping, List<Object> dontWarn) {
	// 	for (int i = 0; i < dependentsRegistry.size(); i++) {
	// 		List<Ref<AxisDependency>>::Element *O = dontWarn.find(dependentsRegistry[i]);
	// 		if (O) {
	// 			O->get().axisSlipCompletionNotice(globalAxisPriorToSlipping, getGlobalCopy(), this);
	// 		}
	// 	}
	// }
	// void notifyDependentsOfSlip(Ref<DMIKNode3D> newAxisGlobal);
	// void notifyDependentsOfSlipCompletion(Ref<DMIKNode3D> globalAxisPriorToSlipping) {
	// 	for (int i = 0; i < dependentsRegistry.size(); i++) {
	// 		dependentsRegistry[i]->axisSlipCompletionNotice(globalAxisPriorToSlipping, getGlobalCopy(), this);
	// 	}
	// }

	void mark_dirty();

	void mark_dependents_dirty();
	DMIKNode3D();
	/**
     * @param globalMBasis a Basis object for this Axes to adopt the values of
     * @param customBases set to true if you intend to use a custom Bases class, in which case, this constructor will not initialize them.
     */
	DMIKNode3D(DMIKTransform p_globalBasis, Ref<DMIKNode3D> p_parent);

	/**
		 * @param origin the center of this axes basis. The basis vector parameters will be automatically ADDED to the origin in order to create this basis vector.
		 * @param inX the direction of the X basis vector in global coordinates, given as an offset from this base's origin in global coordinates.
		 * @param inY the direction of the Y basis vector in global coordinates, given as an offset from this base's origin in global coordinates.
		 * @param inZ the direction of the Z basis vector in global coordinates, given as an offset from this base's origin in global coordinates.
		 * @param forceOrthoNormality
		 * @param customBases set to true if you intend to use a custom Bases class, in which case, this constructor will not initialize them.
		 */
	DMIKNode3D(Vector3 origin, Vector3 inX, Vector3 inY, Vector3 inZ, Ref<DMIKNode3D> parent, bool customBases);

	DMIKNode3D(Transform p_globalBasis, Ref<DMIKNode3D> p_parent);
};
