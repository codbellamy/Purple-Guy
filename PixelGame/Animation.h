#pragma once

#include <vector>
#include <utility>

class AnimationManager {
public:
	AnimationManager(std::vector<int> animationCounts, int framesPerAnimation)
		: animations(animationCounts)
	{
		target = float(framesPerAnimation) / 60;
		numberOfAnimations = animationCounts.size();
	}

private:
	// Information about the animations
	int numberOfAnimations;
	float target;					// Determines when to increment the current frame
	std::vector<int> animations;

	// Current status
	int currentFrame = 0;
	int currentAnimation = 0;
	
	// Accumulated total time
	float accumulatedTime = 0.0f;

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

	// Get the current animation to display
	std::pair<int, int> getAnimation() { return std::make_pair(currentAnimation, currentFrame); }
};