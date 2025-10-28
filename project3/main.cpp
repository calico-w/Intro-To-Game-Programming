/**
* Author: Calico Wang
* Assignment: Lunar Lander
* Date due: 2025-10-13, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/Entity.h"

// Global Constants
constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120;

constexpr char    BG_COLOUR[] = "#C0897E";
constexpr Vector2 ORIGIN      = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

constexpr int	NUMBER_OF_BLOCKS        = 3;
constexpr float TILE_DIMENSION          = 50.0f,
                // in m/ms², since delta time is in ms
                ACCELERATION_OF_GRAVITY = 50.0f,
                FIXED_TIMESTEP          = 1.0f / 60.0f,
                END_GAME_THRESHOLD      = 800.0f;

// Global Variables
enum GameResult {START, WIN, LOSE };
GameResult gResult = START;
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f,
	  gMaxFuel = 100.0f;
float gCurrentFuel = 100.0f;

Entity *gPlane = nullptr;
Entity *gDestinations  = nullptr;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

bool haveFuel(){return gCurrentFuel > 0.0f;}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PaperPlane Lander");

    float sizeRatio  = 48.0f / 64.0f;

    // Assets from @see https://sscary.itch.io/the-adventurer-female
    gPlane = new Entity(
        {ORIGIN.x - 300.0f, ORIGIN.y - 200.0f}, // position
        {100.0f * sizeRatio, 100.0f},  // scale
		"assets/game/plane.png",
        PLAYER                         // entity type
    );

    gPlane->setColliderDimensions({
        gPlane->getScale().x/1.5f,
        gPlane ->getScale().y/1.5f
    });
    gPlane->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    gDestinations = new Entity[NUMBER_OF_BLOCKS];

    for (int i = 0; i < NUMBER_OF_BLOCKS; i++)
    {
        gDestinations[i].setTexture("assets/game/tile_0061.png");
        gDestinations[i].setEntityType(BLOCK);
        gDestinations[i].setScale({TILE_DIMENSION, TILE_DIMENSION});
        gDestinations[i].setColliderDimensions({TILE_DIMENSION, TILE_DIMENSION});
    }

    gDestinations[0].setPosition({ORIGIN.x - TILE_DIMENSION * 3, ORIGIN.y + 200.0f});
    gDestinations[1].setPosition({ORIGIN.x, ORIGIN.y - TILE_DIMENSION * 1.5f});
    gDestinations[2].setPosition({ORIGIN.x + TILE_DIMENSION * 3, ORIGIN.y});
    gDestinations[1].setAcceleration({20.0f, 0.0f});

    SetTargetFPS(FPS);
}

void processInput() 
{
	if (gResult != WIN && gResult != LOSE){
		Vector2 acceleration = {0.0f,ACCELERATION_OF_GRAVITY};

    	if      (IsKeyDown(KEY_A) && haveFuel()) {
			acceleration.x -= 200.0f;
			gCurrentFuel -= 2.0f * FIXED_TIMESTEP;
		}
    	else if (IsKeyDown(KEY_D) && haveFuel()) {
			acceleration.x += 200.0f;
			gCurrentFuel -= 2.0f * FIXED_TIMESTEP;
		}

		if (IsKeyDown(KEY_SPACE) && haveFuel()) {
			acceleration.y -= 150.0f;
			gCurrentFuel -= 5.0f * FIXED_TIMESTEP;
		}
    	acceleration.x -= gPlane->getVelocity().x * 5.0f;
    	gPlane->setAcceleration(acceleration);

		if (gCurrentFuel <= 0.0f) gCurrentFuel = 0.0f;
	}
    	if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() 
{
    // Delta time
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    // Fixed timestep
    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }
	gTimeAccumulator = 0.0f;

    while (deltaTime >= FIXED_TIMESTEP)
    {
		Vector2 p = gDestinations[1].getPosition();
		if (p.x >= ORIGIN.x + 100.0f) { gDestinations[1].setAcceleration({-20.0f, 0.0f}); }
		else if (p.x <= ORIGIN.x - 100.0f) { gDestinations[1].setAcceleration({20.0f, 0.0f}); }
        gPlane->update(FIXED_TIMESTEP, nullptr, gDestinations,0, NUMBER_OF_BLOCKS);

		if (gPlane->isCollidingBottom()) {
		gResult = WIN;
		}

        for (int i = 0; i < NUMBER_OF_BLOCKS; i++) 
        {
            gDestinations[i].update(FIXED_TIMESTEP, nullptr, nullptr, 0, 0);
        }


        deltaTime -= FIXED_TIMESTEP;
    }


    if (gPlane->getPosition().y > END_GAME_THRESHOLD) gResult = LOSE;
	if (gResult == WIN || gResult == LOSE) {
		gPlane->setAcceleration({0.0f, 0.0f});
	}

}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

    gPlane->render();

    for (int i = 0; i < NUMBER_OF_BLOCKS; i++) gDestinations[i].render();

	DrawText(TextFormat("Fuel: %f", gCurrentFuel), 20.0f, 20.0f, 20.0f, WHITE);
	if (gResult == GameResult::WIN) {
    	DrawText("Mission Accomplished", ORIGIN.x - 100.0f, ORIGIN.y, 50.0f, WHITE);
	} else if (gResult == GameResult::LOSE) {
    	DrawText("Mission Failed", ORIGIN.x - 100.0f, ORIGIN.y, 50.0f, WHITE);
	}

    EndDrawing();
}

void shutdown() 
{ 
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}