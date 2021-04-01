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
	int currentAnimation = 0;	// [0, numberOfAnimations.size()-1]
	bool isPlaying = false;
	bool isIdle = false;
	int alpha = 1;
	int accumulatedFrames = 0;
	
	// Accumulated total time
	float accumulatedTime = 0.0f;

	// Sprite and decal
	olc::Sprite* original;
	olc::Decal* decal;

public:

	// 1/alpha probability that animation-0 will play
	void idle(int alpha) { 
		if (isIdle) return;
		isPlaying = false;
		isIdle = true;
		this->alpha = alpha;
	}
	void play() { 
		if (isPlaying) return;
		isPlaying = true; 
		isIdle = false;
	}
	
	// Update the current frame number
	void updateAnimation(float elapsedTime) {
		
		accumulatedTime += elapsedTime;

		if (accumulatedTime >= target) {
			accumulatedTime = fmod(accumulatedTime, target);

			if (isPlaying || accumulatedFrames > 0) {

				if (isPlaying) {
					currentFrame++;
					currentFrame %= animations[currentAnimation];
				}
				else {
					currentFrame = animations[currentAnimation] - accumulatedFrames;
					accumulatedFrames--;
				}
			}
			else {
				currentFrame = 0;
			}

			if (isIdle && (rand() % alpha == 0) && accumulatedFrames <= 0) {
				accumulatedFrames = animations[0];
			}
		}
	}

	// Choose which animation to play
	void selectAnimation(int animationIndex) {

		if (animationIndex != currentAnimation) {
			currentAnimation = animationIndex < numberOfAnimations ? animationIndex : 0;
			currentFrame = 0;
		}
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