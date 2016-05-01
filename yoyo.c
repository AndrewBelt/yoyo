#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#undef N
#undef M
#include "yoyo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <gsl/gsl_odeiv2.h>
#include <gsl/gsl_errno.h>
#include <omp.h>

struct {
	gsl_odeiv2_system sys;
	gsl_odeiv2_driver *driver;
	double *u;
	Vec x0_pos;
} solver;


// Nonzero only for i=k and i=k+1
// For dJdt, pass v instead of x
Vec J_get(const Vec *x, int i, int k) {
	if (i == k + 1)
		return vec_mult(vec_minus(x[k+1], x[k]), 2.0);
	if (i == k)
		return vec_mult(vec_minus(x[k+1], x[k]), -2.0);
	return (Vec){0.0, 0.0};
}

double C_get(const Vec *x, int k) {
	return vec_square(vec_minus(x[k+1], x[k])) - RNODE*RNODE;
}

double A_get(const Vec *x, int k, int l) {
	double A_kl = 0.0;
	// J W J^T
	for (int j = 0; j < N; j++) {
		A_kl += 1.0/m * vec_dot(J_get(x, j, k), J_get(x, j, l));
	}
	return A_kl;
}


int f_callback(double t, const double u[], double dudt[], void *data) {
	const Vec * X = (Vec *) (u+1);
	const Vec * V = (Vec *) (u+3);
	const Vec * x = (Vec *) (u+5);
	const Vec * v = (Vec *) (u+5+(2*N));
	Vec * dXdt = (Vec *) (dudt+1);
	Vec * dVdt = (Vec *) (dudt+3);
	Vec * dxdt = (Vec *) (dudt+5);
	Vec * dvdt = (Vec *) (dudt+5+(2*N));

	// printf("t: %lg\n", t);

	// Initialize dudt to zero
	for (int i = 0; i < solver.sys.dimension; i++) {
		dudt[i] = 0.0;
	}

	// Velocity
	dXdt[0] = V[0];
	for (int i = 0; i < N; i++) {
		dxdt[i] = v[i];
		// printf("%lf, %lf; %lf, %lf\n", x[i].x, x[i].y, v[i].x, v[i].y);
	}

	// Rotation friction
	dudt[0] = -OMEGA*u[0];

	// External force
	Vec F_g = (Vec){0.0, 9.8*m};
	Vec F[N];
	for (int i = 0; i < N; i++) {
		F[i] = F_g;
		F[i] = vec_add(F[i], vec_mult(v[i], -RHODRAG));
	}
	{
		Vec F_fix = vec_mult(vec_minus(x[0], solver.x0_pos), -KFIX);
		F[0] = vec_add(F[0], F_fix);
	}

	// Build b vector in A*lambda=b
	gsl_vector *b = gsl_vector_calloc(N-1);
	for (int k = 0; k < N-1; k++) {
		double b_k = 0.0;
		// -1/m J W Q
		for (int j = k; j <= k+1; j++) {
			b_k -= 1.0/m * vec_dot(J_get(x, j, k), F[k]);
		}
		// -J' q'
		for (int j = k; j <= k+1; j++) {
			b_k -= vec_dot(J_get(v, j, k), v[j]);
		}
		// -alpha*C
		b_k -= ALPHA * C_get(x, k);
		// -beta*C'
		for (int j = k; j <= k+1; j++) {
			b_k -= BETA * vec_dot(J_get(x, j, k), v[j]);
		}
		// printf("%lg\n", b_k);
		gsl_vector_set(b, k, b_k);
	}

	// Build A matrix in A*lambda=b
	gsl_vector *A_d = gsl_vector_calloc(N-1);
	gsl_vector *A_e = gsl_vector_calloc(N-2);
	gsl_vector *A_f = gsl_vector_calloc(N-2);
	for (int k = 0; k < N-1; k++) {
		gsl_vector_set(A_d, k, A_get(x, k, k));
	}
	for (int k = 0; k < N-2; k++) {
		gsl_vector_set(A_e, k, A_get(x, k, k+1));
	}
	for (int k = 0; k < N-2; k++) {
		gsl_vector_set(A_f, k, A_get(x, k+1, k));
	}

	// Solve A*lambda=b
	gsl_vector *lambda = gsl_vector_calloc(N-1);
	gsl_linalg_solve_tridiag(A_d, A_e, A_f, b, lambda);

	// for (int i = 0; i < N-1; i++) {
	// 	printf("%lg\n", gsl_vector_get(lambda, i));
	// }

	gsl_vector_free(A_d);
	gsl_vector_free(A_e);
	gsl_vector_free(A_f);
	gsl_vector_free(b);

	// Set x'' from the external force and constraint forces
	for (int i = 0; i < N; i++) {
		Vec F_c = (Vec){0.0, 0.0};
		for (int k = 0; k < N-1; k++) {
			F_c = vec_add(F_c, vec_mult(J_get(x, i, k), gsl_vector_get(lambda, k)));
		}
		// printf("%lg\t", F_c);
		dvdt[i] = vec_div(vec_add(F[i], F_c), m);
		assert(fabs(vec_norm(F_c))<1e10);
	}
	// printf("\n");
	gsl_vector_free(lambda);

	// for (int k = 0; k < N-1; k++) {
	// 	printf("%lg\n", vec_norm(vec_minus(x[k+1], x[k])));
	// }
	// printf("\n");

	// Phew
	return GSL_SUCCESS;
}


void init(){
	int dim = 4*N + 5;
	solver.u = malloc(sizeof(double)*dim);
	memset(solver.u, 0, dim*sizeof(double));

	solver.x0_pos = (Vec) {0.5, 0.2};

	Vec * X = (Vec *) (solver.u+1);
	Vec * V = (Vec *) (solver.u+3);
	Vec * x = (Vec *) (solver.u+5);
	Vec * v = (Vec *) (solver.u+5+(2*N));

	solver.u[0] = -2*M_PI*20.0;

	double y = 0.0;
	int i;
	for (i = 0; i < N; i++) {
		y += RNODE;
		x[i] = vec_add(solver.x0_pos, (Vec){y, 0.0});
		// v[i] = (Vec){1e-4*(drand48()-0.5), 1e-4*(drand48()-0.5)};
		v[i] = (Vec){0.0, 0.0};
	}
	v[0] = (Vec){0.0, 1.0};
	v[N-1] = (Vec){0.0, -1.0};
	X[0] = (Vec){0.0, RAXLE+RPAD};
	V[0] = (Vec){0.0, 0.0};


	solver.sys = (gsl_odeiv2_system) {f_callback, NULL, dim, NULL};
	solver.driver = gsl_odeiv2_driver_alloc_y_new(&solver.sys, gsl_odeiv2_step_rkck, 5e-6, 5e-3, 0.0);
	gsl_odeiv2_driver_set_nmax(solver.driver, 1000);
}

void step(double dt) {
	double t = 0.0;
	gsl_odeiv2_driver_apply(solver.driver, &t, t + dt, solver.u);
}

void destroy(){
	free(solver.u);
}


double get_omega() {
	return solver.u[0];
}

Vec get_X() {
	Vec * X = (Vec *) (solver.u+1);
	return X[0];
}

Vec get_x(int i) {
	Vec * x = (Vec *) (solver.u+5);
	return x[i];
}

void fix_x0(Vec v) {
	solver.x0_pos = v;
}
