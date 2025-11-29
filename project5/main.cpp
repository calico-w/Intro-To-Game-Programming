/**
* Author: Calico Wang
* Assignment: Forest Adventure
* Date due: 11/26/2025, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/ShaderProgram.h"



// Global Constants
constexpr int SCREEN_WIDTH     = 1000,
              SCREEN_HEIGHT    = 600,
              FPS              = 120,
              NUMBER_OF_LEVELS = 7;

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
Win* gWin = nullptr;
Lose* gLose = nullptr;

ShaderProgram gShader;
Vector2 gLightPosition = { 0.0f, 0.0f };


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
    if (gCurrentScene) {
        const GameState& s = gCurrentScene->getState();
        lives = s.lives;
    }
    gCurrentScene = scene;
    gCurrentScene->initialise();

    gCurrentScene->getState().lives = lives;
    gCurrentScene->getState().nextSceneID = 0;
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Forest Adventure");
    InitAudioDevice();

    gShader.load("shaders/vertex.glsl", "shaders/fragment.glsl");

    gMenu = new Menu(ORIGIN, "#011627");

    gLevelA = new LevelA(ORIGIN, "#011627");
    gLevelB = new LevelB(ORIGIN, "#011627");
    gLevelC = new LevelC(ORIGIN, "#011627");
    gWin = new Win(ORIGIN, "#011627");
    gLose = new Lose(ORIGIN, "#011627");

    gLevels.push_back(gMenu);

    gLevels.push_back(gLevelA);
    gLevels.push_back(gLevelB);
    gLevels.push_back(gLevelC);
    gLevels.push_back(gWin);
    gLevels.push_back(gLose);
    gLevels.push_back(gMenu);

    switchToScene(gLevels[0]);

    SetTargetFPS(FPS);
}

void processInput() 
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) {
        gAppStatus = TERMINATED;
        return;
    }
    if (IsKeyPressed(KEY_ONE)) {           // go to Level A
        gCurrentScene->getState().nextSceneID = 1;
        return;
    }
    if (IsKeyPressed(KEY_TWO)) {           // go to Level B
        gCurrentScene->getState().nextSceneID = 2;
        return;
    }
    if (IsKeyPressed(KEY_THREE)) {         // go to Level C
        gCurrentScene->getState().nextSceneID = 3;
        return;
    }
    Entity* player = gCurrentScene->getState().pink;
    if (!player) {
        if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
        return;
    }

    GameState& state = gCurrentScene->getState();

    state.pink->resetMovement();
    if (state.shroom)  state.shroom->resetMovement();
    if (state.chicken) state.chicken->resetMovement();
    if (state.ghost)   state.ghost->resetMovement();

    //Top-down movement: WASD
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))   state.pink->moveLeft();
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))  state.pink->moveRight();
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))     state.pink->moveUp();
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))   state.pink->moveDown();

    // Normalise diagonal speed
    if (GetLength(state.pink->getMovement()) > 1.0f)
        state.pink->normaliseMovement();
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
        if (gCurrentScene->getState().pink) gLightPosition = gCurrentScene->getState().pink->getPosition();
        deltaTime -= FIXED_TIMESTEP;
    }
}

void render()
{
    BeginDrawing();

    if (gCurrentScene == gMenu || gCurrentScene == gWin || gCurrentScene == gLose) {
        gCurrentScene->render();
    }
    else {
        BeginMode2D(gCurrentScene->getState().camera);

        gShader.begin();
        gShader.setVector2("lightPosition", gLightPosition);

        gCurrentScene->render();

        gShader.end();
        EndMode2D();
    }

    EndDrawing();
}

void shutdown() 
{
    delete gMenu;
    delete gLevelA;
    delete gLevelB;
    delete gLevelC;
    delete gWin;
    delete gLose;

    for (int i = 0; i < NUMBER_OF_LEVELS; i++) gLevels[i] = nullptr;
    gShader.unload();

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