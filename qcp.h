/*************************************************************************/
/*  qcp.h                                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef QCP_H
#define QCP_H

#include "core/config/engine.h"
#include "ik_quat.h"

class QCP : public Reference {

	/**
     * Implementation of the Quaternionff-Based Characteristic Polynomial algorithm
     * for RMSD and Superposition calculations.
     * <p>
     * Usage:
     * <p>
     * The input consists of 2 SGVec_3f arrays of equal length. The input coordinates
     * are not changed.
     *
     * <pre>
     *    SGVec_3f[] x = ...
     *    SGVec_3f[] y = ...
     *    SuperPositionQCP qcp = new SuperPositionQCP();
     *    qcp.set(x, y);
     * </pre>
     * <p>
     * or with weighting factors [0 - 1]]
     *
     * <pre>
     *    float[] weights = ...
     *    qcp.set(x, y, weights);
     * </pre>
     * <p>
     * For maximum efficiency, create a SuperPositionQCP object once and reuse it.
     * <p>
     * A. Calculate rmsd only
     *
     * <pre>
     * float rmsd = qcp.getRmsd();
     * </pre>
     * <p>
     * B. Calculate a 4x4 transformation (rotation and translation) matrix
     *
     * <pre>
     * Matrix4f rottrans = qcp.getTransformationMatrix();
     * </pre>
     * <p>
     * C. Get transformated points (y superposed onto the reference x)
     *
     * <pre>
     * SGVec_3f[] ySuperposed = qcp.getTransformedCoordinates();
     * </pre>
     * <p>
     * Citations:
     * <p>
     * Liu P, Agrafiotis DK, & Theobald DL (2011) Reply to comment on: "Fast
     * determination of the optimal rotation matrix for macromolecular
     * superpositions." Journal of Computational Chemistry 32(1):185-186.
     * [http://dx.doi.org/10.1002/jcc.21606]
     * <p>
     * Liu P, Agrafiotis DK, & Theobald DL (2010) "Fast determination of the optimal
     * rotation matrix for macromolecular superpositions." Journal of Computational
     * Chemistry 31(7):1561-1563. [http://dx.doi.org/10.1002/jcc.21439]
     * <p>
     * Douglas L Theobald (2005) "Rapid calculation of RMSDs using a
     * quaternion-based characteristic polynomial." Acta Crystallogr A
     * 61(4):478-480. [http://dx.doi.org/10.1107/S0108767305015266 ]
     * <p>
     * This is an adoption of the original C code QCProt 1.4 (2012, October 10) to
     * Java. The original C source code is available from
     * http://theobald.brandeis.edu/qcp/ and was developed by
     * <p>
     * Douglas L. Theobald Department of Biochemistry MS 009 Brandeis University 415
     * South St Waltham, MA 02453 USA
     * <p>
     * dtheobald@brandeis.edu
     * <p>
     * Pu Liu Johnson & Johnson Pharmaceutical Research and Development, L.L.C. 665
     * Stockton Drive Exton, PA 19341 USA
     * <p>
     * pliu24@its.jnj.com
     * <p>
     *
     * @author Douglas L. Theobald (original C code)
     * @author Pu Liu (original C code)
     * @author Peter Rose (adopted to Java)
     * @author Aleix Lafita (adopted to Java)
     * @author Eron Gjoni (adopted to EWB IK)
     */

	float evec_prec = (float)1E-6;
	float eval_prec = (float)1E-11;
	int max_iterations = 5;

	Vector<Vector3> moved;

	Vector<real_t> weight;
	float wsum = 0;

	Vector3 target_center;
	Vector3 moved_center;

	float e0 = 0;
	float rmsd = 0;
	float Sxy, Sxz, Syx, Syz, Szx, Szy = 0;
	float SxxpSyy, Szz, mxEigenV, SyzmSzy, SxzmSzx, SxymSyx = 0;
	float SxxmSyy, SxypSyx, SxzpSzx = 0;
	float Syy, Sxx, SyzpSzy = 0;
	bool rmsd_calculated = false;
	bool transformation_calculated = false;

public:
	Vector<Vector3> target;

	/**
     * Constructor with option to set the precision values.
     *
     * @param p_eval_prec
     *            required eigenvector precision
     * @param p_eval_prec
     *            required eigenvalue precision
     */
	void set_precision(float p_evec_prec, float p_eval_prec) {
		evec_prec = p_evec_prec;
		eval_prec = p_eval_prec;
	}

	/**
     * Sets the maximum number of iterations QCP should run before giving up.
     * In most situations QCP converges in 3 or 4 iterations, but in some situations convergence
     * occurs slowly or not at all, and so an exit condition is used. The default value is 5.
     * Increase it for more stability.
     * @param p_max
     */
	void set_max_iterations(int p_max);

	/**
     * Sets the two input coordinate arrays. These input arrays must be of equal
     * length. Input coordinates are not modified.
     *
     * @param p_target
     *            3f points of reference coordinate set
     * @param p_moved
     *            3f points of coordinate set for superposition
     */
private:
	void set(Vector<Vector3> p_target, Vector<Vector3> p_moved);

	/**
     * Sets the two input coordinate arrays and weight array. All input arrays
     * must be of equal length. Input coordinates are not modified.
     *
     * @param p_target
     *            3f points of reference coordinate set
     * @param p_moved
     *            3f points of coordinate set for superposition
     * @param p_weight
     *            a weight in the inclusive range [0,1] for each point
     */
public:
	void set(Vector<Vector3> p_moved, Vector<Vector3> p_target, Vector<real_t> p_weight, bool p_translate);

	/**
     * Return the RMSD of the superposition of input coordinate set y onto x.
     * Note, this is the fasted way to calculate an RMSD without actually
     * superposing the two sets. The calculation is performed "lazy", meaning
     * calculations are only performed if necessary.
     *
     * @return root mean square deviation for superposition of y onto x
     */
public:
	float get_rmsd();

	/**
     * Weighted superposition.
     *
     * @param p_target
     * @param p_moved
     * @param p_weight
     *            array of weigths for each equivalent point position
     * @return
     */
	Quat weighted_superpose(Vector<Vector3> p_moved, Vector<Vector3> p_target, Vector<real_t> p_weight, bool p_translate);

private:
	Quat get_rotation();

	/**
     * Calculates the RMSD value for superposition of y onto x. This requires
     * the coordinates to be precentered.
     *
     * @param p_x
     *            3f points of reference coordinate set
     * @param p_y
     *            3f points of coordinate set for superposition
     */
	void calc_rmsd(Vector<Vector3> p_x, Vector<Vector3> p_y);

	/**
     * Calculates the inner product between two coordinate sets x and y
     * (optionally weighted, if weights set through
     * {@link #set(SGVec_3f[], SGVec_3f[], float[])}). It also calculates an
     * upper bound of the most positive root of the key matrix.
     * http://theobald.brandeis.edu/qcp/qcprot.c
     *
     * @param p_coords1
     * @param p_coords2
     * @return
     */
	void inner_product(Vector<Vector3> p_coords1, Vector<Vector3> p_coords2);

	int calc_rmsd(float p_len);

	Quat calc_rotation();

public:
	float get_rmsd(Vector<Vector3> p_fixed, Vector<Vector3> p_moved);

	void translate(Vector3 p_trans, Vector<Vector3> p_x);

	Vector3 get_weighted_center(Vector<Vector3> p_to_center, Vector<real_t> p_weight, Vector3 p_center);

	Vector3 get_translation();
};

#endif //QCP_H
