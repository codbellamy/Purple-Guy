#include "Entity.h"

NPC::NPC(olc::vf2d pos, int32_t width, int32_t height, float mass)
	: Entity(	// position, velocity, boundary, mass, type
		pos,
		{ 0.0f, 0.0f },
		{ 0, 0, 0, 0 },
		mass,
		Type::NPC)
{
	// NPC behavior
	this->setPhysics(50.0f, 10.0f, 0.05f);

	// Map boundaries
	this->updateBoundary({ r, (float)width - r, r, (float)height - r });

	// Init random movement variables
	moveTimerX = moveTimerY = 0;
	positiveX = positiveY = true;	// This will be reassessed each time the npc decides to move

}

// Virtual functions
void NPC::updatePosition(float elapsedTime) {

	// Various checks and adjustments to velocity
	this->velDecay();
	this->speedCheck();

	// Randomly decide if the NPC should move
	this->randMove();

	// Update position
	this->increasePos(this->getVel() * elapsedTime);

	this->collision();			// Check collision

}

// Private functions
void NPC::randMove() {

	// Should the NPC decide to move?
	// 1/alpha chance per frame to decide to move
	if (rand() % alpha == 0) {

		// Choose axis parameters
		positiveX = rand() % 2 == 0;
		positiveY = rand() % 2 == 0;
		int direction = rand() % 3;

		// Based on directional decisions
		switch (direction)
		{
		case 0:		// x-axis
			moveTimerX += rand() % maxDist;
			break;

		case 1:		// x-axis and y-axis (case 1 and 2)
			moveTimerX += rand() % maxDist;

		case 2:		// y-axis only
			moveTimerY += rand() % maxDist;
			break;

			// This should never actually happen
		default:
			break;
		}
	}

	// As long as the move timer is positive for the axis...
	if (moveTimerX > 0) {



		// Adjust velocity
		moveTimerX--;
		positiveX ? this->increaseVel({ this->getSpeed(), 0.0f }) : this->increaseVel({ -this->getSpeed(), 0.0f });
	}
	if (moveTimerY > 0) {
		moveTimerY--;
		positiveY ? this->increaseVel({ 0.0f, this->getSpeed() }) : this->increaseVel({ 0.0f, -this->getSpeed() });
	}
}