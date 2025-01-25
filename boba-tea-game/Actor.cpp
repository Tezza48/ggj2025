#include "Actor.h"
#include "World.h"
#include <algorithm>
#include <cassert>

Actor::Actor() {
    parent = nullptr;
}

void Actor::Tick(World* world) {
    for (auto* child : children) {
        child->Tick(world);
    }
}

Actor::~Actor() {
}

void Actor::AddChild(Actor* child)
{
    children.push_back(child);
    child->parent = this;
}

void Actor::RemoveChild(Actor* child)
{
    auto it = std::find(children.begin(), children.end(), child);
    assert(it != children.end());
    children.erase(it);
}
