#include "Win.h"

Win::Win()                                      : Scene { {0.0f}, nullptr   } {}
Win::Win(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

Win::~Win() { shutdown(); }

void Win::initialise()
{
   mGameState.nextSceneID = 0;
   mGameState.camera = { 0 };
   mGameState.camera.target = {0.0f, 0.0f};
   mGameState.camera.offset = {0.0f, 0.0f};
   mGameState.camera.rotation = 0.0f;
   mGameState.camera.zoom = 1.0f;
   mTimer   = 0.0f;
}

void Win::update(float deltaTime)
{
   mTimer += deltaTime;
   if (IsKeyPressed(KEY_ENTER)||mTimer > 1.5f) {
      mGameState.lives = 3;
      mGameState.nextSceneID = 6;
      return;
   }
}

void Win::render()
{
   ClearBackground(ColorFromHex(mBGColourHexCode));

   const char* text ="YOU WIN!";

   DrawText(text,  mOrigin.x - 200.0f,  mOrigin.y-60.0f, 90, WHITE);
}

void Win::shutdown(){};