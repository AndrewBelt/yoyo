#pragma once

#include <math.h>

typedef struct {
	double x, y;
} Vec;

static inline double vec_norm(Vec v) {
	return hypot(v.x, v.y);
}

static inline Vec vec_add(Vec v, Vec w) {
	return (Vec) {v.x + w.x, v.y + w.y};
}

static inline Vec vec_minus(Vec v, Vec w) {
	return (Vec) {v.x - w.x, v.y - w.y};
}

static inline Vec vec_mult(Vec v, double a) {
	return (Vec) {v.x * a, v.y * a};
}

static inline Vec vec_div(Vec v, double a) {
	return (Vec) {v.x / a, v.y / a};
}

static inline double vec_dot(Vec v, Vec w) {
	return v.x*w.x + v.y*w.y;
}

static inline Vec vec_normalize(Vec v) {
	double h = vec_norm(v);
	if (h == 0)
		*((int*)0)=0;
	return vec_div(v, vec_norm(v));
}

static inline Vec vec_rot90CW(Vec v) {
	return (Vec) {v.y, -v.x};
}

static inline double vec_l1norm(Vec v) {
	return fabs(v.x) + fabs(v.y);
}

static inline double vec_square(Vec v) {
	return v.x*v.x + v.y*v.y;
}