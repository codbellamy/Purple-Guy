#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "./PixelGame/Entity.h"
#include "./PixelGame/Camera.h"
#include "./PixelGame/json.hpp"
#include <istream>

class Game : public olc::PixelGameEngine
{
public:
	Game()
	{
		sAppName = "Purple Guy";
	}

public:
	bool OnUserCreate() override
	{
		std::cout << "Initializing..." << std::endl;

		//pack->AddFile("./Assets/data/leveldata.json");
		//pack->SavePack("./Assets/data/0.dat", resourcePass);

		this->loadLevel();

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		// Get the camera offsets
		cameraOffsets = player->getCamera()->getOffsets();

		// Clear previous frame
		Clear(olc::BLACK);

		// Input
		// Movement
		if (GetKey(olc::Key::W).bHeld)			player->move(Player::Move::UP);
		if (GetKey(olc::Key::S).bHeld)			player->move(Player::Move::DOWN);
		if (GetKey(olc::Key::A).bHeld)			player->move(Player::Move::LEFT);
		if (GetKey(olc::Key::D).bHeld)			player->move(Player::Move::RIGHT);

		// Debug and exit
		if (GetKey(olc::Key::F5).bPressed)		debugFlag = !debugFlag;
		if (GetKey(olc::ESCAPE).bHeld)			exit(0);

		// Update position
		player->updatePosition(fElapsedTime);

		// Draw map to the screen
		DrawSprite(cameraOffsets, mapSprite);

		// Update NPC positions and render
		SetPixelMode(olc::Pixel::ALPHA);

		// Update and render entities
		this->updateEntities(fElapsedTime);

		// Draw Player
		this->drawPlayer();

		// Reset pixel mode since drawing with alpha is computationally heavy
		SetPixelMode(olc::Pixel::NORMAL);
	}

private:

	// Constants
	const float spriteSize = 16.0f;
	const olc::vf2d spriteAdjust = { float(spriteSize) / 2, float(spriteSize) / 2 };
	const std::string resourcePass = "";

	olc::vf2d cameraOffsets;

	// Relative starting position for the player (this will adjust offsets accordingly)
	// {0, 0} will not offset anything... (the player will spawn at the normal center)
	olc::vf2d startingPos;

	// Player
	std::unique_ptr<Player> player;

	// Vector to hold all aditional entities
	std::vector<std::unique_ptr<Entity>> entities;

	// Sprite and image data
	olc::Sprite* mapSprite;

	// Look behind the curtain
	bool debugFlag = false;

	// Sprite and decal loaders

	// Resources
	olc::ResourcePack* pack = new olc::ResourcePack();

private:

	void loadLevel(int level=0) {
		// Quick cleanup
		delete mapSprite;

		using json = nlohmann::json;

		// Load the appropriate resource pack for the level
		pack->LoadPack("./Assets/data/" + std::to_string(level) + ".dat", resourcePass);

		// Load level data into input stream from buffer
		olc::ResourceBuffer rb = pack->GetFileBuffer("./Assets/data/leveldata.json");
		std::istream i(&rb);
		
		// Read level data
		json j;
		i >> j;
		j = j[std::to_string(level)];

		// Load the map sprite
		mapSprite = new olc::Sprite
			("./Assets/images/sprites/" + j["name"].get<std::string>() + ".png", pack);

		// Load player and set initial position
		startingPos = olc::vf2d(
			{ 
				j["player"]["location"][0].get<float>(),
				j["player"]["location"][1].get<float>()
			});
		player = std::make_unique<Player>(ScreenWidth(), ScreenHeight(), startingPos, 1000.0f);
		
		// Load player sprite
		std::string path;
		if (j["player"]["animated"].get<bool>()) {
			path = "./Assets/images/sprite_sheets/";
		}
		else {
			path = "./Assets/images/sprites/";
		}
		player->setDecal(path + j["player"]["skin"].get<std::string>() + ".png", pack);
		player->initAnimations({ 11, 7, 7, 7, 7 }, 8);

		// Load NPCs
		olc::vf2d ePos;
		for (auto& npc : j["npcs"]) {

			// Check if the entity is animated
			if (npc["animated"].get<bool>()) {
				path = "./Assets/images/sprite_sheets/";
			}
			else {
				path = "./Assets/images/sprites/";
			}

			// Skin for npc
			path += npc["skin"].get<std::string>() + ".png";

			// Location
			ePos = {npc["location"][0].get<float>(), npc["location"][1].get<float>()};

			// Init the NPC and assign decal from the path
			std::unique_ptr<NPC> newNPC = std::make_unique<NPC>(ePos, ScreenWidth(), ScreenHeight());
			newNPC->setDecal(path, pack); 

			// Add entity to the vector
			entities.push_back(std::move(newNPC));
		}
	}

	void drawPlayer(){
		// Get and adjust the position for the sprite
		olc::vf2d pos = player->getPos();
		olc::vf2d spriteSize = { player->r, player->r };
		olc::vf2d adjust = pos - spriteSize;

		// Get animation data for which frame to render
		std::pair<olc::vf2d, olc::vf2d> animationData = player->am->getPartialCoords();

		// Render the animation from the sprite sheet
		DrawPartialDecal(adjust, player->am->getDecal(), animationData.first, animationData.second);

		// Debug information (camera, player hitbox, bounds, etc.)
		if (debugFlag) {
			Entity::Boundary b = player->getBoundary();
			DrawLine(pos, olc::vf2d({ float(ScreenWidth()) / 2, float(ScreenHeight()) / 2 }), olc::RED);
			DrawRect(olc::vf2d({ b.xLower, b.yLower }), olc::vf2d({ b.xUpper - b.xLower, b.yUpper - b.yLower }), olc::RED);
			DrawCircle(olc::vf2d({ float(ScreenWidth()) / 2, float(ScreenHeight()) / 2 }), 7, olc::RED);
		}
	}

	void updateEntities(float fElapsedTime) {

		for (auto& e : entities) {

			// Get the entity's position
			olc::vf2d pos = e->getPos();

			// Dont render the entity if they are outside the screen boundaries
			if ((pos + cameraOffsets).x + e->r < 0
				|| (pos + cameraOffsets).x - e->r > ScreenWidth()
				|| (pos + cameraOffsets).y + e->r < 0
				|| (pos + cameraOffsets).y - e->r > ScreenHeight())
				continue;

			// Check for collision with player
			player->elasticCollision(e, cameraOffsets);

			// Check for collision with other entities
			for (auto& other : entities) {
				if (other == e) continue;
				else e->elasticCollision(other, cameraOffsets);
			}

			// Update entity's position
			e->updatePosition(fElapsedTime);

			// Entity specific actions and decal rendering
			switch (e->getType()) {

			case Entity::Type::NPC:


				// Draw the NPC with the npcDecal
				DrawDecal(pos - spriteAdjust + cameraOffsets, e->getDecal());

				// Debug visuals (boundaries and entity radius)
				if (debugFlag) {
					Entity::Boundary b = e->getBoundary();
					DrawCircle(pos + cameraOffsets, e->r, olc::BLUE);
					DrawRect(olc::vf2d({ b.xLower - e->r, b.yLower - e->r }) + cameraOffsets, olc::vf2d({ b.xUpper + e->r, b.yUpper + e->r }), olc::BLUE);
				}
				break;

			default:
				// Whoops
				std::cout << "No Entity::Type handler for type:" << e->getType() << std::endl;
				break;
			}
		}
	}

};

struct AspectRatio
{
	int x;
	int y;
};

int main()
{
	// Setup
	AspectRatio ratio	= { 16, 9 };	// Aspect ratio
	int pixel_size		= 3;			// How many display pixels should make up a virtual pixel?
	int real_width		= 1536;			// How many display pixels should the width be?

	// Calculate the width and height based on real width, aspect ratio, and pixel size
	int width			= real_width / pixel_size;
	int height			= (width * ratio.y) / ratio.x;

	// Initialize the game
	Game game;
	if (game.Construct(width, height, pixel_size, pixel_size, false, true))
		game.Start();

	return 0;
}
