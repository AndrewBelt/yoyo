#pragma once

#include "vec.h"

#define N 50
#define m (1e-3/N)
#define M 66.5E-3 // Yoyofactory MVP mass
#define RNODE 1e-2
#define RAXLE 20.0e-3
#define RPAD 40.0e-3
#define KNODE (20.0/4e-2*1.0/(2*RNODE))
#define KFIX 1.0e0
#define KAXLE 1.0e3
#define OMEGA 0.05
#define RHODRAG 1.0e-4

#define ALPHA 1e5
#define BETA 1e3

void init();
void destroy();
void step(double);

double get_omega();
Vec get_X();
Vec get_x(int i);
void fix_x0(Vec v);
