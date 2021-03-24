#pragma once
#include "../olcPixelGameEngine.h"
#include "Camera.h"

class Entity {

public:
	// X = 0, Y = 1
	enum BounceAxis {
		X,
		Y
	};

	// Used to identify what the entity is
	// New entities should be added to the end
	enum Type {
		NONE,
		PLAYER,
		NPC
	};

	// Maximum x and y boundaries
	// Upper bound describes the largest value
	// Lower bound describes the smallest value
	struct Boundary {
		// Each boundary for how far the object can travel
		// Note that lower and upper refer to the integer values (not acutal position, this is inverted)
		float xLower;
		float xUpper;
		float yLower;
		float yUpper;
	};

public:
	// Preferred constructor to initialize a entity with specific values
	Entity(olc::vf2d iPos, olc::vf2d iVel, Boundary b, float mass, Type t)
		: pos(iPos), vel(iVel), b(b), m(mass), type(t) { }

	// Used when initializing a player (the camera will dictate the player's boundary)
	Entity(olc::vf2d iPos, olc::vf2d iVel, float mass, Type t)
		: pos(iPos), vel(iVel), m(mass), type(t) { }

	// Default constructor (used for testing)
	Entity()
		: Entity(
			olc::vf2d({ 50.0f, 50.0f }),
			olc::vf2d({ 0.0f, 0.0f }),
			Entity::Boundary({ 0, 100, 0, 100 }),
			100.0f,
			Type::NONE)
	{ }

public:
	// Specific behavior (limiters)
	int speedCap;
	float speed;
	float dampen;
	Boundary b;

	// General physics variables
	olc::vf2d pos;
	olc::vf2d vel;
	float m;
	const float spriteSize = 16;
	const float r = spriteSize / 2;

private:

	// Identifiers and flags
	Type type;

public:
	// Getters
	Boundary getBoundary()	{ return b; }
	Type getType()			{ return type; }

	// Perfectly elastic collision between entities
	void elasticCollision(Entity* e, Camera* c) {

		olc::vf2d posA, posB;

		// Remove offset from entity to determine if there is a collision
		if (this->getType() == PLAYER) {
			posA = pos;
			posB = e->pos + c->getOffsets();
		}
		else {	// Entities that are both offset do not need an offset correction (relative)
			posA = pos;
			posB = e->pos;
		}

		// If the two entities have collided
		if ((posB - posA).mag() < (e->r + r)) {

			// Calculations for v1
			float coeffA = (m - e->m) / (m + e->m);
			float coeffB = (2 * e->m) / (m + e->m);
			olc::vf2d result1 = (vel * coeffA) + (e->vel * coeffB);

			// Calculations for v2
			coeffA = (2 * m) / (m + e->m);
			coeffB = (e->m - m) / (m + e->m);
			olc::vf2d result2 = (vel * coeffA) + (e->vel * coeffB);

			// Apply velocities from collision
			vel = result1;
			e->vel = result2;

			// Player collisions have an offset that needs to be addressed
			if (this->getType() == PLAYER) {
				// Calculate player with offsets since the entities are offset
				pos = posB + (((posA - posB) / (posA - posB).mag()) * (r + e->r));

				// Remove offsets from both player position and entity position
				// since everything is relative to an offset origin
				posA -= c->getOffsets();
				posB -= c->getOffsets();
				e->pos = posA + (((posB - posA) / (posB - posA).mag()) * (r + e->r));
			}
			else {
				// Entity to entity interactions are all relative anyway
				pos = posB + (((posA - posB) / (posA - posB).mag()) * (r + e->r));
				e->pos = posA + (((posB - posA) / (posB - posA).mag()) * (r + e->r));
			}
		}
	}

	// Updates the ball's position based on global acceleration and the elapsed time
	virtual void updatePosition(float elapsedTime) {

		// Various checks and adjustments to velocity
		this->velDecay();
		this->speedCheck();

		pos += vel * elapsedTime;	// Update position

		this->collision();			// Check collision
	}

	// Invert the velocity based on which boundary it bounces on
	void bounce(int a) {
		switch (a)
		{
		case X:
			vel.x = -vel.x;
			break;
		case Y:
			vel.y = -vel.y;
			break;
		default:
			break;
		}
	}

	// Allows for directly adding velocity to the entity
	void increaseVel(olc::vf2d dVel) { vel += dVel; }

	// Handles collision
	virtual void collision() {
		// If the entity collides with a boundary
		if (pos.x < b.xLower || pos.x > b.xUpper) {
			pos.x < b.xLower ? pos.x = b.xLower : pos.x = b.xUpper;
			this->bounce(X);
		}
		if (pos.y < b.yLower || pos.y > b.yUpper) {
			pos.y < b.yLower ? pos.y = b.yLower : pos.y = b.yUpper;
			this->bounce(Y);
		}
	}

	// Control the speed of the entity
	virtual void speedCheck() {

		if (vel.mag2() > speedCap * speedCap) {
			vel = vel.norm() * speedCap;
		}

	}

	// How should the movement of the entity be dictated
	virtual void velDecay() {

		// Exponentially decrease speed when velocity is greater than 5 (smooth deceleration)
		if (vel.mag2() > 25) {
			vel -= vel * dampen;
		}
		else {
			vel.y = vel.x = 0;
		}
	}
};

class Player : public Entity {

public:
	// Allow the player to manipulate a camera object
	Camera* cam;

	enum Move {
		UP,
		DOWN,
		LEFT,
		RIGHT
	};

	Boundary mapBounds;

private:

	// These should remain the same values as those set using cam->setPanningOptions()
	// Changing these values independently between objects will have adverse effects on the illusion
	const int stopRadius = 49;	// r^2 from the center where the player's pos should stop being adjusted
	const float accel = 0.05f;	// How quickly should the player's position be corrected (0..1]

public:

	// Width and height are used to find the bounds for the camera.
	// An initial position is needed to provide an initial "starting position"
	// The iPos will directly manipulate the offset of the camera to
	// place the player anywhere on the map
	Player(int32_t w, int32_t h, olc::vf2d iPos, float mass=500.0f)
		: Entity(
			{ float(w)/2, float(h)/2 },		// Initial position
			{ 0.0f, 0.0f },					// Initial velocity
			mass,							// Entity mass
			Type::PLAYER){					// Entity type
		
		// Movement behavior
		dampen = 0.2f;
		speedCap = 250.0f;
		speed = 50.0f;

		cam = new Camera(w, h, -iPos);		// Initialize camera
		cam->setPanningOptions(stopRadius, accel);

		// Set the boundary of the player based on the camera
		b = {
			// These points are generated by the camera constructor
			cam->getPoint(0).x,
			cam->getPoint(1).x,
			cam->getPoint(0).y,
			cam->getPoint(1).y
		};

		// Set the map boundary
		mapBounds = { 0, float(w), 0, float(h) };
	}

	// Updates the position based on global acceleration and the elapsed time
	void updatePosition(float elapsedTime) {

		// Various checks and adjustments to velocity
		this->velDecay();
		this->speedCheck();

		pos += vel * elapsedTime;	// Update position

		this->cameraManip();		// Cool camera effects and illusions
		this->collision();			// Check collision with boundaries
	}

	// The player is attempting to move the player in a cardinal direction
	void move(Move m) {

		// TODO: use this to set flags on movement for animation
		switch (m)
		{
		case Player::UP:
			this->increaseVel({ 0.0f, -speed });
			break;
		case Player::DOWN:
			this->increaseVel({ 0.0f, speed });
			break;
		case Player::LEFT:
			this->increaseVel({ -speed, 0.0f });
			break;
		case Player::RIGHT:
			this->increaseVel({ speed, 0.0f });
			break;
		default:
			break;
		}
	}

private:

	// Manipulates the player's position along with the camera to create a smooth camera illusion
	void cameraManip() {

		// Adjust the camera based on current position
		cam->smooth(pos);

		// Find midpoint
		float midX = (b.xLower + b.xUpper) / 2;
		float midY = (b.yLower + b.yUpper) / 2;
		olc::vf2d midPoint = { midX, midY };

		// Calculate the distance from the midpoint
		olc::vf2d dist = midPoint - pos;

		// Determine how quickly to adjust the position based on distance from the midpoint
		if (dist.mag2() > stopRadius) {
			pos += (dist * accel);
		}
	}

	// Deals with collision with boundaries
	void collision() {

		// If the entity collides with a boundary
		if (pos.x < b.xLower || pos.x > b.xUpper) {
			pos.x < b.xLower ? pos.x = b.xLower : pos.x = b.xUpper;
		}
		if (pos.y < b.yLower || pos.y > b.yUpper) {
			pos.y < b.yLower ? pos.y = b.yLower : pos.y = b.yUpper;
		}
		
		// Map boundary
		olc::vf2d bounds = cam->getOffsets();
		if (pos.x - bounds.x - r < mapBounds.xLower || pos.x - bounds.x + r > mapBounds.xUpper) {
			pos.x - bounds.x - r < mapBounds.xLower ?
				pos.x = bounds.x + mapBounds.xLower + r: pos.x = bounds.x + mapBounds.xUpper - r;
		}
		if (pos.y - bounds.y - r < mapBounds.yLower || pos.y - bounds.y + r > mapBounds.yUpper) {
			pos.y - bounds.y - r < mapBounds.yLower ?
				pos.y = bounds.y + mapBounds.yLower + r: pos.y = bounds.y + mapBounds.yUpper - r;
		}
	}
};

class NPC : public Entity {

public:

	// Init the NPC as a child of Entity
	NPC(olc::vf2d pos, int32_t width, int32_t height, float mass = 100.0f)
		: Entity(	// position, velocity, boundary, mass, type
			pos,
			{ 0.0f, 0.0f },
			{ 0, 0, 0, 0 },
			mass,
			Type::NPC)
	{

		// NPC behavior
		dampen = 0.05f;
		speedCap = 50.0f;
		speed = 10.0f;

		// Map boundaries
		b = { r, (float)width - r, r, (float)height - r };

		// Init random movement variables
		moveTimerX = moveTimerY = 0;
		positiveX = positiveY = true;	// This will be reassessed each time the npc decides to move

	}

private:

	// Variables to assist with random movements
	int moveTimerX, moveTimerY;
	bool positiveX, positiveY;
	const int alpha = 200;			// 1/alpha probability to move each frame
	const int maxDist = 30;			// maximum distance that the npc can decide to move

public:
	void updatePosition(float elapsedTime) {

		// Various checks and adjustments to velocity
		this->velDecay();
		this->speedCheck();

		// Randomly decide if the NPC should move
		this->randMove();

		pos += vel * elapsedTime;	// Update position

		this->collision();			// Check collision
	
	}

private:

	void randMove() {

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
			positiveX ? vel.x += speed : vel.x -= speed;
		}
		if (moveTimerY > 0) {
			moveTimerY--;
			positiveY ? vel.y += speed : vel.y -= speed;
		}
	}
};