#pragma once
#include <raylib.h>

class Actor;


struct World {
	Actor* root;
	Camera* cam;
};