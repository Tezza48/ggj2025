#include "Actor.h"
#include "World.h"
#include <algorithm>
#include <cassert>
#include <raymath.h>

Actor::Actor() {
    parent = nullptr;
}

void Actor::Tick(World* world) {
    for (auto* child : children) {
        child->Tick(world);
    }
}

void Actor::Draw(World* world)
{
    for (auto* child : children) {
        child->Draw(world);
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
