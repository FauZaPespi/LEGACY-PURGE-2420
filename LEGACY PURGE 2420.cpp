#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <utility>
#include "raylib.h"
#include "raymath.h"

class LegacyPurgeCore {
private:
	// --- Gameplay Variables ---
	int fps_objective = 144;
	int health = 100;
	int screenWidth = 1280;
	int screenHeight = 720;

	// --- Physics & Movement ---
	Vector3 playerPos = { 0.0f, 1.0f, 0.0f };
	Vector3 playerVelocity = { 0.0f, 0.0f, 0.0f };
	Vector3 playerSize = { 0.5f, 1.8f, 0.5f };

	float playerSpeed = 8.0f;
	float gravity = -22.0f;      // Downward acceleration
	float jumpForce = 9.0f;      // Upward burst
	bool isGrounded = false;     // Safety check for jumping

	// --- Camera & View ---
	Camera camera = { 0 };
	int cameraMode = CAMERA_FIRST_PERSON;
	float yaw = 0.0f;
	float pitch = 0.0f;
	float mouseSensitivity = 0.003f;

	// --- Environment ---
	struct Wall {
		Vector3 pos;
		float height;
		Color color;
	};
	std::vector<Wall> walls;
	bool isShowingHitbox = false;
	const float cellSize = 3.0f;
	const int mazeSize = 67; // Must be odd for the algorithm

	BoundingBox GetPlayerBox(Vector3 pos) {
		return {
			{ pos.x - playerSize.x / 2, pos.y - playerSize.y / 2, pos.z - playerSize.z / 2 },
			{ pos.x + playerSize.x / 2, pos.y + playerSize.y / 2, pos.z + playerSize.z / 2 }
		};
	}

	void GenerateLabyrinth() {
		walls.clear();
		std::vector<std::vector<int>> maze(mazeSize, std::vector<int>(mazeSize, 1)); // 1 = Wall, 0 = Path

		// Recursive backtracking maze generation (iterative with stack)
		std::vector<std::pair<int, int>> stack;
		int startX = 1;
		int startY = 1;
		maze[startX][startY] = 0;
		stack.push_back({ startX, startY });

		int dx[] = { 0, 0, 2, -2 };
		int dy[] = { 2, -2, 0, 0 };

		while (!stack.empty()) {
			std::pair<int, int> current = stack.back();
			std::vector<int> neighbors;

			for (int i = 0; i < 4; i++) {
				int nx = current.first + dx[i];
				int ny = current.second + dy[i];
				if (nx > 0 && nx < mazeSize - 1 && ny > 0 && ny < mazeSize - 1 && maze[nx][ny] == 1) {
					neighbors.push_back(i);
				}
			}

			if (!neighbors.empty()) {
				int dir = neighbors[GetRandomValue(0, (int)neighbors.size() - 1)];
				int nx = current.first + dx[dir];
				int ny = current.second + dy[dir];

				// Remove wall between
				maze[current.first + dx[dir] / 2][current.second + dy[dir] / 2] = 0;
				maze[nx][ny] = 0;

				stack.push_back({ nx, ny });
			}
			else {
				stack.pop_back();
			}
		}

		// Convert grid to walls
		float offset = -(mazeSize * cellSize) / 1.0f + cellSize / 1.0f;

		for (int i = 0; i < mazeSize; i++) {
			for (int j = 0; j < mazeSize; j++) {
				if (maze[i][j] == 1) {
					float h = (float)GetRandomValue(4, 10);
					Wall w;
					w.pos = { offset + i * cellSize, h / 2.0f, offset + j * cellSize };
					w.height = h;
					w.color = { (uint8_t)GetRandomValue(30, 80), (uint8_t)GetRandomValue(30, 80), (uint8_t)GetRandomValue(30, 80), 255 };
					walls.push_back(w);
				}
			}
		}

		// Set player start at the first path cell
		playerPos = { offset + 1 * cellSize, 1.0f, offset + 1 * cellSize };
	}

public:
	void Run() {
		// Initialization
		InitWindow(screenWidth, screenHeight, "LEGACY PURGE 2420");
		SetTargetFPS(fps_objective);
		DisableCursor();

		// Set Fullscreen and Monitor Logic
		int monitor = GetCurrentMonitor();
		SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
		ToggleFullscreen();

		// Camera Setup
		camera.up = { 0.0f, 1.0f, 0.0f };
		camera.fovy = 75.0f;
		camera.projection = CAMERA_PERSPECTIVE;

		// Generate labyrinth
		GenerateLabyrinth();

		while (!WindowShouldClose()) {
			Update();
			Draw();
		}

		CloseWindow();
	}

	void Update() {
		float dt = GetFrameTime();

		// 1. Toggle UI/Fullscreen
		if (IsKeyPressed(KEY_F10)) isShowingHitbox = !isShowingHitbox;
		if (IsKeyPressed(KEY_F11)) {
			if (IsWindowFullscreen()) {
				ToggleFullscreen();
				SetWindowSize(1280, 720);
				int monitor = GetCurrentMonitor();
				SetWindowPosition((GetMonitorWidth(monitor) - 1280) / 2, (GetMonitorHeight(monitor) - 720) / 2);
			}
			else {
				int monitor = GetCurrentMonitor();
				SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
				ToggleFullscreen();
			}
		}

		// 2. Mouse Look (Yaw/Pitch)
		Vector2 mouseDelta = GetMouseDelta();
		yaw -= mouseDelta.x * mouseSensitivity;
		pitch -= mouseDelta.y * mouseSensitivity;
		pitch = Clamp(pitch, -1.5f, 1.5f); // Limit vertical look

		// 3. Horizontal Movement Calculation
		Vector3 forwardXZ = { sinf(yaw), 0.0f, cosf(yaw) };
		Vector3 rightXZ = { cosf(yaw), 0.0f, -sinf(yaw) };
		Vector3 direction = { 0 };

		if (IsKeyDown(KEY_W)) direction = Vector3Add(direction, forwardXZ);
		if (IsKeyDown(KEY_S)) direction = Vector3Subtract(direction, forwardXZ);
		if (IsKeyDown(KEY_D)) direction = Vector3Subtract(direction, rightXZ);
		if (IsKeyDown(KEY_A)) direction = Vector3Add(direction, rightXZ);

		if (Vector3Length(direction) > 0) direction = Vector3Normalize(direction);

		// Set horizontal velocity based on input
		playerVelocity.x = direction.x * playerSpeed;
		playerVelocity.z = direction.z * playerSpeed;

		// 4. Gravity & Jumping
		playerVelocity.y += gravity * dt; // Apply constant gravity

		if (isGrounded && IsKeyPressed(KEY_SPACE)) {
			playerVelocity.y = jumpForce;
			isGrounded = false;
		}

		// 5. Apply Movement & Collision Resolution
		Vector3 oldPos = playerPos;

		// --- X AXIS ---
		playerPos.x += playerVelocity.x * dt;
		BoundingBox boxX = GetPlayerBox(playerPos);
		for (const auto& w : walls) {
			BoundingBox colBox = {
				{ w.pos.x - cellSize / 2.0f, w.pos.y - w.height / 2.0f, w.pos.z - cellSize / 2.0f },
				{ w.pos.x + cellSize / 2.0f, w.pos.y + w.height / 2.0f, w.pos.z + cellSize / 2.0f }
			};

			if (CheckCollisionBoxes(boxX, colBox)) {
				playerPos.x = oldPos.x;
				break;
			}
		}

		// --- Z AXIS ---
		playerPos.z += playerVelocity.z * dt;
		BoundingBox boxZ = GetPlayerBox(playerPos);
		for (const auto& w : walls) {
			BoundingBox colBox = {
				{ w.pos.x - cellSize / 2.0f, w.pos.y - w.height / 2.0f, w.pos.z - cellSize / 2.0f },
				{ w.pos.x + cellSize / 2.0f, w.pos.y + w.height / 2.0f, w.pos.z + cellSize / 2.0f }
			};

			if (CheckCollisionBoxes(boxZ, colBox)) {
				playerPos.z = oldPos.z;
				break;
			}
		}

		// Move Y
		playerPos.y += playerVelocity.y * dt;
		isGrounded = false;

		// Floor Collision
		if (playerPos.y <= playerSize.y / 2) {
			playerPos.y = playerSize.y / 2;
			playerVelocity.y = 0;
			isGrounded = true;
		}

		// Wall Collision (Vertical - Landing on top or hitting from below)
		BoundingBox playerBox = GetPlayerBox(playerPos);
		for (const auto& w : walls) {
			BoundingBox colBox = {
				{ w.pos.x - cellSize / 2.0f, 0.0f, w.pos.z - cellSize / 2.0f },
				{ w.pos.x + cellSize / 2.0f, w.height, w.pos.z + cellSize / 2.0f }
			};

			if (CheckCollisionBoxes(playerBox, colBox)) {
				if (playerVelocity.y < 0 && oldPos.y > w.height) {
					playerPos.y = w.height + playerSize.y / 2;
					playerVelocity.y = 0;
					isGrounded = true;
				}
				else {
					playerPos.y = oldPos.y;
					playerVelocity.y = 0;
				}
			}
		}

		// 6. Finalize Camera
		Vector3 forward = { cosf(pitch) * sinf(yaw), sinf(pitch), cosf(pitch) * cosf(yaw) };
		camera.position = { playerPos.x, playerPos.y + 0.6f, playerPos.z };
		camera.target = Vector3Add(camera.position, forward);
	}

	void Draw() {
		BeginDrawing();
		ClearBackground(RAYWHITE);

		BeginMode3D(camera);
		DrawPlane({ 0.0f, 0.0f, 0.0f }, { 128.0f, 128.0f }, LIGHTGRAY); // Larger floor for the maze

		// Maze Walls
		for (const auto& w : walls) {
			DrawCube(w.pos, cellSize, w.height, cellSize, w.color);
			DrawCubeWires(w.pos, cellSize, w.height, cellSize, Fade(BLACK, 0.3f));
		}

		if (isShowingHitbox) {
			DrawCubeWires(playerPos, playerSize.x, playerSize.y, playerSize.z, DARKPURPLE);
		}
		EndMode3D();

		// UI - Player Stats
		DrawRectangle(10, GetScreenHeight() - 110, 200, 100, Fade(SKYBLUE, 0.5f));
		DrawRectangleLines(10, GetScreenHeight() - 110, 200, 100, BLUE);
		DrawText("PLAYER STATUS", 20, GetScreenHeight() - 100, 10, DARKGRAY);
		DrawText(TextFormat("Health: %d", health), 20, GetScreenHeight() - 80, 20, BLACK);
		DrawText(isGrounded ? "Grounded" : "In Air", 20, GetScreenHeight() - 50, 15, isGrounded ? DARKGREEN : MAROON);

		// UI - Instructions
		DrawRectangle(10, 10, 250, 80, Fade(DARKGRAY, 0.3f));
		DrawText("WASD to Move", 20, 20, 10, BLACK);
		DrawText("SPACE to Jump", 20, 35, 10, BLACK);
		DrawText("F10: Toggle Hitbox | F11: Fullscreen", 20, 50, 10, BLACK);

		EndDrawing();
	}
};

int main() {
	LegacyPurgeCore game;
	game.Run();
	return 0;
}