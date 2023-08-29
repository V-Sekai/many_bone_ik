#ifndef MLBFGSSolver_H
#define MLBFGSSolver_H

#include "core/error/error_macros.h"
#include "core/object/gdvirtual.gen.inc"
#include "core/object/ref_counted.h"
#include "core/object/script_language.h"

#include "core/variant/variant.h"
#include "thirdparty/LBFGSpp/include/LBFGSB.h"
#include "thirdparty/eigen/Eigen/Core"

#include <iostream>
#include <functional>

class LBFGSBSolver : public RefCounted {
	GDCLASS(LBFGSBSolver, RefCounted);

protected:
	static void _bind_methods() {
		GDVIRTUAL_BIND(_call_operator, "x", "grad");
		ClassDB::bind_method(D_METHOD("minimize", "x", "fx", "lower_bound", "upper_bound"), &LBFGSBSolver::minimize);
	}
	GDVIRTUAL2RC(double, _call_operator, TypedArray<double>, TypedArray<double>);

    std::function<double(const Eigen::VectorXd &p_x, Eigen::VectorXd &r_grad)> operator_pointer;

    double operator_call(const TypedArray<double> &p_x, TypedArray<double> &r_grad) { 
        return call_operator(p_x, r_grad); 
    };
public:
    LBFGSBSolver() {        
        operator_pointer = std::bind(&LBFGSBSolver::native_operator, this, std::placeholders::_1, std::placeholders::_2);
    }
	double native_operator(const Eigen::VectorXd &p_x, Eigen::VectorXd &r_grad) {
		TypedArray<double> x = LBFGSBSolver::eigen_to_godot(p_x);
		TypedArray<double> grad = LBFGSBSolver::eigen_to_godot(r_grad);
		Eigen::VectorXd vec(r_grad.size());
		for (int i = 0; i < r_grad.size(); ++i) {
			vec[i] = r_grad[i];
		}
        double fx = call_operator(x, grad);
        r_grad = godot_to_eigen(grad);
		return fx;
	}
	double call_operator(const TypedArray<double> &p_x, TypedArray<double> &r_grad) {
		double ret = 0;
		if (GDVIRTUAL_CALL(_call_operator, p_x, r_grad, ret)) {
			return ret;
		};
		return 0;
	}
	static Eigen::VectorXd godot_to_eigen(const TypedArray<double> &array) {
        ERR_FAIL_COND_V(!array.size(), Eigen::VectorXd());
		Eigen::VectorXd vector(array.size());
		for (int i = 0; i < array.size(); ++i) {
			vector[i] = array[i];
		}
		return vector;
	}
	static TypedArray<double> eigen_to_godot(const Eigen::VectorXd &vector) {
        ERR_FAIL_COND_V(!vector.size(), TypedArray<double>());
        int size = vector.size();
        TypedArray<double> array;
        array.resize(size);
        for (int i = 0; i < size; ++i) {
            array[i] = vector[i];
        }
        return array;
	};
	Array minimize(TypedArray<double> p_x,
			double p_fx, TypedArray<double> p_lower_bound, TypedArray<double> p_upper_bound) {
		LBFGSpp::LBFGSBParam<double> param;
		param.epsilon = 1e-6;
		param.max_iterations = 100;

		LBFGSpp::LBFGSBSolver<double> solver(param);

		Eigen::VectorXd lower_bound = godot_to_eigen(p_lower_bound);
		Eigen::VectorXd upper_bound = godot_to_eigen(p_upper_bound);

		Eigen::VectorXd x = godot_to_eigen(p_x);

		double fx = 0;
		int niter = solver.minimize(operator_pointer, x, fx, lower_bound, upper_bound);
		Array ret;
		ret.push_back(niter);
		ret.push_back(eigen_to_godot(x));
		ret.push_back(fx);

		std::cout << niter << " iterations" << std::endl;
		std::cout << "x = \n"
				  << x.transpose() << std::endl;
		std::cout << "f(x) = " << fx << std::endl;

		return ret;
	}
};

#endif // MLBFGSSolver_H
