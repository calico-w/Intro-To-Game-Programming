#include "Entity.h"

#ifndef SCENE_H
#define SCENE_H

struct GameState
{
    Entity *pink;
    Entity* shroom;
    Entity* chicken;
    Entity* ghost;
    Entity* throwable;
    Map *map;

    Music bgm;
    Sound hitSound;
    Sound proceedSound;
    Sound gameOver;

    Camera2D camera;

    int lives;
    bool won;

    int nextSceneID;
};

class Scene 
{
protected:
    GameState mGameState;
    Vector2 mOrigin;
    const char *mBGColourHexCode = "#000000";
    
public:
    Scene();
    Scene(Vector2 origin, const char *bgHexCode);

    virtual void initialise() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void shutdown() = 0;

    GameState&       getState()       { return mGameState; }
    const GameState& getState() const { return mGameState; }
    Vector2     getOrigin()          const { return mOrigin;    }
    const char* getBGColourHexCode() const { return mBGColourHexCode; }
};

#endif