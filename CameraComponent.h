#pragma once
#include "Component.h"

class GameObject;

class CameraComponent :
    public Component
{
public:
    CameraComponent(GameObject* owner);
    Component::Type GetComponentType() override { return Component::Type::Camera; };

    void Render() override;

};

