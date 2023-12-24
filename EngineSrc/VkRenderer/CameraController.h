#ifndef CAMERA_CONTROLLER_HDR
#define CAMERA_CONTROLLER_HDR

#include "Core/TimeStep.h"
#include "Entity/GameObject.h"

namespace Gust 
{

class CameraController 
{
public:
    void moveInPlaneXZ(TimeStep timestep, GameObject &gameObjects);
private:
    float _moveSpeed{ 5.f };
    float _lookSpeed{ 1.f };
};

}

#endif // !CAMERA_CONTROLLER_HDR
