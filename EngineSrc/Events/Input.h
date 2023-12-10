#ifndef INPUT_HDR
#define INPUT_HDR

#include <glm/glm.hpp>

namespace Gust 
{

class Input
{
public:
    static bool isKeyPressed(int keyCode);
    static bool isMouseButtonPressed(int button);
    static glm::vec2 getMousePosition();
    static float getMouseX();
    static float getMouseY();
};

}//Gust

#endif // !INPUT_HDR
