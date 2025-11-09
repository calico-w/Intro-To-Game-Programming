#include "Scene.h"

constexpr int LEVEL_WIDTH = 14,
              LEVEL_HEIGHT = 8;

class Menu : public Scene {
public:
    Menu();
    Menu(Vector2 origin, const char *bgHexCode);
    ~Menu();
    
    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};
