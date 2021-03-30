#pragma once

#include "../olcPixelGameEngine.h"
#include <vector>
#include <utility>

class AnimationManager {
public:
	AnimationManager(std::vector<int> animationCounts, int framesPerAnimation, std::string file)
		: animations(animationCounts)
	{
		target = float(framesPerAnimation) / 60;
		numberOfAnimations = animationCounts.size();
		original = new olc::Sprite(file);
		decal = new olc::Decal(original);
	}

	~AnimationManager()
	{
		delete original;
		delete decal;
	}

private:
	// Information about the animations
	int numberOfAnimations;
	float target;					// Determines when to increment the current frame
	std::vector<int> animations;
	const int spriteSize = 16;

	// Current status
	int currentFrame = 0;		// [0, nFrames-1]
	int currentAnimation = 0;	// [0, numberOfAnimations.size()]
	
	// Accumulated total time
	float accumulatedTime = 0.0f;

	// Sprite and decal
	olc::Sprite* original;
	olc::Decal* decal;

public:
	
	// Update the current frame number
	void updateAnimation(float elapsedTime) {
		
		accumulatedTime += elapsedTime;
		if (accumulatedTime >= target) {
			accumulatedTime = fmod(accumulatedTime, target);
			currentFrame++;
			currentFrame %= animations[currentAnimation];
		}
	}

	// Choose which animation frame to return
	void selectAnimation(int animationIndex) {
		currentAnimation = animationIndex < numberOfAnimations ? animationIndex : 0;
		currentFrame = 0;
	}

	// Get the current decal to display
	olc::Decal* getDecal()
	{ 
		return decal;
	}

	// We will need to index our sprite sheet for the current frame
	// The row is the current animation and the column is the current frame
	// void DrawPartialDecal(const olc::vf2d& pos, olc::Decal* decal, const olc::vf2d& source_pos, const olc::vf2d& source_size, const olc::vf2d& scale = { 1.0f,1.0f }, const olc::Pixel& tint = olc::WHITE);
	std::pair<olc::vf2d, olc::vf2d> getPartialCoords()
	{
		olc::vf2d source_pos = { float(currentFrame * spriteSize), float(currentAnimation * spriteSize)};
		olc::vf2d source_size = { float(spriteSize), float(spriteSize) };
		return std::make_pair(source_pos, source_size);
	}
};