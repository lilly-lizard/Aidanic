precision highp float;

// global config

#define ELLIPSOID_COUNT_PER_TLAS 8
#define MAX_MARCHING_STEPS 100
#define EPSILON 0.0001
#define MAX_DISTANCE 100.0

#define AMBIENT 0.2
#define T_MIN_SHADOW 0.0001

// structs

struct HitPayload {
	vec4 color;
	vec4 normal;
	int objectID;
};

struct RayPayload {
    vec4 color;
	int objectID;
};

struct ShadowPayload {
    bool in_shadow;
};

struct Ellipsoid {
	vec4 center;
	vec4 radius;
	vec4 color;
	int objectID;
};