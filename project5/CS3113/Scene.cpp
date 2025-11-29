#include "Scene.h"

Scene::Scene() : mOrigin{{}} {
    mGameState.pink      = nullptr;
    mGameState.shroom    = nullptr;
    mGameState.chicken   = nullptr;
    mGameState.ghost     = nullptr;
    mGameState.throwable = nullptr;
    mGameState.map       = nullptr;

    mGameState.lives       = 0;
    mGameState.won         = false;
    mGameState.nextSceneID = 0;
}

Scene::Scene(Vector2 origin, const char *bgHexCode)
    : mOrigin{origin}, mBGColourHexCode{bgHexCode}
{
    mGameState.pink      = nullptr;
    mGameState.shroom    = nullptr;
    mGameState.chicken   = nullptr;
    mGameState.ghost     = nullptr;
    mGameState.throwable = nullptr;
    mGameState.map       = nullptr;

    mGameState.lives       = 0;
    mGameState.won         = false;
    mGameState.nextSceneID = 0;
}
