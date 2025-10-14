/**
* Author: [Your name here]
* Assignment: Pong Clone
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
const Vector2 PADDLE_SIZE = { 50.0f, 120.0f };
const Vector2 BALL_SIZE = { 20.0f, 20.0f };


constexpr int   MAX_BALLS = 3;
constexpr float FIXED_TIMESTEP          = 1.0f / 60.0f,
                END_GAME_THRESHOLD      = 800.0f;

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

Entity *gPaddleL = nullptr;
Entity *gPaddleR   = nullptr;
Entity *gBalls  = nullptr;

Vector2 gBallV[MAX_BALLS];
int   BALLS_ACTIVE = 1;

bool RightAI = false;


// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

void activateBalls(int num) {
    BALLS_ACTIVE = num;
    for (int i = 0; i < MAX_BALLS; ++i) {
        if (i < num) {gBalls[i].activate();}
        else gBalls[i].deactivate();
    }
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong");

    gPaddleL = new Entity(
        { 60.0f, (float)SCREEN_HEIGHT / 2.0f },
        PADDLE_SIZE,
        "assets/racket.png",
        PLAYER
    );
    gPaddleL->setColliderDimensions(PADDLE_SIZE);
    gPaddleL->setAcceleration({0,0});

    gPaddleR = new Entity(
        { (float)SCREEN_WIDTH - 60.0f, (float)SCREEN_HEIGHT / 2.0f },
        PADDLE_SIZE,
        "assets/racket.png",
        PLAYER
    );
    gPaddleR->setColliderDimensions(PADDLE_SIZE);
    gPaddleR->setAcceleration({0,0});

    gBalls = new Entity[MAX_BALLS];
    for (int i = 0; i < MAX_BALLS; ++i) {
        gBalls[i].setEntityType(BLOCK);
        gBalls[i].setScale(BALL_SIZE);
        gBalls[i].setColliderDimensions(BALL_SIZE);
        gBalls[i].setPosition(ORIGIN);
        gBalls[i].setTexture("assets/ball.png");
        gBalls[i].setSpeed(200);
        gBalls[i].setAcceleration({0,0});
        gBalls[i].setMovement({1.0f, 1.0f});
        gBalls[i].deactivate();
    }
    gBalls->activate();

    SetTargetFPS(FPS);
}

void processInput() 
{
    if (IsKeyPressed(KEY_ONE)) activateBalls(1);
    if (IsKeyPressed(KEY_TWO)) activateBalls(2);
    if (IsKeyPressed(KEY_THREE)) activateBalls(3);
    if (IsKeyPressed(KEY_T)) RightAI = !RightAI;

    float halfH = gPaddleR->getColliderDimensions().y * 0.5f;


    gPaddleL->resetMovement();
    float gPaddleLY = gPaddleL->getPosition().y;
    if (IsKeyDown(KEY_W) && gPaddleLY > halfH) gPaddleL->moveUp();
    else if (IsKeyDown(KEY_S) && gPaddleLY < SCREEN_HEIGHT - halfH) gPaddleL->moveDown();

    gPaddleR->resetMovement();

    float gPaddleRY = gPaddleR->getPosition().y;
    if (!RightAI) {
        if (IsKeyDown(KEY_UP) && gPaddleRY > halfH) gPaddleR->moveUp();
        else if (IsKeyDown(KEY_DOWN) && gPaddleRY < SCREEN_HEIGHT - halfH) gPaddleR->moveDown();
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

    while (deltaTime >= FIXED_TIMESTEP)
    {
        gPaddleL->update(FIXED_TIMESTEP, nullptr, nullptr, 0, 0);
        if (RightAI) {
            static float gDirection = 1.0f;
            gPaddleR->resetMovement();
            gPaddleR->setMovement({0, gDirection});
            gPaddleR->update(FIXED_TIMESTEP, nullptr, nullptr, 0, 0);

            float halfH = gPaddleR->getColliderDimensions().y * 0.5f;
            float y = gPaddleR->getPosition().y;
            if (y  <= halfH) gDirection = 1.0f;
            else if (y >= SCREEN_HEIGHT - halfH) gDirection = -1.0f;
        } else {
            gPaddleR->update(FIXED_TIMESTEP, nullptr, nullptr, 0, 0);
        }

        for (int i=0;i<MAX_BALLS;++i) {
            if (!gBalls[i].isActive()) continue;
            gBalls[i].update(FIXED_TIMESTEP, gPaddleL, gPaddleR, 1, 1);

            Vector2 pos = gBalls[i].getPosition();
            Vector2 mv = gBalls[i].getMovement();
            float halfBall = gBalls[i].getColliderDimensions().x * 0.5f;

            if (pos.y - halfBall <= 0) { pos.y = halfBall; mv.y *= -1.0f; }
            else if (pos.y + halfBall >= SCREEN_HEIGHT) { pos.y = SCREEN_HEIGHT - halfBall; mv.y *= -1.0f; }

            if (gBalls[i].isCollidingLeft()) {
                float offset = (pos.y - gPaddleL->getPosition().y) / (gPaddleL->getColliderDimensions().y * 0.5f);
                mv.x *= -1.0f;
                mv.y = offset;
                pos.x = gPaddleL->getPosition().x + gPaddleL->getColliderDimensions().x * 0.5f + halfBall;
            }
            else if (gBalls[i].isCollidingRight()) {
                float offset = (pos.y - gPaddleR->getPosition().y) / (gPaddleR->getColliderDimensions().y * 0.5f);
                mv.x *= -1.0f;
                mv.y = offset;
                pos.x = gPaddleR->getPosition().x - gPaddleR->getColliderDimensions().x * 0.5f - halfBall;
            }

            gBalls[i].setPosition(pos);
            gBalls[i].setMovement(mv);

            if (pos.x + halfBall < 0.0f || pos.x - halfBall > SCREEN_WIDTH) {gAppStatus = TERMINATED;}

        }

        deltaTime -= FIXED_TIMESTEP;
    }

}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

    gPaddleL->render();
    gPaddleR->render();

    for (int i = 0; i < BALLS_ACTIVE;  i++) gBalls[i].render();

    EndDrawing();
}

void shutdown() 
{
    delete gPaddleL;
    delete gPaddleR;
    delete[] gBalls;
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