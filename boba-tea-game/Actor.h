#pragma once

#include <vector>

struct World;

class Actor {
public:
    Actor* parent;
    std::vector<Actor*> children;
    virtual void Tick(World* world);

    Actor();
    ~Actor();

    void AddChild(Actor* child);
    void RemoveChild(Actor* child);
};