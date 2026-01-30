#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include "raylib.h"
#include "raymath.h"

#define MAX_COLUMNS 20

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
	float heights[MAX_COLUMNS] = { 0 };
	Vector3 positions[MAX_COLUMNS] = { 0 };
	Color colors[MAX_COLUMNS] = { 0 };
	bool isShowingHitbox = false;

	BoundingBox GetPlayerBox(Vector3 pos) {
		return {
			{ pos.x - playerSize.x / 2, pos.y - playerSize.y / 2, pos.z - playerSize.z / 2 },
			{ pos.x + playerSize.x / 2, pos.y + playerSize.y / 2, pos.z + playerSize.z / 2 }
		};
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

		// Generate random environment
		for (int i = 0; i < MAX_COLUMNS; i++) {
			heights[i] = (float)GetRandomValue(1, 12);
			positions[i] = { (float)GetRandomValue(-15, 15), heights[i] / 2.0f, (float)GetRandomValue(-15, 15) };
			colors[i] = { (uint8_t)GetRandomValue(20, 255), (uint8_t)GetRandomValue(10, 55), 30, 255 };
		}

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
			int display = GetCurrentMonitor();

			if (IsWindowFullscreen()) {
				int windowWidth = 800;
				int windowHeight = 400;
				SetWindowSize(windowWidth, windowHeight);

				int centerX = (GetMonitorWidth(display) - windowWidth) / 2;
				int centerY = (GetMonitorHeight(display) - windowHeight) / 2;

				SetWindowPosition(centerX, centerY);
				if (IsKeyPressed(KEY_F11)) {

					int display = GetCurrentMonitor();

					if (IsWindowFullscreen()) {
						int windowWidth = 800;
						int windowHeight = 400;

						SetWindowSize(windowWidth, windowHeight);

						int centerX = (GetMonitorWidth(display) - windowWidth) / 2;
						int centerY = (GetMonitorHeight(display) - windowHeight) / 2;

						SetWindowPosition(centerX, centerY);
					}
					else {
						SetWindowSize(GetMonitorWidth(display), GetMonitorHeight(display));
					}
					ToggleFullscreen();
				}
			}
			else {
				SetWindowSize(GetMonitorWidth(display), GetMonitorHeight(display));
			}
			ToggleFullscreen();
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
		// We move horizontal first, check collisions, then vertical
		Vector3 oldPos = playerPos;

		playerPos.x += playerVelocity.x * dt;

		BoundingBox boxX = GetPlayerBox(playerPos);
		for (int i = 0; i < MAX_COLUMNS; i++) {
			BoundingBox colBox = {
				{ positions[i].x - 1.0f, positions[i].y - heights[i] / 2, positions[i].z - 1.0f },
				{ positions[i].x + 1.0f, positions[i].y + heights[i] / 2, positions[i].z + 1.0f }
			};

			if (CheckCollisionBoxes(boxX, colBox)) {
				playerPos.x = oldPos.x;
				break;
			}
		}

		// --- Z AXIS ---
		playerPos.z += playerVelocity.z * dt;

		BoundingBox boxZ = GetPlayerBox(playerPos);
		for (int i = 0; i < MAX_COLUMNS; i++) {
			BoundingBox colBox = {
				{ positions[i].x - 1.0f, positions[i].y - heights[i] / 2, positions[i].z - 1.0f },
				{ positions[i].x + 1.0f, positions[i].y + heights[i] / 2, positions[i].z + 1.0f }
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

		// Column Collision (Vertical - Landing on top)
		BoundingBox playerBox = GetPlayerBox(playerPos);
		for (int i = 0; i < MAX_COLUMNS; i++) {
			BoundingBox colBox = {
				{ positions[i].x - 1.0f, 0.0f, positions[i].z - 1.0f },
				{ positions[i].x + 1.0f, heights[i], positions[i].z + 1.0f }
			};

			if (CheckCollisionBoxes(playerBox, colBox)) {
				// If we are falling and hit the top
				if (playerVelocity.y < 0 && oldPos.y > heights[i]) {
					playerPos.y = heights[i] + playerSize.y / 2;
					playerVelocity.y = 0;
					isGrounded = true;
				}
				else {
					// Hit from below
					playerPos.y = oldPos.y;
					playerVelocity.y = 0;
				}
			}
		}

		// 6. Finalize Camera
		Vector3 forward = { cosf(pitch) * sinf(yaw), sinf(pitch), cosf(pitch) * cosf(yaw) };
		camera.position = { playerPos.x, playerPos.y + 0.6f, playerPos.z }; // Eyes slightly below top of head
		camera.target = Vector3Add(camera.position, forward);
	}

	void Draw() {
		BeginDrawing();
		ClearBackground(RAYWHITE);

		BeginMode3D(camera);
		DrawPlane({ 0.0f, 0.0f, 0.0f }, { 64.0f, 64.0f }, LIGHTGRAY); // Floor

		// Walls
		DrawCube({ -32.0f, 5.0f, 0.0f }, 1.0f, 10.0f, 64.0f, BLUE);
		DrawCube({ 32.0f, 5.0f, 0.0f }, 1.0f, 10.0f, 64.0f, LIME);
		DrawCube({ 0.0f, 5.0f, 32.0f }, 64.0f, 10.0f, 1.0f, GOLD);

		// Environment Columns
		for (int i = 0; i < MAX_COLUMNS; i++) {
			DrawCube(positions[i], 2.0f, heights[i], 2.0f, colors[i]);
			DrawCubeWires(positions[i], 2.0f, heights[i], 2.0f, MAROON);
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