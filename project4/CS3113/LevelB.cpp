#include "LevelB.h"

LevelB::LevelB()                                      : Scene { {0.0f}, nullptr   } {}
LevelB::LevelB(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

LevelB::~LevelB() { shutdown(); }

void LevelB::initialise()
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

   std::map<Direction, std::vector<int>> chickenAnimationAtlas = {
      {LEFT,  {0,1,2,3,4,5,6,7,8,9,10,11,12,13}},
      {RIGHT,  {0,1,2,3,4,5,6,7,8,9,10,11,12,13}},
      {UP,  {0,1,2,3,4,5,6,7,8,9,10,11,12,13}},
      {DOWN,  {0,1,2,3,4,5,6,7,8,9,10,11,12,13}},
  };

   mGameState.chicken = new Entity(
       {mOrigin.x+200.0f, mOrigin.y}, // position
       {50.0f, 60.0f},                       // scale
       "assets/game/Chicken.png",                // texture file address
       ATLAS,                                  // single image or atlas?
       {1,14},                       // atlas dimensions
       chickenAnimationAtlas,                    // actual atlas
       NPC                                     // entity type
   );

   mGameState.chicken->setAIType(FOLLOWER);
   mGameState.chicken->setAIState(IDLE);
   mGameState.chicken->setSpeed(Entity::DEFAULT_SPEED * 0.50f);

   mGameState.chicken->setColliderDimensions({
       mGameState.chicken->getScale().x / 2.0f,
       mGameState.chicken->getScale().y
   });

   mGameState.chicken->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
   mGameState.chicken->setDirection(LEFT);

   /*
      ----------- CAMERA -----------
   */
   mGameState.camera = { 0 };                                    // zero initialize
   mGameState.camera.target = mGameState.pink->getPosition(); // camera follows player
   mGameState.camera.offset = mOrigin;                           // camera offset to center of screen
   mGameState.camera.rotation = 0.0f;                            // no rotation
   mGameState.camera.zoom = 1.0f;                                // default zoom
}

void LevelB::update(float deltaTime)
{
   UpdateMusicStream(mGameState.bgm);
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

   mGameState.pink->update(
      deltaTime,      // delta time / fixed timestep
      nullptr,        // player
      mGameState.map, // map
      mGameState.chicken,        // collidable entities
      1               // col. entity count
   );
   mGameState.chicken->update(
       deltaTime,
       mGameState.pink,
       mGameState.map,
       nullptr,
       0
   );

   // CAMERA
   Vector2 currentPlayerPosition = { mGameState.pink->getPosition().x, mOrigin.y };

   if (mGameState.pink->getPosition().y > 800.0f || !mGameState.chicken->isActive()) {
      PlaySound(mGameState.hitSound);
      mGameState.lives -= 1;
      if (mGameState.lives <= 0) {
         mGameState.won = false;
         PlaySound(mGameState.gameOver);
         mGameState.nextSceneID = 4;
      } else {
         mGameState.nextSceneID = 2;
      }
   }

   else if (mGameState.pink->getPosition().x >= mOrigin.x+400.0f) {
      PlaySound(mGameState.proceedSound);
      mGameState.nextSceneID = 3;
      return;
   }

   panCamera(&mGameState.camera, &currentPlayerPosition);
}

void LevelB::render()
{
   ClearBackground(ColorFromHex(mBGColourHexCode));

   mGameState.pink->render();
   mGameState.chicken->render();
   mGameState.map->render();
   DrawText(TextFormat("Lives: %d", mGameState.lives), 20, 20, 25, WHITE);
}

void LevelB::shutdown()
{
   delete mGameState.pink;
   delete mGameState.chicken;
   delete mGameState.map;

   UnloadMusicStream(mGameState.bgm);
   UnloadSound(mGameState.gameOver);
   UnloadSound(mGameState.proceedSound);
   UnloadSound(mGameState.hitSound);
}