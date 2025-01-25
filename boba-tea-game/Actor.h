#pragma once

#include <vector>
#include <raylib.h>

struct World;

class Actor {
public:
    Actor* parent;
    std::vector<Actor*> children;

    Vector3 pos;
    bool visible = true;

    //Vector3 globalPos;

    virtual void Tick(World* world);
    void PreDraw3D();
    virtual void Draw3D(World* world);
    void PostDraw3D();

    virtual void Draw2D(World* world);

    Actor();
    ~Actor();

    void AddChild(Actor* child);
    void RemoveChild(Actor* child);
};