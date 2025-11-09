#include "End.h"

End::End()                                      : Scene { {0.0f}, nullptr   } {}
End::End(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

End::~End() { shutdown(); }

void End::initialise()
{
   mGameState.nextSceneID = 0;
   mGameState.camera = { 0 };
   mGameState.camera.target = {0.0f, 0.0f};
   mGameState.camera.offset = {0.0f, 0.0f};
   mGameState.camera.rotation = 0.0f;
   mGameState.camera.zoom = 1.0f;
   mTimer   = 0.0f;
   mShowWin = mGameState.won;
}

void End::update(float deltaTime)
{
   mTimer += deltaTime;
   if (IsKeyPressed(KEY_ENTER)||mTimer > 1.5f) {
      mGameState.lives = 3;
      mGameState.nextSceneID = 5;
      return;
   }
}

void End::render()
{
   ClearBackground(ColorFromHex(mBGColourHexCode));

   const char* text = mShowWin ? "YOU WIN" : "YOU LOSE";

   DrawText(text,  mOrigin.x - 200.0f,  mOrigin.y-60.0f, 90, WHITE);
}

void End::shutdown(){};