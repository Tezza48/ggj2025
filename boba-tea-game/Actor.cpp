#include "Actor.h"
#include "World.h"
#include <algorithm>
#include <cassert>
#include <raymath.h>

#include <rlgl.h>

Actor::Actor() : pos(), scale(1.0f) {
	parent = nullptr;
}

void Actor::Tick(World* world) {
	for (auto* child : children) {
		child->Tick(world);
	}
}

void Actor::PreDraw3D()
{
	rlPushMatrix();
	rlScalef(scale, scale, scale);
	rlRotatef(rotation.x, 1.0f, 0.0f, 0.0f);
	rlRotatef(rotation.y, 0.0f, 1.0f, 0.0f);
	rlRotatef(rotation.z, 0.0f, 0.0f, 1.0f);
	rlTranslatef(pos.x, pos.y, pos.z);
}

void Actor::Draw3D(World* world)
{
	for (auto* child : children) {
		if (!child->visible) continue;
		child->PreDraw3D();
		child->Draw3D(world);
		child->PostDraw3D();
	}
}

void Actor::PostDraw3D() {
	rlPopMatrix();
}

void Actor::Draw2D(World* world) {
	for (auto* child : children) {
		if (!child->visible) continue;
		child->Draw2D(world);
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
