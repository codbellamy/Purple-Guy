#include "Entity.h"

// Preferred constructor to initialize a entity with specific values
Entity::Entity(olc::vf2d iPos, olc::vf2d iVel, Boundary b, float mass, Type t)
	: pos(iPos), vel(iVel), b(b), m(mass), type(t) { }

// Used when initializing a player
Entity::Entity(olc::vf2d iPos, olc::vf2d iVel, float mass, Type t)
	: pos(iPos),
	vel(iVel),
	m(mass),
	type(t)
{
	b = { 0,0,2000,2000 };
	this->setPhysics(50.0f, 20.0f, 0.05f);
}

// Default constructor (used for testing)
Entity::Entity()
	: pos({ 50.0f, 50.0f }),
	vel({ 0.0f, 0.0f }),
	b({ 0, 100, 0, 100 }),
	m(100.0f), type(NONE)

{
	this->setPhysics(100.0f, 50.0f, 0.05f);
}

// Getters
Entity::Boundary Entity::getBoundary() { return b; }
Entity::Type Entity::getType() { return type; }
float Entity::getSpeed() { return speed; }
olc::vf2d Entity::getPos() { return pos; }
olc::vf2d Entity::getVel() { return vel; }
float Entity::getMass() { return m; }

// Setters
void Entity::setSpeed(float newSpeed) { speed = newSpeed; }
void Entity::setSpeedCap(float newSpeedCap) { speedCap = newSpeedCap; }
void Entity::setPos(olc::vf2d newPos) { pos = newPos; }
void Entity::setVel(olc::vf2d newVel) { vel = newVel; }
void Entity::setMass(float newMass) { m = newMass; }
void Entity::increasePos(olc::vf2d deltaPos) { pos += deltaPos; }
void Entity::increaseVel(olc::vf2d deltaVel) { vel += deltaVel; }
void Entity::updateBoundary(Boundary newBoundary) { b = newBoundary; }
void Entity::setPhysics(float newSpeedCap, float newSpeed, float newDampen) {
	speedCap = newSpeedCap;
	speed = newSpeed;
	dampen = newDampen;
}

void Entity::elasticCollision(std::unique_ptr<Entity>& e, olc::vf2d offsets) {

	olc::vf2d posA, posB;

	// Remove offset from entity to determine if there is a collision
	if (this->getType() == PLAYER) {
		posA = pos;
		posB = e->pos + offsets;
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
			pos = posB + ((posA - posB).norm() * (r + e->r));

			// Remove offsets from both player position and entity position
			// since everything is relative to an offset origin
			posA -= offsets;
			posB -= offsets;
			e->pos = posA + ((posB - posA).norm() * (r + e->r));
		}
		else {
			// Entity to entity interactions are all relative anyway
			pos = posB + ((posA - posB).norm() * (r + e->r));
			e->pos = posA + ((posB - posA).norm() * (r + e->r));
		}
	}
}
void Entity::bounce(int a) {
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

// Virtual functions that will likely need to be overwritten for child classes
void Entity::updatePosition(float elapsedTime) {

	// Various checks and adjustments to velocity
	this->velDecay();
	this->speedCheck();

	pos += vel * elapsedTime;	// Update position

	this->collision();			// Check collision with boundary
}
void Entity::collision() {
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
void Entity::speedCheck() {

	if (vel.mag2() > speedCap * speedCap) {
		vel = vel.norm() * speedCap;
	}

}
void Entity::velDecay() {

	// Exponentially decrease speed when velocity is greater than 5 (smooth deceleration)
	if (vel.mag2() > 25) {
		vel -= vel * dampen;
	}
	else {
		vel.y = vel.x = 0;
	}
}