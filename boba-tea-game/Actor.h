#pragma once

#include <vector>
#include <raylib.h>

struct World;

class Actor {
public:
    Actor* parent;
    std::vector<Actor*> children;

    Vector3 pos;

    virtual void Tick(World* world);
    virtual void Draw(World* world);

    Actor();
    ~Actor();

    void AddChild(Actor* child);
    void RemoveChild(Actor* child);
};