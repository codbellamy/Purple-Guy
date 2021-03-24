#pragma once
#include "../olcPixelGameEngine.h"
#include "Entity.h"

class Camera {

public:

	Camera(int32_t screenWidth, int32_t screenHeight, olc::vf2d iOffset) {
		// Screen dimensions
		screenDim = { float(screenWidth), float(screenHeight) };

		// Camera boundaries
		points[0] = { margin * screenWidth, margin * screenHeight };
		points[1] = { (boxSize + margin) * screenWidth, (boxSize + margin) * screenHeight };

		// Offset of entities that should be print to the screen (based change of position)
		offset[x] = iOffset.x + (float(screenWidth) / 2);
		offset[y] = iOffset.y + (float(screenHeight) / 2);
	}

private:
	enum Axis
	{
		x,
		y
	};

	olc::vf2d screenDim;		// Screen dimensions
	const float boxSize = 0.5f; // Percentage of the total window the bounding box should occupy
	// the margins are half of the remaining window (half for one side, half for the other)
	const float margin = 0.5f * (1 - boxSize);

	olc::vf2d points[2];		// Bounding upper-left and lower-right
	float offset[2];			// Offset x and y

	// These values should be set directly from the player to maintain a nice camera panning illusion
	int stopRadius;	// Radius from the center where the camera should stop panning
	float accel;	// Camera acceleration

public:

	// Adjust the behavior of the camera when it pans
	void setPanningOptions(int r, float a) {
		stopRadius = r;
		accel = a;
	}

	// Reterns the boundary point
	olc::vf2d getPoint(int point) {
		if (point < 0 || point > 1) {
			std::cout << "Invalid point" << std::endl;
			return { 0.0f, 0.0f };
		}
		else {
			return points[point];
		}
	}

	// Smoothy moves the camera based on how far the new point is from the center
	void smooth(olc::vf2d newPos) {
		olc::vf2d dist = (screenDim / 2) - newPos;
		if (dist.mag2() > stopRadius) {
			offset[x] +=  dist.x * accel;
			offset[y] +=  dist.y * accel;
		}
	}

	// Adjusts the offset using a box scheme
	void box(olc::vf2d newPos) {

		// Entity moves to the left of the boundary
		if (newPos.x <= points[0].x) {
			offset[x] += points[0].x - newPos.x;
		}

		// Entity moves to the right of the boundary
		if (newPos.x >= points[1].x) {
			offset[x] += points[1].x - newPos.x;
		}

		// Entity moves above the boundary
		if (newPos.y <= points[0].y) {
			offset[y] += points[0].y - newPos.y;
		}

		// Entity moves below the boundary
		if (newPos.y >= points[1].y) {
			offset[y] += points[1].y - newPos.y;
		}

	}

	// Get a specific offset value (0=x-axis, 1=y-axis)
	float getOffset(int axis) {
		if (axis < x || axis > y) {
			std::cout << "Invalid offset query" << std::endl;
			return 0.0f;
		}
		else {
			return offset[axis];
		}
	}

	// Get both offset values in the form of a vector
	olc::vf2d getOffsets() {
		return { offset[x], offset[y] };
	}
};