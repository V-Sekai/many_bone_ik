#pragma once
#include "core/object/reference.h"
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


class DMIKTransform {
protected:
	Vector3 xBase = Vector3(1,0,0);
	Vector3 yBase = Vector3(0,1,0);
	Vector3 zBase = Vector3(0,0,1);
	Ray xRay = Ray(Vector3(0,0,0), Vector3(1,0,0));
	Ray yRay = Ray(Vector3(0,0,0), Vector3(0,1,0));
	Ray zRay = Ray(Vector3(0,0,0), Vector3(0,0,1));

public:
	const static int LEFT = -1;
	const static int RIGHT = 1;
	const int chirality = RIGHT;
	const static int NONE = -1;
	const static int X = 0;
	const static int Y = 1;
	const static int Z = 2;
	Quat rotation;
	Quat inverseRotation;
	/**
	 * a vector representing the translation of this basis relative to its parent. 
	 */
	Vector3 translate;
private:
	void updateRays() {
		xRay.position = this->translate;
		xRay.normal = xBase;
		yRay.position = this->translate;
		yRay.normal = yBase;
		zRay.position = this->translate;
		zRay.normal = zBase;

		rotation.set_euler(xRay.normal);
		rotation.set_euler(yRay.normal);
		rotation.set_euler(zRay.normal);

		xRay.normal += this->translate;
		yRay.normal += this->translate;
		zRay.normal += this->translate;
	}
	Quat createPrioritizedRotation(Vector3 xHeading, Vector3 yHeading, Vector3 zHeading) {
		Quat toYZ = Quat(yBase * zBase * yHeading * zHeading);
		Vector3 tempV = toYZ.get_euler() * yBase;
		Quat toY = Quat(tempV * yHeading);
		return toY * toYZ;
	}
	void set(Vector3 x, Vector3 y, Vector3 z) {
		xBase = translate;
		yBase = translate;
		zBase = translate;
		xBase = Vector3(1, 0, 0);
		yBase = Vector3(0, 1, 0);
		zBase = Vector3(0, 0, 1);
		Vector3 zero;
		xRay.position = zero;
		xRay.normal = xBase;
		yRay.position = zero;
		yRay.normal = yBase;
		zRay.position = zero;
		zRay.normal = zBase;
		this->rotation = createPrioritizedRotation(x, y, z);
		this->refreshPrecomputed();
	}
public:
	/**
	 * Initialize this basis at the origin. The basis will be right handed by default. 
	 * @param origin
	 */
	DMIKTransform(Vector3 origin) {
		translate = origin;
		xBase = origin;
		yBase = origin;
		zBase = origin;
		xBase *= Vector3(1, 0, 0);
		yBase *= Vector3(0, 1, 0);
		zBase *= Vector3(0, 0, 1);
		Vector3 zero;
		xRay = Ray(zero, xBase);
		yRay = Ray(zero, yBase);
		zRay = Ray(zero, zBase);
		refreshPrecomputed();
	}

	DMIKTransform(const DMIKTransform &input) {
		translate = input.translate;
		xBase = translate;
		yBase = translate;
		zBase = translate;
		xBase = Vector3(1, 0, 0);
		yBase = Vector3(0, 1, 0);
		zBase = Vector3(0, 0, 1);
		Vector3 zero = translate;
		zero = Vector3(0, 0, 0);
		xRay = Ray(zero, xBase);
		yRay = Ray(zero, yBase);
		zRay = Ray(zero, zBase);
		this->adoptValues(input);
	}

	/**
	 * Initialize this basis at the origin.
	 *  The basis will be backed by a rotation object which presumes right handed chirality. 
	 *  Therefore, the rotation object will align so its local XY plane aligns with this basis' XY plane
	 *  Afterwards, it will check chirality, and if the basis isn't right handed, this class will assume the
	 *  z-axis is the one that's been flipped. 
	 *  
	 *  If you want to manually specify which axis has been flipped 
	 *  (so that the rotation object aligns with respect to the plane formed 
	 *  by the other two basis vectors) then use the constructor dedicated for that purpose
	 * @param origin
	 * @param x basis vector direction
	 * @param y basis vector direction
	 * @param z basis vector direction
	 */
	DMIKTransform(Vector3 origin, Vector3 x, Vector3 y, Vector3 z) {
		this->translate = origin;
		xRay = Ray(origin, origin);
		yRay = Ray(origin, origin);
		zRay = Ray(origin, origin);
		this->set(x, y, z);
	}

	/**
	 * Initialize this basis at the origin defined by the base of the @param x Ray.
	 * 
	 *  The basis will be backed by a rotation object which presumes right handed chirality. 
	 *  Therefore, the rotation object will align so its local XY plane aligns with this basis' XY plane
	 *  Afterwards, it will check chirality, and if the basis isn't right handed, this class will assume the
	 *  z-axis is the one that's been flipped.
	 *   
	 *  If you want to manually specify which axis has been flipped 
	 *  (so that the rotation object aligns with respect to the plane formed 
	 *  by the other two basis vectors) then use the constructor dedicated for that purpose
	 *  
	 *  
	 * @param x basis Ray 
	 * @param y basis Ray 
	 * @param z basis Ray 
	 */
	DMIKTransform(Ray x, Ray y, Ray z) {
		this->translate = x.position;
		xRay = x;
		yRay = y;
		zRay = z;
		Vector3 xDirNew = x.heading();
		Vector3 yDirNew = y.heading();
		Vector3 zDirNew = z.heading();
		xDirNew.normalize();
		yDirNew.normalize();
		zDirNew.normalize();
		set(xDirNew, yDirNew, zDirNew);
	}

	/**
	 * takes on the same values as the input basis. 
	 * @param in
	 */
	void adoptValues(DMIKTransform in) {
		this->translate = in.translate;
		this->rotation = in.rotation;
		xBase = translate * Vector3(1, 0, 0);
		yBase = translate * Vector3(0, 1, 0);
		zBase = translate * Vector3(0, 0, 1);
		xRay = in.xRay;
		yRay = in.yRay;
		zRay = in.zRay;
		this->refreshPrecomputed();
	}
	void setIdentity() {
		this->translate = Vector3();
		xBase = Vector3(1, 0, 0);
		yBase = Vector3(0, 1, 0);
		zBase = Vector3(0, 0, 1);
		this->xRay.position = this->translate;
		this->xRay.normal = xBase;
		this->yRay.position = this->translate;
		this->yRay.normal = yBase;
		this->zRay.position = this->translate;
		this->zRay.normal = zBase;
		this->rotation = Quat();
		refreshPrecomputed();
	}

	Quat getLocalOfRotation(Quat inRot) {
		Quat resultNew = inverseRotation * inRot * rotation;
		return resultNew;
	}

	void setToLocalOf(DMIKTransform global_input, DMIKTransform local_output) {
		local_output.translate = this->getLocalOf(global_input.translate);
		inverseRotation *= global_input.rotation * local_output.rotation;
		local_output.refreshPrecomputed();
	}

	void refreshPrecomputed() {
		this->rotation = inverseRotation.inverse();
		this->updateRays();
	}

	Vector3 getLocalOf(Vector3 v) {
		Vector3 result = v;
		setToLocalOf(v, result);
		return result;
	}

	void setToLocalOf(Vector3 input, Vector3 &output) {
		output = input;
		output -= this->translate;
		output = inverseRotation.get_euler() * output;
	}

	void rotateTo(Quat newRotation) {
		this->rotation = newRotation;
		this->refreshPrecomputed();
	}

	void rotateBy(Quat addRotation) {
		this->rotation *= addRotation;
		this->refreshPrecomputed();
	}

	/**
	 * sets globalOutput such that the result of 
	 * this->getLocalOf(globalOutput) == localInput. 
	 * 
	 * @param localInput
	 * @param globalOutput
	 */
	void setToGlobalOf(DMIKTransform localInput, DMIKTransform globalOutput) {
		this->rotation *= localInput.rotation * globalOutput.rotation;
		this->setToGlobalOf(localInput.translate, globalOutput.translate);
		globalOutput.refreshPrecomputed();
	}

	void setToGlobalOf(Vector3 input, Vector3 &output) {
		rotation = Quat(input);
		output += this->translate;
	}

	void translateBy(Vector3 transBy) {
		this->translate.x += transBy.x;
		this->translate.y += transBy.y;
		this->translate.z += transBy.z;
		updateRays();
	}

	void translateTo(Vector3 newOrigin) {
		this->translate.x = newOrigin.x;
		this->translate.y = newOrigin.y;
		this->translate.z = newOrigin.z;
		updateRays();
	}

	Ray getXRay() {
		return xRay;
	}

	Ray getYRay() {
		return yRay;
	}

	Ray getZRay() {
		return zRay;
	}

	Vector3 getXHeading() {
		return this->xRay.heading();
	}

	Vector3 getYHeading() {
		return this->yRay.heading();
	}

	Vector3 getZHeading() {
		return this->zRay.heading();
	}

	Vector3 getOrigin() {
		return translate;
	}

	/**
	 * true if the input axis should be multiplied by negative one after rotation. 
	 * By default, this always returns false. But can be overridden for more advanced implementations
	 * allowing for reflection transformations. 
	 * @param axis
	 * @return true if axis should be flipped, false otherwise. Default is false. 
	 */
	bool isAxisFlipped(int axis) {
		return false;
	}

	/**
	 * @return a precomputed inverse of the rotation represented by this basis object.
	 */
	Quat getInverseRotation() {
		return this->inverseRotation;
	}
};
