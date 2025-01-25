#include <raylib.h>
#include "Actor.h"
#include "World.h"

class Player : public Actor {
public:
    Vector3 pos;

    Player() : Actor(), pos() {
    }

    void Tick(World* world) override {
        Actor::Tick(world);

        float moveSpeed = 10;
        if (IsKeyDown(KEY_W)) {
            pos.z -= GetFrameTime() * moveSpeed;
        }

        if (IsKeyDown(KEY_S)) {
            pos.z += GetFrameTime() * moveSpeed;
        }

        if (IsKeyDown(KEY_A)) {
            pos.x -= GetFrameTime() * moveSpeed;
        }

        if (IsKeyDown(KEY_D)) {
            pos.x += GetFrameTime() * moveSpeed;
        }

        DrawSphere(pos, 1.0f, BLACK);
    }
};

class LevelGrid : public Actor {
public:
    Vector3 pos;
    int width;

    LevelGrid(int width) : Actor(), width(width), pos() {
    }

    void Tick(World* world) override {
        DrawGrid(10, 1.0);
    }
};

int main(void)
{
    InitWindow(1280, 720, "GGJ 2025 Boba Tea Roller");

    Actor root = {};
    World world = {
        &root
    };

    Camera cam;
    cam.projection = CAMERA_PERSPECTIVE;
    cam.position = { 0.0f, 10.0f, 1.0f };
    cam.fovy = 60.0f;
    cam.up = { 0.0f, 1.0f, 0.0f };
    cam.target = {};

    Player player = Player();
    root.AddChild(&player);

    LevelGrid grid = LevelGrid(5);
    root.AddChild(&grid);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        BeginMode3D(cam);

        world.root->Tick(&world);
        EndMode3D();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}