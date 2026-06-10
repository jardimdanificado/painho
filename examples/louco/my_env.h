#ifndef LOUCO_ENV_H
#define LOUCO_ENV_H

#include "../../src/louro/louro.h"
#include "../../src/louro/libs/louro_std.h"
#include <math.h>

// Some variables we want to expose to our script
double player_x = 100.0;
double player_y = 150.0;
double speed = 2.5;

// A custom function to expose
static inline double calculate_distance(double x1, double y1, double x2, double y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

// LOURO_EXPORTS array is strictly required by the transpiler
LouroVariable louro_exports[] = {
    LOURO_STD, // Built-in std library (math, logic)
    LOURO_VAR("player_x", &player_x),
    LOURO_VAR("player_y", &player_y),
    LOURO_VAR("speed", &speed),
    LOURO_PURE("dist", calculate_distance, 4)
};

#endif
