#pragma once

#include <Urho3D/Urho3DAll.h>

enum ThrowableType {
	APPLE,
	BANANA,
	// BOTTLE,
	BURGER,
	CARROT,
	CUPCAKE,
	DOUNUT,
	MILK,
	POTATO,
	SANDWICH,
	SAUSAGE,
	NONE
};

struct MapDefinition {
	String name;
	String filename;
	String image;
	unsigned int size;
};