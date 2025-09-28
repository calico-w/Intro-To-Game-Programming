/**
* Author: Calico Wang
* Assignment: Simple 2D Scene
* Date due: 2025-09-27, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#include "CS3113/cs3113.h"
#include <math.h>

// Global Constants
constexpr int   SCREEN_WIDTH  = 800,
                SCREEN_HEIGHT = 800,
                FPS           = 60;
constexpr float MAX_AMP       = 10.0f,
                SUN_SPEED = 1.0f,
                CLOUD_SPEED = 0.008f,
                CLOUD_ORBIT_RADIUS = 100.0f,
                TREE_MAX_SCALE = 2.0f;

constexpr char    BG_COLOUR[] = "#87CEEB";
constexpr Vector2 ORIGIN      = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
constexpr Vector2 SUN_BASE_SIZE   = { 250.0f, 250.0f },
                  CLOUD_BASE_SIZE = {500.0f, 250.0f},
                  TREE_BASE_SIZE = {200.0f, 400.0f},
                  SUN_INIT_POS  = { ORIGIN.x-200.0f, ORIGIN.y - 200.0f },
                  TREE_INIT_POS    = { ORIGIN.x, ORIGIN.y + 500.0f },
                  CLOUD_INIT_POS = { ORIGIN.x + 200.0f, ORIGIN.y - 115.0f };

constexpr char SUN_FP[]  = "assets/sun.png";
constexpr char CLOUD_FP[]  = "assets/cloud.png";
constexpr char TREE_FP[]  = "assets/tree.png";

// Global Variables
AppStatus gAppStatus     = RUNNING;
float     gSunAngle = 0.0f,
          gCloudAngle = 0.0f,
          gPulseTime     = 0.0f,
          gGrowFactor = 1.0f,
          gOrbitLocation = 0.0f;
Vector2   gPosition      = ORIGIN,
          gSunPos = SUN_INIT_POS,
          gCloudPos = CLOUD_INIT_POS,
          gTreePos = TREE_INIT_POS;
Vector2   gSunScale         = SUN_BASE_SIZE,
          gCloudScale = CLOUD_BASE_SIZE,
          gTreeScale = TREE_BASE_SIZE;
float     gPreviousTicks = 0.0f,
          gTime = 0.0f;

Texture2D gSunTexture,
          gCloudTexture,
          gTreeTexture;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

void renderObject(const Texture2D *texture, const Vector2 *position,
                  const Vector2 *scale, float angle = 0.0f)
{
    // Whole texture (UV coordinates)
    Rectangle textureArea = {
        // top-left corner
        0.0f, 0.0f,

        // bottom-right corner (of texture)
        static_cast<float>(texture->width),
        static_cast<float>(texture->height)
    };

    // Destination rectangle – centred on gPosition
    Rectangle destinationArea = {
        position->x,
        position->y,
        static_cast<float>(scale->x),
        static_cast<float>(scale->y)
    };

    // Origin inside the source texture (centre of the texture)
    Vector2 originOffset = {
        static_cast<float>(scale->x) / 2.0f,
        static_cast<float>(scale->y) / 2.0f
    };

    // Render the texture on screen
    DrawTexturePro(
        *texture,
        textureArea, destinationArea, originOffset,
        angle, WHITE
    );
}

// Function Definitions
void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Simple 2D Scene");

    gSunTexture = LoadTexture(SUN_FP);
    gCloudTexture = LoadTexture(CLOUD_FP);
    gTreeTexture = LoadTexture(TREE_FP);

    SetTargetFPS(FPS);
}

void processInput()
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update()
{
    // Delta time
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    gTime += deltaTime;
    gPulseTime += deltaTime;

    //sun movements
    gSunScale = {
        SUN_BASE_SIZE.x + MAX_AMP * cos(gPulseTime),
        SUN_BASE_SIZE.y + MAX_AMP * cos(gPulseTime)
    };
    gSunAngle += 10.0f * deltaTime;
    gSunPos.x = (ORIGIN.x - 180.0f) + 100.0f * 1/2*sin(gTime*SUN_SPEED);
    gSunPos.y = (ORIGIN.y - 200.0f) + 100.0f * 1/2*sin(gTime*SUN_SPEED);

    //cloud movements
    gOrbitLocation += CLOUD_SPEED;
    gCloudPos.x = CLOUD_ORBIT_RADIUS * cos(gOrbitLocation) + gSunPos.x;
    gCloudPos.y = CLOUD_ORBIT_RADIUS * sin(gOrbitLocation) + gSunPos.y;
    gCloudAngle = sin(gTime) * 5.0f;

    //tree
    if (gGrowFactor < TREE_MAX_SCALE) gGrowFactor += 0.5f * deltaTime;
    gTreeScale = {
        TREE_BASE_SIZE.x * gGrowFactor,
        TREE_BASE_SIZE.y * gGrowFactor
    };
    if (gTreePos.y > 500.0f) gTreePos.y -= 100.0f * deltaTime;
    gTreePos.x = TREE_INIT_POS.x + 10.0f * sin(3.0f*gTime);
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

    // render teardrop
    renderObject(&gSunTexture, &gSunPos, &gSunScale, gSunAngle);
    renderObject(&gCloudTexture, &gCloudPos, &gCloudScale,gCloudAngle);
    renderObject(&gTreeTexture, &gTreePos, &gTreeScale);


    EndDrawing();
}

void shutdown() { CloseWindow(); }

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