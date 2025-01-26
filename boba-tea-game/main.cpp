#include <raylib.h>
#include "Actor.h"
#include "World.h"
#include <raymath.h>
#include <vector>
#include <cmath>
#include <string>
#include <iostream>
#include <sstream>

class Timer {
public:
	float time, duration;
	Timer(float duration) : time(0), duration(duration) {

	}

	Timer() : duration(0) {

	}

	void Update() {
		time += GetFrameTime();
	}

	float isComplete() {
		return time > duration;
	}

	float Progress() {
		return Clamp(time / duration, 0.0f, 1.0f);
	}

	// https://easings.net/#easeInOutBack
	float ProgressEaseInOutBack() {
		float x = Progress();

		float c1 = 1.70158;
		float c2 = c1 * 1.525;

		return x < 0.5
			? (powf(2 * x, 2) * ((c2 + 1) * 2 * x - c2)) / 2
			: (powf(2 * x - 2, 2) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;
	}

	float ProgressEaseOutBack() {
		float x = Progress();

		float c1 = 1.70158;
		float c3 = c1 + 1;

		return 1 + c3 * powf(x - 1, 3) + c1 * powf(x - 1, 2);
	}
};

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

	float sizeScale = 0.05f;

	Timer moveTimer;
	Vector3 moveStartPos = Vector3Zero();
	Vector3 moveEndPos = Vector3Zero();

	int moveStartSize;

	bool canMove = true;

	Player() : Actor(), ballModel("assets/ball.gltf") {
		AddChild(&ballModel);
	}

	void SetStartPos(Vector3 position) {
		pos = position;
		moveStartPos = position;
		moveEndPos = position;
	}

	void MoveTo(Vector3 newPos, int newSize) {
		moveStartPos = pos;
		moveEndPos = newPos;

		moveStartSize = size;
		size = newSize;

		moveTimer = Timer(0.2);
	}

	void Tick(World* world) override {
		Actor::Tick(world);

		moveTimer.Update();
		canMove = moveTimer.isComplete();


		std::cout << "Running the move tween logic" << std::endl;
		pos = Vector3Lerp(moveStartPos, moveEndPos, moveTimer.ProgressEaseOutBack());
		ballModel.scale = 0.2f + Lerp((float)moveStartSize, (float)size, moveTimer.ProgressEaseInOutBack()) * sizeScale;
		ballModel.pos.y = 0.5f;
	}

private: 
	void UpdateBallScale() {

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

enum LevelCell {
	FLOOR,
	GROW,
	SPIKE,
	GOAL,
};

class Level : public Actor {
public:
	Player player;

	int goal;
	int width;
	int height;

	std::vector<LevelCell> level;

	std::vector<ModelActor*> props;

	TextActor youWinText;
	TextActor youLoseText;

	bool levelComplete = false;

	bool levelFailed = false;

	Level(int width, int height, int goal, Vector2 startPos, std::vector<LevelCell> data) : Actor(), goal(goal), width(width), height(height), level(data) {
		AddChild(&player);

		AddChild(&youWinText);
		youWinText.visible = false;
		youWinText.text = "You Win!";
		youWinText.pos.x = GetScreenWidth() / 2.0f;
		youWinText.pos.y = GetScreenHeight() / 2.0f;
		youWinText.anchor = 0.5;

		AddChild(&youLoseText);
		youLoseText.visible = false;
		youLoseText.text = "you lose\nbaby boba too smol :(";
		youLoseText.pos = youWinText.pos;
		youLoseText.anchor = youWinText.anchor;

		player.SetStartPos({ startPos.x, 0.0f, startPos.y });

		for (int z = 0; z < height; z++) {
			for (int x = 0; x < width; x++) {
				auto& cell = level[z * width + x];

				if (cell == LevelCell::GOAL) {
					auto* floorModel = props.emplace_back(new ModelActor("assets/floorGoal.gltf"));
					AddChild(floorModel);
					floorModel->pos = { (float)x, 0.0f, (float)z };
				}
				else {
					auto* floorModel = props.emplace_back(new ModelActor("assets/floorRegular.gltf"));
					AddChild(floorModel);
					floorModel->pos = { (float)x, 0.0f, (float)z };
				}
				switch (cell) {
				case LevelCell::SPIKE:
					auto* modelActor = props.emplace_back(new ModelActor("assets/spikes.gltf"));
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

		world->cam->target = Vector3Add(pos, { width / 2.0f, 0.0f, height / 2.0f });
		world->cam->target.x -= 0.5f;
		world->cam->target.z -= 0.5f;
		world->cam->position = Vector3Add(world->cam->target, { 0.0f, 3.0f, 4.0f });


		if (levelComplete) {
			youWinText.visible = true;
			player.pos.y -= GetFrameTime();
			return;
		}
		if (levelFailed) {
			youLoseText.visible = true;
		}

		if (player.canMove) {
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
				int x = (int)floor(Clamp(player.pos.x + playerMoveDir.x, 0, width - 1));
				int z = (int)floor(Clamp(player.pos.z + playerMoveDir.z, 0, height - 1));

				// If the new position is in bounds
				if (x != oldX || z != oldZ) {

					std::cout << "Pos: " << x << "," << z << std::endl;

					bool canMoveThere = true;
					int newSize = player.size;

					LevelCell& cell = level[z * width + x];
					switch (cell) {
					case LevelCell::FLOOR:
						newSize = newSize > 0 ? newSize - 1 : newSize;
						break;
					case LevelCell::GROW:
						newSize += 2;
						cell = LevelCell::FLOOR;
						break;
					case LevelCell::SPIKE:
						canMoveThere = false;
						break;
					case LevelCell::GOAL:
						if (newSize >= goal) {
							levelComplete = true;
						}
						else {
							levelFailed = true;
						}
						break;
					}

					if (canMoveThere) {

						player.MoveTo({ (float)x, 0.0f, (float)z }, newSize);
					}

					if (std::count(level.begin(), level.end(), LevelCell::GROW) == 0 && newSize < goal) {
						levelFailed = true;
					}
				}
			}
		}
	}

	void Draw3D(World* world) override {
		Actor::Draw3D(world);

		for (int z = 0; z < height; z++) {
			for (int x = 0; x < width; x++) {
				switch (level[z * width + x]) {
				case LevelCell::GROW:
					DrawPlane(
						Vector3Add(pos, Vector3{ (float)x, 0.1f, (float)z }),
						Vector2{ 1.0f, 1.0f },
						DARKBROWN
					);
					break;
				case LevelCell::GOAL:
					break;
				}
			}
		}
	}

	void Draw2D(World* world) override {
		Actor::Draw2D(world);

		int fontSize = 40;
		std::stringstream text;
		text << "Size: " << player.size << "\t\tGoal: " << goal << std::endl;
		int width = MeasureText(text.str().c_str(), fontSize);
		DrawText(text.str().c_str(), (GetScreenWidth() - width) / 2, 10, fontSize, ORANGE);
	}
};

int main(void)
{
	InitWindow(1280, 720, "GGJ 2025 Boba Tea Roller");

	Actor root = {};


	Camera cam;
	cam.projection = CAMERA_PERSPECTIVE;
	cam.position = { 2.5f, 5.0f, 7.0f };
	cam.fovy = 60.0f;
	cam.up = { 0.0f, 1.0f, 0.0f };
	cam.target = { 2.5f, 0.0f, 2.5f };

	World world = {
		&root,
		&cam
	};

	const char* testLevelData =
		"size 5 1\n"
		"goal 8\n"
		"FFFFG\n";


	Level straightLineLevel = Level(
		5,
		1,
		6,
		{ 0, 0 },
		{ LevelCell::FLOOR, LevelCell::GROW, LevelCell::GROW, LevelCell::GROW, LevelCell::GOAL });

	Level complexLevel = Level(
		5,
		5,
		18,
		{ 2.0f, 2.0f },
		{
			GROW, GROW, SPIKE, GROW, GROW,
			GROW, GROW, FLOOR, GROW, GROW,
			SPIKE, SPIKE, FLOOR, GOAL, SPIKE,
			GROW, SPIKE, FLOOR, FLOOR, GROW,
			GROW, GROW, FLOOR, GROW, GROW
		}
	);

	//root.AddChild(&straightLineLevel);
	root.AddChild(&complexLevel);

	while (!WindowShouldClose())
	{
		world.root->Tick(&world);

		BeginDrawing();
		ClearBackground(LIGHTGRAY);

		BeginMode3D(cam);

		world.root->Draw3D(&world);
		//DrawGrid(10, 1.0);

		EndMode3D();


		world.root->Draw2D(&world);

		//std::string playerInfo = "Player Pos: "
		//	+ std::to_string(straightLineLevel.player.pos.x)
		//	+ ","
		//	+ std::to_string(straightLineLevel.player.pos.y)
		//	+ ","
		//	+ std::to_string(straightLineLevel.player.pos.z);
		//DrawText(playerInfo.c_str(), 10, 10, 24, BLACK);
		//DrawText(("Size: " + std::to_string(straightLineLevel.player.size)).c_str(), 10, 30, 24, BLACK);
		EndDrawing();
	}

	CloseWindow();

	return 0;
}