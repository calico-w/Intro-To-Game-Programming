#include "LevelA.h"

LevelA::LevelA()                                      : Scene { {0.0f}, nullptr   } {}
LevelA::LevelA(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

LevelA::~LevelA() { shutdown(); }

void LevelA::initialise()
{
   mGameState.nextSceneID = 0;

   mGameState.bgm = LoadMusicStream("assets/game/BGM.mp3");
   SetMusicVolume(mGameState.bgm, 0.33f);
   PlayMusicStream(mGameState.bgm);

   mGameState.hitSound = LoadSound("assets/game/Hit.mp3");
   mGameState.proceedSound = LoadSound("assets/game/Proceed.mp3");
   mGameState.gameOver = LoadSound("assets/game/GameOver.mp3");

   /*
      ----------- MAP -----------
   */
   mGameState.map = new Map(
      LEVEL_WIDTH, LEVEL_HEIGHT,   // map grid cols & rows
      (unsigned int *) mLevelData, // grid data
      "assets/game/tile_0041.png",   // texture filepath
      TILE_DIMENSION,              // tile size
      1, 1,                        // texture cols & rows
      mOrigin                      // in-game origin
   );

   /*
      ----------- PROTAGONIST -----------
   */
   pinkIdleAtlas = {
      {LEFT,  {0,1,2,3,4,5,6,7,8,9,10}},
      {RIGHT, {0,1,2,3,4,5,6,7,8,9,10}},
      {UP,    {0,1,2,3,4,5,6,7,8,9,10}},
      {DOWN,  {0,1,2,3,4,5,6,7,8,9,10}},
   };
   pinkRunAtlas = {
      {LEFT,  {0,1,2,3,4,5,6,7,8,9,10,11}},
      {RIGHT, {0,1,2,3,4,5,6,7,8,9,10,11}},
      {UP,    {0,1,2,3,4,5,6,7,8,9,10,11}},
      {DOWN,  {0,1,2,3,4,5,6,7,8,9,10,11}},
   };

   float sizeRatio  = 48.0f / 64.0f;

   mGameState.pink = new Entity(
      {mOrigin.x - 300.0f, mOrigin.y - 200.0f}, // position
      {100.0f * sizeRatio, 100.0f},             // scale
      "assets/game/Idle.png",                   // texture file address
      ATLAS,                                    // single image or atlas?
      { 1, 11 },                                 // atlas dimensions
      pinkIdleAtlas,                    // actual atlas
      PLAYER                                    // entity type
   );

   mGameState.pink->setJumpingPower(550.0f);
   mGameState.pink->setColliderDimensions({
      50.0f,
      100.0f
   });
   mGameState.pink->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

   //AI enemy
   std::map<Direction, std::vector<int>> shroomAnimationAtlas = {
      {LEFT,  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}},
      {RIGHT,  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}},
      {UP,  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}},
      {DOWN,  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}},
  };

   mGameState.shroom = new Entity(
       {mOrigin.x+200.0f, mOrigin.y}, // position
       {50.0f, 60.0f},                       // scale
       "assets/game/Mushroom.png",                // texture file address
       ATLAS,                                  // single image or atlas?
       {1,16},                       // atlas dimensions
       shroomAnimationAtlas,                    // actual atlas
       NPC                                     // entity type
   );

   mGameState.shroom->setAIType(WANDERER);
   mGameState.shroom->setAIState(IDLE);
   mGameState.shroom->setSpeed(Entity::DEFAULT_SPEED * 0.50f);

   mGameState.shroom->setColliderDimensions({
       mGameState.shroom->getScale().x / 2.0f,
       mGameState.shroom->getScale().y
   });

   mGameState.shroom->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
   mGameState.shroom->setDirection(LEFT);

   /*
      ----------- CAMERA -----------
   */
   mGameState.camera = { 0 };                                    // zero initialize
   mGameState.camera.target = mGameState.pink->getPosition(); // camera follows player
   mGameState.camera.offset = mOrigin;                           // camera offset to center of screen
   mGameState.camera.rotation = 0.0f;                            // no rotation
   mGameState.camera.zoom = 1.0f;                                // default zoom
}

void LevelA::update(float deltaTime)
{
   bool moving = (std::fabs(mGameState.pink->getMovement().x) > 0.01f);
   if (moving && !mIsRunning) {
      mIsRunning = true;
      mGameState.pink->setTexture("assets/game/Run.png");
      mGameState.pink->setSpriteSheetDimensions({1, 12});
      mGameState.pink->setAnimationAtlas(pinkRunAtlas);
   } else if (!moving && mIsRunning) {
      mIsRunning = false;
      mGameState.pink->setTexture("assets/game/Idle.png");
      mGameState.pink->setSpriteSheetDimensions({1, 11});
      mGameState.pink->setAnimationAtlas(pinkIdleAtlas);
   }
   UpdateMusicStream(mGameState.bgm);

   mGameState.pink->update(
      deltaTime,      // delta time / fixed timestep
      nullptr,        // player
      mGameState.map, // map
      mGameState.shroom,        // collidable entities
      1               // col. entity count
   );
   mGameState.shroom->update(
       deltaTime,
       nullptr,
       mGameState.map,
       nullptr,
       0
   );
   if (mGameState.shroom->getPosition().x < mOrigin.x+100.0f) mGameState.shroom->setDirection(RIGHT);
   else if (mGameState.shroom->getPosition().x > mOrigin.x+400.0f) mGameState.shroom->setDirection(LEFT);

   Vector2 currentPlayerPosition = { mGameState.pink->getPosition().x, mOrigin.y };

   if (mGameState.pink->getPosition().y > 800.0f || !mGameState.shroom->isActive()) {
      PlaySound(mGameState.hitSound);
      mGameState.lives -= 1;
      if (mGameState.lives <= 0) {
         mGameState.won = false;
         PlaySound(mGameState.gameOver);
         mGameState.nextSceneID = 4;
      } else {
         mGameState.nextSceneID = 1;
      }
   }

   else if (mGameState.pink->getPosition().x >= mOrigin.x+400.0f) {
     PlaySound(mGameState.proceedSound);
     mGameState.nextSceneID = 2;
     return;
   }


   panCamera(&mGameState.camera, &currentPlayerPosition);
}

void LevelA::render()
{
   ClearBackground(ColorFromHex(mBGColourHexCode));

   mGameState.pink->render();
   mGameState.shroom->render();
   mGameState.map->render();
   mGameState.pink->displayCollider();
   mGameState.shroom->displayCollider();
   DrawText(TextFormat("Lives: %d", mGameState.lives), 20, 20, 25, WHITE);
}

void LevelA::shutdown()
{
   delete mGameState.pink;
   delete mGameState.shroom;
   delete mGameState.map;

   UnloadMusicStream(mGameState.bgm);
   UnloadSound(mGameState.gameOver);
   UnloadSound(mGameState.proceedSound);
   UnloadSound(mGameState.hitSound);
}