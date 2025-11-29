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

    mGameState.hitSound     = LoadSound("assets/game/Hit.mp3");
    mGameState.proceedSound = LoadSound("assets/game/Proceed.mp3");
    mGameState.gameOver     = LoadSound("assets/game/GameOver.mp3");

    mCollidables.clear();
    mGems.clear();
    mCollidables.reserve(2000);
    mGems.reserve(NUM_GEMS);

    /*
        ----------- MAP -----------
    */
    mGameState.map = new Map(
        LEVELB_COLUMNS, LEVELB_ROWS,        // map grid cols & rows
        (unsigned int *) mLevelData,        // grid data
        "assets/game/TX Tileset Grass.png", // texture filepath
        TILE_DIMENSION,                     // tile size
        2, 2,                               // texture cols & rows (same as LevelA)
        mOrigin                             // in-game origin
    );

    auto* map     = mGameState.map;
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

    // Outer border
    for (int c = 2; c < LEVELB_COLUMNS - 2; ++c) {
        placeTree(2, c);
        placeTree(LEVELB_ROWS - 3, c);
    }
    for (int r = 3; r < LEVELB_ROWS - 3; ++r) {
        placeTree(r, 2);
        placeTree(r, LEVELB_COLUMNS - 3);
    }

    // Inner horizontal walls
    for (int c = 5; c < LEVELB_COLUMNS - 5; ++c) {
        if (c == 10 || c == 19) continue; // gaps
        placeTree(8, c);
        placeTree(21, c);
    }

    // Inner vertical walls
    for (int r = 6; r < LEVELB_ROWS - 6; ++r) {
        if (r == 12 || r == 17) continue; // gaps
        placeTree(r, 8);
        placeTree(r, 21);
    }

    placeTree(10, 14);
    placeTree(11, 14);
    placeTree(18, 15);
    placeTree(19, 15);

    placeTree(13, 5);
    placeTree(16, 24);
    placeTree(24, 9);
    placeTree(24, 20);

    /*
        ----------- GEMS -----------
    */
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


    placeGem(4, 4);
    placeGem(7, 22);
    placeGem(11, 12);
    placeGem(15, 26);
    placeGem(19, 6);
    placeGem(22, 16);
    placeGem(25, 23);

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
        Vector2{ mOrigin.x - 300.0f, mOrigin.y - 200.0f }, // position
        Vector2{ 100.0f * sizeRatio, 100.0f },             // scale
        "assets/game/Idle.png",                            // texture file address
        ATLAS,                                             // single image or atlas?
        Vector2{ 1, 11 },                                  // atlas dimensions
        pinkIdleAtlas,                                     // actual atlas
        PLAYER                                             // entity type
    );

    mGameState.pink->setJumpingPower(550.0f);
    mGameState.pink->setColliderDimensions({
        50.0f,
        100.0f
    });
    mGameState.pink->setAcceleration({0.0f, 0.0f});

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

// Spawn a few around the map (tweak offsets as you like)
    spawnFollower( +200.0f, +100.0f);
    spawnFollower( -100.0f, -100.0f);
    spawnFollower( +100.0f, +180.0f);
    spawnFollower( -220.0f, +120.0f);


    mGameState.camera = { 0 };                             // zero initialize
    mGameState.camera.target = mGameState.pink->getPosition(); // camera follows player
    mGameState.camera.offset = mOrigin;                    // camera offset to center of screen
    mGameState.camera.rotation = 0.0f;                     // no rotation
    mGameState.camera.zoom = 1.2f;                         // default zoom
}

void LevelB::update(float deltaTime)
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


    Entity* collidables = mCollidables.empty() ? nullptr : mCollidables.data();
    int     count       = static_cast<int>(mCollidables.size());


    mGameState.pink->update(deltaTime,
                            nullptr,
                            nullptr,
                            collidables,
                            count);


    for (Entity& e : mCollidables) {
        e.update(deltaTime,
             mGameState.pink,  // player pointer
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
            mGameState.nextSceneID = 2;
        }

        return;
    }

    if (mGemsCollected >= NUM_GEMS) {
        PlaySound(mGameState.proceedSound);
        mGameState.nextSceneID = 3;   // go to LevelC
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

void LevelB::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));
    BeginMode2D(mGameState.camera);

    mGameState.map->render();

    mGameState.pink->render();

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

void LevelB::shutdown()
{
    delete mGameState.pink;
    mGameState.shroom = nullptr;
    delete mGameState.map;

    UnloadMusicStream(mGameState.bgm);
    UnloadSound(mGameState.gameOver);
    UnloadSound(mGameState.proceedSound);
    UnloadSound(mGameState.hitSound);
}
