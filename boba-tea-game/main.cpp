#include <raylib.h>
#include "Actor.h"
#include "World.h"
#include <raymath.h>
#include <vector>
#include <cmath>
#include <string>
#include <iostream>


class ModelActor : public Actor {
public:

	Model model;

	ModelActor(const char* filename) : Actor() {
		model = LoadModel(filename);
	}

	void Draw3D(World* world) override {
		Actor::Draw3D(world);
		DrawModel(model, Vector3Zero(), 1.0f, WHITE);
	}
};


class Player : public Actor {
public:
	ModelActor ballModel;
	int size = 0;

	float sizeScale = 0.02f;

	Player() : Actor(), ballModel("assets/ball.gltf") {
		AddChild(&ballModel);
	}

	void Tick(World* world) override {
		Actor::Tick(world);

		ballModel.pos.y = 0.5f;
		ballModel.scale = 0.1f + size * sizeScale;
	}
};

class TextActor : public Actor {
public:
	std::string text;
	int fontSize = 64;
	Color color = BLACK;
	float anchor = 0.0f;

	void Draw2D(World* world) override {
		Actor::Draw2D(world);

		int width = MeasureText(text.c_str(), fontSize);
		int height = MeasureText("M", fontSize);
		DrawText(text.c_str(), (int)pos.x - (width * anchor), (int)pos.y - (height * anchor), fontSize, color);
	}
};

enum class LevelCell {
	FLOOR,
	GROWSTUFF,
	SPIKE,
	GOAL,
};

class Level : public Actor {
public:
	Player player;

	int goalMin = 23, goalMax = INT32_MAX;

	int levelWidth = 5;
	std::vector<LevelCell> level;

	std::vector<ModelActor*> props;

	TextActor youWinText;

	bool levelComplete = false;

	Level() : Actor() {
		AddChild(&player);

		AddChild(&youWinText);
		youWinText.visible = false;
		youWinText.text = "You Win!";
		youWinText.pos.x = GetScreenWidth() / 2.0f;
		youWinText.pos.y = GetScreenHeight() / 2.0f;
		youWinText.anchor = 0.5;

		level = std::vector({
			LevelCell::SPIKE, LevelCell::SPIKE, LevelCell::SPIKE, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF,
			LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF,
			LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GOAL, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF,
			LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::GROWSTUFF,
			LevelCell::GROWSTUFF, LevelCell::GROWSTUFF, LevelCell::SPIKE, LevelCell::SPIKE, LevelCell::GROWSTUFF,
		});

		player.pos = { 0.0f, 0.0f, 1.0f };

		for (int z = 0; z < levelWidth; z++) {
			for (int x = 0; x < levelWidth; x++) {
				switch (level[z * levelWidth + x]) {
				case LevelCell::SPIKE:
					auto *modelActor = props.emplace_back(new ModelActor("assets/spikes.gltf"));
					AddChild(modelActor);
					modelActor->pos = { (float)x, 0.0f, (float)z };
					break;
				}
			}
		}
	}

	virtual ~Level() {
		for (const auto* prop : props) {
			delete prop;
		}
	}

	void Tick(World* world) override {
		Actor::Tick(world);

		if (levelComplete) {
			youWinText.visible = true;
		} else {
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
				int x = (int)floor(Clamp(player.pos.x + playerMoveDir.x, 0, levelWidth - 1));
				int z = (int)floor(Clamp(player.pos.z + playerMoveDir.z, 0, levelWidth - 1));

				// If the new position is in bounds
				if (x != oldX || z != oldZ) {

					std::cout << "Pos: " << x << "," << z << std::endl;

					bool canMoveThere = true;

					LevelCell& cell = level[z * levelWidth + x];
					switch (cell) {
					case LevelCell::FLOOR:
						player.size = player.size > 0 ? player.size - 1 : player.size;
						break;
					case LevelCell::GROWSTUFF:
						player.size += 2;
						cell = LevelCell::FLOOR;
						break;
					case LevelCell::SPIKE:
						std::cout << "Player has gone into spikes, game over" << std::endl;
						break;
					case LevelCell::GOAL:
						if (player.size >= goalMin && player.size < goalMax) {
							levelComplete = true;
						}
						break;
					}

					if (canMoveThere) {
						player.pos.x = x;
						player.pos.y = 0.0f;
						player.pos.z = z;
					}

				}
			}
		}
	}

	void Draw3D(World* world) override {
		Actor::Draw3D(world);

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
				case LevelCell::GOAL:
					DrawCircle3D(
						Vector3Add(pos, Vector3{ (float)x, 0.0f, (float)z }),
						0.5f, Vector3UnitY, 0.0f, RED
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

	//Model plateModel = LoadModel("assets/testPlate.gltf");

	while (!WindowShouldClose())
	{
		world.root->Tick(&world);

		BeginDrawing();
		ClearBackground(RAYWHITE);

		BeginMode3D(cam);

		//DrawModel(plateModel, Vector3Zero(), 10.0f, WHITE);
		//cam.position = Vector3Su level.player.globalPos

		world.root->Draw3D(&world);
		DrawGrid(10, 1.0);

		EndMode3D();


		world.root->Draw2D(&world);

		std::string playerInfo = "Player Pos: "
			+ std::to_string(level.player.pos.x)
			+ ","
			+ std::to_string(level.player.pos.y)
			+ ","
			+ std::to_string(level.player.pos.z);
		DrawText(playerInfo.c_str(), 10, 10, 24, BLACK);
		DrawText(("Size: " + std::to_string(level.player.size)).c_str(), 10, 30, 24, BLACK);
		EndDrawing();
	}

	CloseWindow();

	return 0;
}