#include <raylib.h>
#include "Actor.h"
#include "World.h"
#include <raymath.h>
#include <vector>
#include <cmath>
#include <string>
#include <iostream>
//#define RLGL_IMPLEMENTATION
//#include <rlgl.h>

class Player : public Actor {
public:
	int size = 0;

	float sizeScale = 0.02f;

	Player() : Actor() {
	}

	void Tick(World* world) override {
		Actor::Tick(world);

	}

	void Draw(World* world) override {
		Actor::Draw(world);
		float radius = 0.2f + size * sizeScale;
		Vector3 drawPos = { pos.x, pos.y + radius, pos.z };
		DrawSphere(drawPos, radius, BLACK);
	}
};

enum class LevelCell {
	FLOOR,
	GROWSTUFF,
};

class Level : public Actor {
public:
	Player player;
	int playerCellx, playerCellY;

	int levelWidth = 5;
	std::vector<LevelCell> level;

	Level() : Actor() {
		AddChild(&player);

		level = std::vector({
			LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF,
			LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF,
			LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF,
			LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF,
			LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF,
			});
	}

	void Tick(World* world) override {
		Actor::Tick(world);

		Vector3 playerMoveDir = { 0 };
		if (IsKeyReleased(KEY_W)) {
			playerMoveDir.z--;
		}

		if (IsKeyReleased(KEY_S)) {
			playerMoveDir.z++;
		}

		if (IsKeyReleased(KEY_A)) {
			playerMoveDir.x--;
		}

		if (IsKeyReleased(KEY_D)) {
			playerMoveDir.x++;
		}

		if (Vector3LengthSqr(playerMoveDir) > 0.0f) {
			int oldX = (int)floor(player.pos.x);
			int oldZ = (int)floor(player.pos.z);
			player.pos = Vector3Clamp(
				Vector3Add(player.pos, playerMoveDir),
				Vector3{ 0 },
				Vector3{ (float)levelWidth - 1, 0.0f, (float)levelWidth - 1 }
			);
			int x = (int)floor(player.pos.x);
			int z = (int)floor(player.pos.z);

			if (x != oldX || z != oldZ) {

				std::cout << "Pos: " << x << "," << z << std::endl;

				LevelCell& cell = level[z * levelWidth + x];
				switch (cell) {
				case LevelCell::FLOOR:
					player.size = player.size > 0 ? player.size - 1 : player.size;
					break;
				case LevelCell::GROWSTUFF:
					player.size += 2;
					cell = LevelCell::FLOOR;
					break;
				}

			}
		}
	}

	void Draw(World* world) override {
		Actor::Draw(world);

		for (int z = 0; z < levelWidth; z++) {
			for (int x = 0; x < levelWidth; x++) {
				switch (level[z * levelWidth + x]) {
				case LevelCell::FLOOR:
					DrawPlane(
						Vector3Add(pos, Vector3{ (float)x, 0.0f, (float)z }),
						Vector2{ 1.0f, 1.0f },
						GRAY
					);
					break;
				case LevelCell::GROWSTUFF:
					DrawPlane(
						Vector3Add(pos, Vector3{ (float)x, 0.1f, (float)z }),
						Vector2{ 1.0f, 1.0f },
						DARKBROWN
					);
					break;
				}
			}
		}
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
	cam.position = { 2.5f, 5.0f, 7.0f };
	cam.fovy = 60.0f;
	cam.up = { 0.0f, 1.0f, 0.0f };
	cam.target = { 2.5f, 0.0f, 2.5f };

	Level level = Level();
	root.AddChild(&level);

	while (!WindowShouldClose())
	{
		world.root->Tick(&world);

		BeginDrawing();
		ClearBackground(RAYWHITE);

		BeginMode3D(cam);

		world.root->Draw(&world);
		DrawGrid(10, 1.0);

		EndMode3D();

		std::string playerInfo = "Player Pos: "
			+ std::to_string(level.player.pos.x)
			+ ","
			+ std::to_string(level.player.pos.y)
			+ ","
			+ std::to_string(level.player.pos.z);
		DrawText(playerInfo.c_str(), 10, 10, 24, BLACK);
		EndDrawing();
	}

	CloseWindow();

	return 0;
}