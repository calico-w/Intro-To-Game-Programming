#include "Menu.h"

Menu::Menu()                                      : Scene { {0.0f}, nullptr   } {}
Menu::Menu(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

Menu::~Menu() { shutdown(); }

void Menu::initialise()
{
   mGameState.nextSceneID = 0;
   mGameState.camera = { 0 };
   mGameState.camera.target = {0.0f, 0.0f};
   mGameState.camera.offset = {0.0f, 0.0f};
   mGameState.camera.rotation = 0.0f;
   mGameState.camera.zoom = 1.0f;
}

void Menu::update(float deltaTime)
{
   if (IsKeyPressed(KEY_ENTER)) {
      mGameState.lives = 3;
      mGameState.won = false;
      mGameState.nextSceneID = 1;
   }
}

void Menu::render()
{
   ClearBackground(ColorFromHex(mBGColourHexCode));

   //Title
   int titleX = mOrigin.x - 300.0f;
   int titleY = mOrigin.y - 100.0f;
   DrawText("Rise of the AI", titleX, titleY, 100, WHITE);

   //Prompt
   int promptX = mOrigin.x - 250.0f;
   int promptY = mOrigin.y + 20.0f;
   DrawText("Press Enter to Start", promptX, promptY, 50, WHITE);
}

void Menu::shutdown(){};