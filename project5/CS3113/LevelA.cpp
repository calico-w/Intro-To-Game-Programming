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
   mCollidables.clear();
   mGems.clear();
   mCollidables.reserve(500);
   mGems.reserve(NUM_GEMS);

   /*
      ----------- MAP -----------
   */
   mGameState.map = new Map(
      LEVELA_COLUMNS, LEVELA_ROWS,   // map grid cols & rows
      (unsigned int *) mLevelData, // grid data
      "assets/game/TX Tileset Grass.png",   // texture filepath
      TILE_DIMENSION,              // tile size
      2, 2,                        // texture cols & rows
      mOrigin                      // in-game origin
   );
    auto* map = mGameState.map;
    float tileSize   = map->getTileSize();
    float leftEdge   = map->getLeftBoundary();
    float topEdge    = map->getTopBoundary();

    auto placeTree = [&](int row, int col)
    {
        Vector2 pos {
            leftEdge + col * tileSize + tileSize / 2.0f,
            topEdge  + row * tileSize + tileSize / 2.0f
        };

        Vector2 treeSize = { 62.0f, 67.0f };

        mCollidables.emplace_back(
            pos,
            treeSize,
            "assets/game/Tree.png",
            BLOCK
        );

        mCollidables.back().setColliderDimensions({ 32.0f, 24.0f });
    };

    for (int c = 2; c < LEVELA_COLUMNS - 2; ++c) {
        placeTree(2, c);
        placeTree(LEVELA_ROWS - 3, c);
    }
    for (int r = 3; r < LEVELA_ROWS - 3; ++r) {
        placeTree(r, 2);
        placeTree(r, LEVELA_COLUMNS - 3);
    }

    placeTree(12, 11);
    placeTree(13, 9);
    placeTree(11, 18);
    placeTree(12, 20);
    placeTree(18, 11);
    placeTree(19, 9);
    placeTree(17, 18);
    placeTree(18, 20);

    placeTree(25, 5);
    placeTree(26, 7);
    placeTree(24, 23);
    placeTree(25, 21);

    mGemsCollected = 0;

    auto placeGem = [&](int row, int col)
    {
        Vector2 pos {
            leftEdge + col * tileSize + tileSize / 2.0f,
            topEdge  + row * tileSize + tileSize / 2.0f
        };

        Vector2 gemSize = { 30.0f, 30.0f };

        mGems.emplace_back(
            pos,
            gemSize,
            "assets/game/Gem.png",
            NONE
        );

        mGems.back().setColliderDimensions({ 26.0f, 26.0f });
    };

    placeGem(5, 5);
    placeGem(8, 22);
    placeGem(15, 10);
    placeGem(20, 25);
    placeGem(24, 14);


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

   mGameState.pink->setColliderDimensions({
      50.0f,
      100.0f
   });
   mGameState.pink->setAcceleration({0.0f, 0.0f});

   //AI enemy
   std::map<Direction, std::vector<int>> shroomAnimationAtlas = {
      {LEFT,  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}},
      {RIGHT,  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}},
      {UP,  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}},
      {DOWN,  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}},
  };


    mCollidables.emplace_back(
        Vector2{ mOrigin.x + 200.0f, mOrigin.y }, // position
        Vector2{ 50.0f, 60.0f },                  // scale
        "assets/game/Mushroom.png",               // texture file address
        ATLAS,                                    // texture type
        Vector2{ 1.0f, 16.0f },                   // atlas dimensions
        shroomAnimationAtlas,                     // animation atlas
        NPC                                       // entity type
    );

   mGameState.shroom = &mCollidables.back();
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
   mGameState.camera.zoom = 1.2f;                                // default zoom
}

void LevelA::update(float deltaTime)
{
   Vector2 mv = mGameState.pink->getMovement();
   bool moving = (std::fabs(mv.x) > 0.01f || std::fabs(mv.y) > 0.01f);

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

   Entity* collidables = mCollidables.data();
   int count = static_cast<int>(mCollidables.size());

   mGameState.pink->update(deltaTime, nullptr, nullptr, collidables, count);

   for (Entity& e : mCollidables) {
       e.update(deltaTime, mGameState.pink, nullptr, collidables, count);
   }
   if (mGameState.shroom->getPosition().x < mOrigin.x+100.0f) mGameState.shroom->setDirection(RIGHT);
   else if (mGameState.shroom->getPosition().x > mOrigin.x+400.0f) mGameState.shroom->setDirection(LEFT);

   Vector2 currentPlayerPosition = mGameState.pink->getPosition();

   for (Entity& gem : mGems) {
        if (!gem.isActive()) continue;

        if (mGameState.pink->isColliding(&gem)) {
            gem.deactivate();
            ++mGemsCollected;
            PlaySound(mGameState.proceedSound);
        }
    }

    if (mGemsCollected >= NUM_GEMS) {
        PlaySound(mGameState.proceedSound);
        mGameState.nextSceneID = 2;
        return;
    }


   if (!mGameState.pink->isActive()) {
      PlaySound(mGameState.hitSound);
      mGameState.lives -= 1;
      if (mGameState.lives <= 0) {
         mGameState.won = false;
         PlaySound(mGameState.gameOver);
         mGameState.nextSceneID = 5;
      } else {
         mGameState.nextSceneID = 1;
      }
      return;
   }
    /*
   else if (mGameState.pink->getPosition().x >= mOrigin.x+400.0f) {
     PlaySound(mGameState.proceedSound);
     mGameState.nextSceneID = 2;
     return;
   }

     */

    Vector2 camTarget = mGameState.pink->getPosition();

    Map* map = mGameState.map;


    float left = map->getLeftBoundary();
    float right = map->getRightBoundary();
    float top = map->getTopBoundary();
    float bottom = map->getBottomBoundary();

    float halfW = GetScreenWidth()  * 0.5f;
    float halfH = GetScreenHeight() * 0.5f;

    float minX = left + halfW;
    float maxX = right - halfW;
    float minY = top + halfH;
    float maxY = bottom - halfH;

    if (camTarget.x < minX) camTarget.x = minX;
    if (camTarget.x > maxX) camTarget.x = maxX;
    if (camTarget.y < minY) camTarget.y = minY;
    if (camTarget.y > maxY) camTarget.y = maxY;

    mGameState.camera.target = camTarget;

}

void LevelA::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));
    BeginMode2D(mGameState.camera);

    mGameState.map->render();


    mGameState.pink->render();

    for (Entity& gem : mGems) {
        if (gem.isActive()) {
            gem.render();
        }
    }

    for (Entity& e : mCollidables) {
        e.render();
    }
    EndMode2D();
    DrawText(TextFormat("Lives: %d", mGameState.lives),
             20, 20, 25, WHITE);

    DrawText(TextFormat("Gems: %d / %d", mGemsCollected, NUM_GEMS),
             20, 50, 25, WHITE);
}

void LevelA::shutdown()
{
   delete mGameState.pink;
   mGameState.shroom = nullptr;
   delete mGameState.map;

   UnloadMusicStream(mGameState.bgm);
   UnloadSound(mGameState.gameOver);
   UnloadSound(mGameState.proceedSound);
   UnloadSound(mGameState.hitSound);
}