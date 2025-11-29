#include "LevelC.h"
#include <cmath>   // for std::fabs

LevelC::LevelC()                                      : Scene { {0.0f}, nullptr   } {}
LevelC::LevelC(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}
LevelC::~LevelC() { shutdown(); }

void LevelC::initialise()
{
    mGameState.nextSceneID = 0;

    // ----- Audio -----
    mGameState.bgm = LoadMusicStream("assets/game/BGM.mp3");
    SetMusicVolume(mGameState.bgm, 0.33f);
    PlayMusicStream(mGameState.bgm);

    mGameState.hitSound     = LoadSound("assets/game/Hit.mp3");
    mGameState.proceedSound = LoadSound("assets/game/Proceed.mp3");
    mGameState.gameOver     = LoadSound("assets/game/GameOver.mp3");

    mCollidables.clear();
    mGems.clear();
    mMissles.clear();
    mCollidables.reserve(4000);
    mMissles.reserve(MAX_MISSLES);
    mGems.reserve(NUM_GEMS);

    mGameState.map = new Map(
        LEVELC_COLUMNS, LEVELC_ROWS,
        (unsigned int*)mLevelData,
        "assets/game/TX Tileset Grass.png",
        TILE_DIMENSION,
        11, 6,
        mOrigin
    );

    auto* map      = mGameState.map;
    float tileSize = map->getTileSize();
    float leftEdge = map->getLeftBoundary();
    float topEdge  = map->getTopBoundary();

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

    //Trees/walls

    // Outer border
    for (int c = 2; c < LEVELC_COLUMNS - 2; ++c) {
        placeTree(2, c);
        placeTree(LEVELC_ROWS - 3, c);
    }
    for (int r = 3; r < LEVELC_ROWS - 3; ++r) {
        placeTree(r, 2);
        placeTree(r, LEVELC_COLUMNS - 3);
    }

    // Multiple inner horizontal corridors
    for (int c = 6; c < LEVELC_COLUMNS - 6; ++c) {
        if ((c >= 10 && c <= 11) ||
            (c >= 20 && c <= 21) ||
            (c >= 30 && c <= 31) ||
            (c >= 40 && c <= 41)) {
            continue;
        }
        placeTree(8,  c);
        placeTree(16, c);
        placeTree(24, c);
        placeTree(32, c);
        placeTree(40, c);
    }


    // Multiple inner vertical corridors
    for (int r = 10; r < LEVELC_ROWS - 10; ++r) {
        if ((r >= 10 && r <= 20) ||
            (r >= 20 && r <= 30) ||
            (r >= 30 && r <= 40) ||
            (r >= 40 && r <= 50)) {
            continue;
        }
        placeTree(r, 8);
        placeTree(r, 16);
        placeTree(r, 24);
        placeTree(r, 32);
        placeTree(r, 40);
    }

    // Some dead-ends / blocks
    placeTree(22, 23);
    placeTree(23, 23);
    placeTree(22, 27);
    placeTree(23, 27);

    placeTree(27, 23);
    placeTree(28, 23);
    placeTree(27, 27);
    placeTree(28, 27);


    placeTree(12, 5);
    placeTree(13, 5);
    placeTree(18, 44);
    placeTree(19, 44);

    placeTree(35, 10);
    placeTree(36, 10);
    placeTree(35, 39);
    placeTree(36, 39);

    placeTree(44, 20);
    placeTree(44, 30);

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

        mGems.back().setColliderDimensions({ 25.0f, 25.0f });
    };

    // 10 gems scattered deeper in the maze
    placeGem(5, 5);
    placeGem(7, 20);
    placeGem(10, 40);
    placeGem(15, 25);
    placeGem(20, 8);
    placeGem(23, 32);
    placeGem(30, 15);
    placeGem(34, 38);
    placeGem(40, 10);
    placeGem(44, 27);

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

    float sizeRatio = 48.0f / 64.0f;

    mGameState.pink = new Entity(
        Vector2{ mOrigin.x - 350.0f, mOrigin.y - 200.0f }, // position
        Vector2{ 100.0f * sizeRatio, 100.0f },             // scale
        "assets/game/Idle.png",                            // texture
        ATLAS,                                             // atlas
        Vector2{ 1, 11 },                                  // atlas dims
        pinkIdleAtlas,                                     // idle atlas
        PLAYER                                             // entity type
    );

    mGameState.pink->setJumpingPower(550.0f);
    mGameState.pink->setColliderDimensions({ 50.0f, 100.0f });
    mGameState.pink->setAcceleration({0.0f, 0.0f});

    //enemy
    std::map<Direction, std::vector<int>> shroomAnimationAtlas = {
        {LEFT,  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}},
        {RIGHT, {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}},
        {UP,    {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}},
        {DOWN,  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}},
    };

    auto spawnFollower = [&](float offsetX, float offsetY)
    {
        mCollidables.emplace_back(
            Vector2{ mOrigin.x + offsetX, mOrigin.y + offsetY }, // position
            Vector2{ 50.0f, 60.0f },                             // scale
            "assets/game/Mushroom.png",                          // texture file
            ATLAS,                                               // texture type
            Vector2{ 1.0f, 16.0f },                              // atlas dimensions
            shroomAnimationAtlas,                                // animation atlas
            NPC                                                  // entity type
        );

        Entity& shroom = mCollidables.back();
        shroom.setAIType(FOLLOWER);
        shroom.setAIState(IDLE);
        shroom.setSpeed(Entity::DEFAULT_SPEED * 0.40f);

        shroom.setColliderDimensions({
            shroom.getScale().x / 2.0f,
            shroom.getScale().y
        });

        shroom.setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
        shroom.setDirection(LEFT);
    };

    // Spawn a few enemies around the map
    spawnFollower( +200.0f, +100.0f);
    spawnFollower( -150.0f, -100.0f);
    spawnFollower( +100.0f, +180.0f);
    spawnFollower( -220.0f, +120.0f);
    spawnFollower( +300.0f, +180.0f);

    //camera
    mGameState.camera = { 0 };
    mGameState.camera.target   = mGameState.pink->getPosition();
    mGameState.camera.offset   = mOrigin;
    mGameState.camera.rotation = 0.0f;
    mGameState.camera.zoom     = 1.2f;
}

void LevelC::update(float deltaTime)
{
    Vector2 mv = mGameState.pink->getMovement();
    bool moving = (std::fabs(mv.x) > 0.01f || std::fabs(mv.y) > 0.01f);

    if (moving && !mIsRunning) {
        mIsRunning = true;
        mGameState.pink->setTexture("assets/game/Run.png");
        mGameState.pink->setSpriteSheetDimensions({1, 12});
        mGameState.pink->setAnimationAtlas(pinkRunAtlas);
    }
    else if (!moving && mIsRunning) {
        mIsRunning = false;
        mGameState.pink->setTexture("assets/game/Idle.png");
        mGameState.pink->setSpriteSheetDimensions({1, 11});
        mGameState.pink->setAnimationAtlas(pinkIdleAtlas);
    }

    UpdateMusicStream(mGameState.bgm);

    //Projectile firing
    if (IsKeyPressed(KEY_SPACE))
    {
        // Direction based on where Pink is facing
        Direction dir = mGameState.pink->getDirection();
        Vector2 moveDir { 0.0f, 0.0f };

        switch (dir)
        {
        case LEFT:  moveDir = { -1.0f,  0.0f }; break;
        case RIGHT: moveDir = {  1.0f,  0.0f }; break;
        case UP:    moveDir = {  0.0f, -1.0f }; break;
        case DOWN:  moveDir = {  0.0f,  1.0f }; break;
        }

        // Limited number of active missles
        if (mMissles.size() < MAX_MISSLES)
        {
            Vector2 spawnPos = mGameState.pink->getPosition();

            mMissles.emplace_back(
                spawnPos,
                Vector2{ 20.0f, 20.0f },
                "assets/game/Missle.png",
                MISSLE
            );

            Entity& missle = mMissles.back();
            missle.setColliderDimensions({ 20.0f, 20.0f });
            missle.setMovement(moveDir);
            missle.setSpeed(400);
            missle.setAcceleration({0.0f, 0.0f});
        }
    }



    Entity* collidables = mCollidables.empty() ? nullptr : mCollidables.data();
    int     count       = static_cast<int>(mCollidables.size());

    mGameState.pink->update(deltaTime,
                            nullptr,
                            nullptr,
                            collidables,
                            count);
    for (Entity& e : mMissles) {
      e.update(deltaTime,
               nullptr,
               nullptr,
               collidables,
               count);
    }

    for (Entity& e : mCollidables) {
        e.update(deltaTime,
                 mGameState.pink,
                 nullptr,
                 collidables,
                 count);
    }


    for (Entity& gem : mGems) {
        if (!gem.isActive()) continue;

        if (mGameState.pink->isColliding(&gem)) {
            gem.deactivate();
            ++mGemsCollected;
            PlaySound(mGameState.proceedSound);
        }
    }

    if (!mGameState.pink->isActive()) {
        PlaySound(mGameState.hitSound);
        mGameState.lives -= 1;

        if (mGameState.lives <= 0) {
            mGameState.won = false;
            PlaySound(mGameState.gameOver);
            mGameState.nextSceneID = 5;
        } else {
            mGameState.nextSceneID = 3;
        }
        return;
    }

    if (mGemsCollected >= NUM_GEMS) {
        PlaySound(mGameState.proceedSound);
        mGameState.nextSceneID = 4;
        return;
    }

    Vector2 camTarget = mGameState.pink->getPosition();

    Map* map = mGameState.map;
    float left   = map->getLeftBoundary();
    float right  = map->getRightBoundary();
    float top    = map->getTopBoundary();
    float bottom = map->getBottomBoundary();

    float halfW = GetScreenWidth()  * 0.5f;
    float halfH = GetScreenHeight() * 0.5f;

    float minX = left   + halfW;
    float maxX = right  - halfW;
    float minY = top    + halfH;
    float maxY = bottom - halfH;

    if (camTarget.x < minX) camTarget.x = minX;
    if (camTarget.x > maxX) camTarget.x = maxX;
    if (camTarget.y < minY) camTarget.y = minY;
    if (camTarget.y > maxY) camTarget.y = maxY;

    mGameState.camera.target = camTarget;
}

void LevelC::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));
    BeginMode2D(mGameState.camera);
    mGameState.map->render();

    mGameState.pink->render();
    //draw missles
    for (Entity& e : mMissles) {
        if (e.isActive()) e.render();
    }
    // draw gems
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

    DrawText(TextFormat("Gems: %d / %d",
                        mGemsCollected, NUM_GEMS),
             20, 50, 25, WHITE);

}

void LevelC::shutdown()
{
    delete mGameState.pink;
    mGameState.pink = nullptr;

    delete mGameState.map;
    mGameState.map = nullptr;

    mCollidables.clear();
    mGems.clear();
    mMissles.clear();

    UnloadMusicStream(mGameState.bgm);
    UnloadSound(mGameState.gameOver);
    UnloadSound(mGameState.proceedSound);
    UnloadSound(mGameState.hitSound);
}
