#include "LevelC.h"

#ifndef END_H
#define END_H

class End : public Scene {
public:
    End();
    End(Vector2 origin, const char *bgHexCode);
    ~End();
    
    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
private:
    float mTimer = 0.0f;
    bool mShowWin = false;
};
#endif