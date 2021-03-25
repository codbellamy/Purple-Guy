#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "./PixelGame/Entity.h"

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
		// Load the map sprite
		mapSprite = new olc::Sprite(mapPath);

		// Create the player and load the decal for the player
		player = new Player(ScreenWidth(), ScreenHeight(), startingPos, 1000.0f);
		charSprite = new olc::Sprite(charPath);
		charDecal = new olc::Decal(charSprite);

		// Load the npc decal
		npcSprite = new olc::Sprite(npcPath);
		npcDecal = new olc::Decal(npcSprite);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Get mouse position for creating new npcs (on click)
		olc::vf2d mouse = { float(GetMouseX()), float(GetMouseY()) };
		mouse -= player->cam->getOffsets();

		// Clear previous frame
		Clear(olc::BLACK);

		// Input
		if (GetKey(olc::Key::W).bHeld)			player->move(Player::Move::UP);
		if (GetKey(olc::Key::S).bHeld)			player->move(Player::Move::DOWN);
		if (GetKey(olc::Key::A).bHeld)			player->move(Player::Move::LEFT);
		if (GetKey(olc::Key::D).bHeld)			player->move(Player::Move::RIGHT);
		if (GetMouse(0).bPressed)				entities.push_back(new NPC(mouse, ScreenWidth(), ScreenHeight()));
		if (GetMouse(1).bHeld)					entities.push_back(new NPC(mouse, ScreenWidth(), ScreenHeight()));
		if (GetKey(olc::Key::F5).bPressed)		debugFlag = !debugFlag;
		if (GetKey(olc::ESCAPE).bHeld)			exit(0);

		// Update position
		player->updatePosition(fElapsedTime);

		// Draw map to the screen
		DrawSprite(player->cam->getOffsets(), mapSprite);

		// Update NPC positions and render
		SetPixelMode(olc::Pixel::ALPHA);
		for (Entity* e : entities) {

			// Check for collision with player
			player->elasticCollision(e, player->cam);
			
			// Check for collision with other entities
			for (Entity* other : entities) {
				if (other == e) continue;
				else e->elasticCollision(other, player->cam);
			}
			
			// Update entity's position
			e->updatePosition(fElapsedTime);

			// Entity specific actions and decal rendering
			switch (e->getType()) {

			case Entity::Type::NPC:
				DrawDecal(e->pos - spriteAdjust + player->cam->getOffsets(), npcDecal);
				if (debugFlag) {
					Entity::Boundary b = e->getBoundary();
					DrawCircle(e->pos + player->cam->getOffsets(), e->r, olc::BLUE);
					DrawRect(olc::vf2d({ b.xLower - e->r, b.yLower - e->r}) + player->cam->getOffsets(), olc::vf2d({ b.xUpper + e->r, b.yUpper  + e->r}), olc::BLUE);
				}
				break;

			default:
				std::cout << "No Entity::Type handler for type:" << e->getType() << std::endl;
				break;
			}
		}

		// Draw Player
		olc::vf2d adjust = player->pos - olc::vf2d({ float(spriteSize) / 2, float(spriteSize) / 2 });
		DrawDecal(adjust, charDecal);
		SetPixelMode(olc::Pixel::NORMAL);

		// Debug information
		if (debugFlag) {
			Entity::Boundary b = player->getBoundary();
			DrawLine(player->pos, olc::vf2d({ float(ScreenWidth()) / 2, float(ScreenHeight()) / 2 }), olc::RED);
			DrawCircle(player->pos, player->r, olc::RED);
			DrawRect(olc::vf2d({ b.xLower, b.yLower }), olc::vf2d({ b.xUpper - b.xLower, b.yUpper - b.yLower }), olc::RED);
			DrawCircle(olc::vf2d({ float(ScreenWidth()) / 2, float(ScreenHeight()) / 2 }), 7, olc::RED);
		}
	}

private:

	Camera* cam;					// Camera
	Player* player;					// Player
	std::vector<Entity*> entities;	// Vector to hold all aditional entities

	// Sprite and image data
	std::string		charPath	= "./Assets/images/sprites/Character.png";
	std::string		mapPath		= "./Assets/images/sprites/TestMap.png";
	std::string		npcPath		= "./Assets/images/sprites/NPC.png";

	bool debugFlag = false;

	// Sprite and decal loaders
	olc::Sprite*	charSprite;
	olc::Sprite*	npcSprite;
	olc::Sprite*	mapSprite;
	olc::Decal*		charDecal;
	olc::Decal*		npcDecal;

	// Constants
	const float spriteSize = 16.0f;
	const olc::vf2d spriteAdjust = { float(spriteSize) / 2, float(spriteSize) / 2 };

	// Relative starting position for the player (this will adjust offsets accordingly)
	// {0, 0} will place the player at the top left of the map
	const olc::vf2d startingPos = { float(ScreenWidth()) / 2, float(ScreenHeight()) / 2 };
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
