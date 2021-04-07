#pragma once
#include "../olcPixelGameEngine.h"
#include "Camera.h"
#include "Animation.h"

class Entity {

public:
	// X = 0, Y = 1
	enum BounceAxis {
		X,
		Y
	};

	// Used to identify what the entity is
	// New entities should be added to the end
	enum class Type {
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
	Entity(olc::vf2d, olc::vf2d, Boundary, float, Type);

	// Used when initializing a player
	Entity(olc::vf2d, olc::vf2d, float, Type);

	// Default constructor (used for testing)
	Entity();

	// Destructor
	~Entity();

public:
	const float spriteSize = 16;
	const float r = spriteSize / 2;

	// Animations
	AnimationManager* am;

private:
	// Specific behavior (limiters)
	// Public to allow each child class to manipulate these values directly
	float speedCap;
	float speed;
	float dampen;

	// General physics variables
	olc::vf2d pos;
	olc::vf2d vel;
	float m;
	
	// General boundaries (use these to set the outer most limits of the entity)
	Boundary b;

	// Identifiers and flags
	Type type;

	olc::Sprite* sprite;
	olc::Decal* decal;

public:
	// Getters
	Boundary getBoundary();
	int getIntType();
	Type getType();
	float getSpeed();
	olc::vf2d getPos();
	olc::vf2d getVel();
	float getMass();
	olc::Decal* getDecal();

	// Setters
	void setSpeed(float);
	void setSpeedCap(float);
	void setPos(olc::vf2d);
	void setVel(olc::vf2d);
	void setMass(float);
	void increasePos(olc::vf2d);
	void increaseVel(olc::vf2d);
	void setDecal(std::string, olc::ResourcePack*);

	// Change movement characteristics in one go
	void setPhysics(float, float, float);

	// specify a different boundary
	void updateBoundary(Boundary);

	// Perfectly elastic collision between entities
	void elasticCollision(std::unique_ptr<Entity>&, olc::vf2d);

	// Invert the velocity based on which boundary it bounces on
	void bounce(int);

	// Updates the ball's position based on global acceleration and the elapsed time
	virtual void updatePosition(float);

	// Handles collision
	virtual void collision();

	// Control the speed of the entity
	virtual void speedCheck();

	// How should the movement of the entity be dictated
	virtual void velDecay();

	// Set up the animations for the entity
	void initAnimations(std::vector<int>, int);
};

class Player : public Entity {

public:

	enum class Move {
		UP,
		DOWN,
		LEFT,
		RIGHT
	};

private:

	// These should remain the same values as those set using cam->setPanningOptions()
	// Changing these values independently between objects will have adverse effects on the illusion
	const int stopRadius = 49;	// r^2 from the center where the player's pos should stop being adjusted
	const float accel = 0.05f;	// How quickly should the player's position be corrected (0..1]

	// Player uses seperate map boundaries from 'b' since 'b' is used to 
	// dictate how far from center the player should be able to move
	// relative to the camera
	Boundary mapBounds;

	// Allow the player to manipulate a camera object
	Camera* cam;

public:

	// Width and height are used to find the bounds for the camera.
	// An initial position is needed to provide an initial "starting position"
	// The iPos will directly manipulate the offset of the camera to
	// place the player anywhere on the map
	Player(int32_t, int32_t, olc::vf2d, float);

	~Player();

public:

	// Getters
	Camera* getCamera();

	// Overwrites from parent class since the player can manipulate the camera
	void updatePosition(float);

	// The player is attempting to move the player in a cardinal direction
	void move(Move);

private:

	// Manipulates the player's position along with the camera to create a smooth camera illusion
	void cameraManip();

	// Deals with collision with boundaries
	void collision();
};

class NPC : public Entity {

public:

	// Init the NPC as a child of Entity
	NPC(olc::vf2d, int32_t, int32_t, float mass = 100.0f);

private:

	// Variables to assist with random movements
	int moveTimerX, moveTimerY;
	bool positiveX, positiveY;
	const int alpha = 200;			// 1/alpha probability to move each frame
	const int maxDist = 30;			// maximum distance that the npc can decide to move

public:
	void updatePosition(float);

private:

	void randMove();
};