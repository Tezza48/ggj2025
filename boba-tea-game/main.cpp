#include <raylib.h>
#include "Actor.h"
#include "World.h"
#include <raymath.h>
#include <vector>
#include <cmath>
#include <string>
#include <iostream>
#include <sstream>
#include <array>

class Timer {
public:
	float time, duration;
	Timer(float duration) : time(0), duration(duration) {

	}

	Timer() {
		time = 0;
		duration = 0;
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

		float c1 = 1.70158f;
		float c2 = c1 * 1.525f;

		return x < 0.5f
			? (powf(2.0f * x, 2.0f) * ((c2 + 1.0f) * 2.0f * x - c2)) / 2.0f
			: (powf(2.0f * x - 2.0f, 2.0f) * ((c2 + 1.0f) * (x * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
	}

	float ProgressEaseOutBack() {
		float x = Progress();

		float c1 = 1.70158f;
		float c3 = c1 + 1.0f;

		return 1.0f + c3 * powf(x - 1.0f, 3.0f) + c1 * powf(x - 1.0f, 2.0f);
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
	int moveStartSize = size;

	float sizeScale = 0.05f;

	Timer moveTimer;
	Vector3 moveStartPos = Vector3Zero();
	Vector3 moveEndPos = Vector3Zero();


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

		moveTimer = Timer(0.2f);
	}

	void Tick(World* world) override {
		Actor::Tick(world);

		moveTimer.Update();
		canMove = moveTimer.isComplete();

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

enum Cell {
	FLOOR,
	GROW,
	SPIKE,
	GOAL,
};

enum class LevelState {
	PLAY,
	LOSS,
	WIN
};

class Level : public Actor {
public:
	Player player;

	int goal;
	int width;
	int height;

	Vector2 startPos;

	const std::vector<Cell> originalLevel;
	std::vector<Cell> level;

	std::vector<ModelActor*> props;

	TextActor youWinText;
	TextActor youLoseText;

	LevelState state = LevelState::PLAY;

	Timer levelChangeTimer;

	Level(
		int width,
		int height,
		int goal,
		Vector2 startPos,
		const std::vector<Cell> data
	) : Actor(),
		goal(goal),
		width(width),
		height(height),
		startPos(startPos) ,
		originalLevel(data)
	{
		for (int i = 0; i < originalLevel.size(); i++) {
			level.push_back(originalLevel[i]);
		}

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

				if (cell == Cell::GOAL) {
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
				case Cell::SPIKE:
					auto* modelActor = props.emplace_back(new ModelActor("assets/spikes.gltf"));
					AddChild(modelActor);
					modelActor->pos = { (float)x, 0.0f, (float)z };
					break;
				}
			}
		}
	}

	Level(const Level& other) :
		Level(
			other.width,
			other.height,
			other.goal,
			other.startPos,
			other.originalLevel
		) {
	}

	virtual ~Level() {
		for (const auto* prop : props) {
			delete prop;
		}
	}

	void Tick(World* world) override {
		Actor::Tick(world);

		levelChangeTimer.Update();

		world->cam->target = Vector3Add(pos, { width / 2.0f, 0.0f, 1.0f + height / 2.0f });
		world->cam->target.x -= 0.5f;
		world->cam->target.z -= 0.5f;
		world->cam->position = Vector3Add(world->cam->target, { 0.0f, 4.0f, 6.0f });


		switch (state) {
		case LevelState::PLAY: {
			if (!player.canMove) break;

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
					bool canMoveThere = true;
					int newSize = player.size;

					Cell& cell = level[z * width + x];
					switch (cell) {
					case Cell::FLOOR:
						newSize = newSize > 0 ? newSize - 1 : newSize;
						break;
					case Cell::GROW:
						newSize += 2;
						cell = Cell::FLOOR;
						break;
					case Cell::SPIKE:
						canMoveThere = false;
						break;
					case Cell::GOAL:
						if (newSize >= goal) {
							WinLevel();
						}
						else {
							FailLevel();
						}
						break;
					}

					if (canMoveThere) {

						player.MoveTo({ (float)x, 0.0f, (float)z }, newSize);
					}

					if (std::count(level.begin(), level.end(), Cell::GROW) == 0 && newSize < goal) {
						FailLevel();
					}
				}
			}
			break;
		}
		case LevelState::WIN:

			youWinText.visible = true;
			player.pos.y -= GetFrameTime();
			return;
			break;

		case LevelState::LOSS:
			youLoseText.visible = true;

			break;
		}
	}

	void Draw3D(World* world) override {
		Actor::Draw3D(world);

		for (int z = 0; z < height; z++) {
			for (int x = 0; x < width; x++) {
				switch (level[z * width + x]) {
				case Cell::GROW:
					DrawPlane(
						Vector3Add(pos, Vector3{ (float)x, 0.1f, (float)z }),
						Vector2{ 1.0f, 1.0f },
						DARKBROWN
					);
					break;
				case Cell::GOAL:
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

private:
	void FailLevel() {
		state = LevelState::LOSS;
		levelChangeTimer = Timer(1.0);
	}

	void WinLevel() {
		state = LevelState::WIN;
		levelChangeTimer = Timer(1.0);
	}
};

int main(void)
{
	InitWindow(1280, 720, "GGJ 2025 Boba Tea Roller");

	Actor root = {};


	Camera cam;
	cam.projection = CAMERA_PERSPECTIVE;
	cam.position = { 2.5f, 5.0f, 7.0f };
	cam.fovy = 40.0f;
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
		{ Cell::FLOOR, Cell::GROW, Cell::GROW, Cell::GROW, Cell::GOAL });

	Level mediumLevel = Level(
		5,
		5,
		21,
		{ 0.0f, 2.0f },
		{
			GROW,	GROW,	GROW,	GROW,	GROW,
			GROW,	SPIKE,	FLOOR,	SPIKE,	GROW,
			FLOOR,	FLOOR,	FLOOR,	FLOOR,	GOAL,
			GROW,	SPIKE,	FLOOR,	SPIKE,	GROW,
			GROW,	GROW,	GROW,	GROW,	GROW
		}
	);

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

	int currentLevelIndex = 0;
	std::vector<Level*> levels = { &straightLineLevel, &mediumLevel, &complexLevel };

	Level* currentLevel = new Level(*levels[currentLevelIndex]);

	root.AddChild(currentLevel);

	while (!WindowShouldClose())
	{
		world.root->Tick(&world);

		if (currentLevel->state == LevelState::WIN && currentLevel->levelChangeTimer.isComplete()) {
			root.RemoveChild(currentLevel);
			currentLevelIndex = (currentLevelIndex + 1) % levels.size();
			delete currentLevel;
			currentLevel = new Level(*levels[currentLevelIndex]);
			root.AddChild(currentLevel);
		}
		else if (currentLevel->state == LevelState::LOSS && currentLevel->levelChangeTimer.isComplete()) {
			root.RemoveChild(currentLevel);
			delete currentLevel;
			currentLevel = new Level(*levels[currentLevelIndex]);
			root.AddChild(currentLevel);
		}

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