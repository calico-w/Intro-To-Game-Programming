/**
* Author:Calico Wang
* Assignment: Rise of the AI
* Date due: 2025-11-08, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/End.h"

// Global Constants
constexpr int SCREEN_WIDTH     = 1000,
              SCREEN_HEIGHT    = 600,
              FPS              = 120,
              NUMBER_OF_LEVELS = 6;

constexpr Vector2 ORIGIN      = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
            
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

Scene *gCurrentScene = nullptr;
std::vector<Scene*> gLevels = {};

Menu *gMenu = nullptr;
LevelA *gLevelA = nullptr;
LevelB *gLevelB = nullptr;
LevelC *gLevelC = nullptr;
End* gEnd = nullptr;

// Function Declarations
void switchToScene(Scene *scene);
void initialise();
void processInput();
void update();
void render();
void shutdown();

void switchToScene(Scene *scene)
{
    int  lives = 3;
    bool won;
    if (gCurrentScene) {
        const GameState& s = gCurrentScene->getState();
        lives = s.lives;
        won   = s.won;
    }
    gCurrentScene = scene;
    gCurrentScene->initialise();

    gCurrentScene->getState().lives = lives;
    gCurrentScene->getState().won   = won;
    gCurrentScene->getState().nextSceneID = 0;
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Scenes");
    InitAudioDevice();

    gMenu = new Menu(ORIGIN, "#C0897E");

    gLevelA = new LevelA(ORIGIN, "#C0897E");
    gLevelB = new LevelB(ORIGIN, "#011627");
    gLevelC = new LevelC(ORIGIN, "#011627");
    gEnd = new End(ORIGIN, "C0897E");

    gLevels.push_back(gMenu);

    gLevels.push_back(gLevelA);
    gLevels.push_back(gLevelB);
    gLevels.push_back(gLevelC);
    gLevels.push_back(gEnd);
    gLevels.push_back(gMenu);

    switchToScene(gLevels[0]);

    SetTargetFPS(FPS);
}

void processInput() 
{
    Entity* player = gCurrentScene->getState().pink;
    if (!player) {
        if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
        return;
    }
    gCurrentScene->getState().pink->resetMovement();
    if (gCurrentScene->getState().shroom) gCurrentScene->getState().shroom->resetMovement();
    if (gCurrentScene->getState().chicken) gCurrentScene->getState().chicken->resetMovement();
    if (gCurrentScene->getState().ghost) gCurrentScene->getState().ghost->resetMovement();

    if      (IsKeyDown(KEY_A)) gCurrentScene->getState().pink->moveLeft();
    else if (IsKeyDown(KEY_D)) gCurrentScene->getState().pink->moveRight();

    if (IsKeyPressed(KEY_W) && 
        gCurrentScene->getState().pink->isCollidingBottom())
    {
        gCurrentScene->getState().pink->jump();
    }

    if (GetLength(gCurrentScene->getState().pink->getMovement()) > 1.0f)
        gCurrentScene->getState().pink->normaliseMovement();

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() 
{
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        gCurrentScene->update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;
    }
}

void render()
{
    BeginDrawing();
    BeginMode2D(gCurrentScene->getState().camera);

    gCurrentScene->render();

    EndMode2D();
    EndDrawing();
}

void shutdown() 
{
    delete gMenu;
    delete gLevelA;
    delete gLevelB;
    delete gLevelC;

    for (int i = 0; i < NUMBER_OF_LEVELS; i++) gLevels[i] = nullptr;

    CloseAudioDevice();
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();

        if (gCurrentScene->getState().nextSceneID > 0)
        {
            int id = gCurrentScene->getState().nextSceneID;
            switchToScene(gLevels[id]);
        }

        render();
    }

    shutdown();

    return 0;
}