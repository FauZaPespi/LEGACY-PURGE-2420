#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <utility>
#include "raylib.h"
#include "raymath.h"
#include "Entities/PHPEmployee.h"
#include "Entities/Bullet.h"

enum GameState { PLAYING, DEATH_SCREEN };

class LegacyPurgeCore {
private:
	// --- Engine & State ---
	GameState currentState = PLAYING;
	float hurtTimer = 0.0f;
	float damageCooldown = 0.0f; // Global damage cooldown

	// --- Gameplay Variables ---
	int fps_objective = 144;
	int health = 100;
	int screenWidth = 1280;
	int screenHeight = 720;

	// --- Entities ---
	Texture2D phpTexture;
	std::vector<PHPEmployee*> enemies;

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

	// --- Gun System ---
	Texture2D gunTextureStill;
	Texture2D gunTextureShooting;
	bool isShooting = false;
	float shootCooldown = 0.0f;
	float shootCooldownMax = 0.3f;
	float reloadTimer = 0.0f;
	float reloadTimeMax = 2.0f;
	bool isReloading = false;
	int ammo = 12;
	int maxAmmo = 12;
	float shootAnimTimer = 0.0f;
	std::vector<Bullet> bulletList;
	float bulletSpeed = 50.0f;

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

	void Restart() {
		health = 100;
		currentState = PLAYING;
		GenerateLabyrinth();
		// Re-spawn enemies
		for (auto e : enemies) delete e;
		enemies.clear();
		float offset = -(mazeSize * cellSize) / 1.0f + cellSize / 1.0f;
		int enemyCount = (mazeSize * mazeSize) / 40;
		for (int i = 0; i < enemyCount; i++) {
			Vector3 pos = {
				offset + (float)GetRandomValue(1, mazeSize - 2) * cellSize,
				1.5f,
				offset + (float)GetRandomValue(1, mazeSize - 2) * cellSize
			};
			enemies.push_back(new PHPEmployee(pos, phpTexture, 2.0f));
		}
		EnableCursor();
		DisableCursor();
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

		// Load PHP Texture
		phpTexture = LoadTexture("assets/php.png");
		if (phpTexture.id == 0) {
			// Fallback: Create a simple purple texture if file not found
			Image image = GenImageChecked(64, 64, 32, 32, PURPLE, BLACK);
			phpTexture = LoadTextureFromImage(image);
			UnloadImage(image);
		}

		// Load Gun Textures
		gunTextureStill = LoadTexture("assets/gun_still.png");
		if (gunTextureStill.id == 0) {
			Image gunImg = GenImageChecked(128, 128, 32, 32, GRAY, DARKGRAY);
			gunTextureStill = LoadTextureFromImage(gunImg);
			UnloadImage(gunImg);
		}
		gunTextureShooting = LoadTexture("assets/gun_shooting.png");
		if (gunTextureShooting.id == 0) {
			gunTextureShooting = gunTextureStill; // Fallback to still texture
		}

		// Initial Spawn
		float offset = -(mazeSize * cellSize) / 1.0f + cellSize / 1.0f;
		int enemyCount = (mazeSize * mazeSize) / 40;
		for (int i = 0; i < enemyCount; i++) {
			Vector3 pos = {
				offset + (float)GetRandomValue(1, mazeSize - 2) * cellSize,
				1.5f,
				offset + (float)GetRandomValue(1, mazeSize - 2) * cellSize
			};
			enemies.push_back(new PHPEmployee(pos, phpTexture, 2.0f));
		}

		while (!WindowShouldClose()) {
			Update();
			Draw();
		}

		// Cleanup
		UnloadTexture(phpTexture);
		UnloadTexture(gunTextureStill);
		if (gunTextureShooting.id != gunTextureStill.id) {
			UnloadTexture(gunTextureShooting);
		}
		for (auto e : enemies) delete e;
		enemies.clear();

		CloseWindow();
	}

	void Update() {
		float dt = GetFrameTime();

		if (currentState == DEATH_SCREEN) {
			EnableCursor();
			if (IsKeyPressed(KEY_R)) {
				Restart();
				currentState = PLAYING;
				DisableCursor();
			}
			return;
		}

		if (hurtTimer > 0) hurtTimer -= dt;
		if (damageCooldown > 0) damageCooldown -= dt;

		// --- Gun System Update ---
		// Cooldowns
		if (shootCooldown > 0) shootCooldown -= dt;
		if (shootAnimTimer > 0) shootAnimTimer -= dt;
		else isShooting = false;

		// Reload
		if (isReloading) {
			reloadTimer -= dt;
			if (reloadTimer <= 0) {
				isReloading = false;
				ammo = maxAmmo;
			}
		}

		// Shooting input
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !isReloading && shootCooldown <= 0 && ammo > 0) {
			// Fire!
			ammo--;
			shootCooldown = shootCooldownMax;
			isShooting = true;
			shootAnimTimer = 0.1f;

			// Spawn bullet
			Vector3 forward = { cosf(pitch) * sinf(yaw), sinf(pitch), cosf(pitch) * cosf(yaw) };
			Vector3 bulletStart = Vector3Add(camera.position, Vector3Scale(forward, 1.0f));
			Vector3 bulletVel = Vector3Scale(forward, bulletSpeed);
			bulletList.push_back(Bullet(bulletStart, bulletVel));

			// Start reload if empty
			if (ammo <= 0) {
				isReloading = true;
				reloadTimer = reloadTimeMax;
			}
		}

		// Manual reload with R
		if (IsKeyPressed(KEY_R) && !isReloading && ammo < maxAmmo) {
			isReloading = true;
			reloadTimer = reloadTimeMax;
		}

		// Update bullets
		for (auto& bullet : bulletList) {
			bullet.Update(dt);
		}

		// Remove inactive bullets
		bulletList.erase(std::remove_if(bulletList.begin(), bulletList.end(),
			[](const Bullet& b) { return !b.active; }), bulletList.end());

		// Bullet-Enemy collision
		for (auto& bullet : bulletList) {
			if (!bullet.active) continue;
			for (auto e : enemies) {
				if (!e->active) continue;
				if (CheckCollisionBoxes(bullet.GetBoundingBox(), e->GetBoundingBox())) {
					e->TakeDamage(30.0f);
					bullet.Deactivate();
					break;
				}
			}
		}

		// Update Enemies
		BoundingBox playerBox = GetPlayerBox(playerPos);
		for (size_t i = 0; i < enemies.size(); i++) {
			PHPEmployee* e = enemies[i];
			e->Update(dt, playerPos);
			
			// PHP vs PHP collision
			for (size_t j = i + 1; j < enemies.size(); j++) {
				e->HandleEntityCollision(enemies[j]);
			}

			// Wall Collision for enemies
			for (const auto& w : walls) {
				// Optimization: only check walls within a certain distance
				if (Vector3Distance(e->GetPosition(), w.pos) < cellSize * 2.0f) {
					e->HandleWallCollision(w.pos, cellSize, w.height);
				}
			}

			// Enemy-Player Collision with cooldown (only active enemies)
			if (e->active && damageCooldown <= 0 && CheckCollisionBoxes(e->GetBoundingBox(), playerBox)) {
				health -= 10;
				hurtTimer = 0.25f; // 250ms red flash
				damageCooldown = 1.0f; // 1 second cooldown
				e->SetAttacking();
				if (health <= 0) {
					health = 0;
					currentState = DEATH_SCREEN;
				}
			}
		}

		// Remove dead enemies from the list
		for (auto it = enemies.begin(); it != enemies.end(); ) {
			if (!(*it)->active) {
				delete *it;
				it = enemies.erase(it);
			} else {
				++it;
			}
		}

		// Remove dead enemies from the list
		for (auto it = enemies.begin(); it != enemies.end(); ) {
			if (!(*it)->active) {
				delete *it;
				it = enemies.erase(it);
			} else {
				++it;
			}
		}

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
		playerBox = GetPlayerBox(playerPos);
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

		// Draw Enemies
		for (auto e : enemies) e->Draw(camera, isShowingHitbox);

		// Draw Bullets (glowing yellow spheres)
		for (const auto& bullet : bulletList) {
			if (bullet.active) {
				DrawSphere(bullet.position, 0.15f, YELLOW);
			}
		}

		// Draw Bullets
		for (const auto& bullet : bulletList) {
			if (bullet.active) {
				DrawCube(bullet.position, 0.15f, 0.15f, 0.15f, YELLOW);
			}
		}

		if (isShowingHitbox) {
			DrawCubeWires(playerPos, playerSize.x, playerSize.y, playerSize.z, DARKPURPLE);
		}
		EndMode3D();

		// Draw Gun (first-person, bottom-right)
		Texture2D gunTex = isShooting ? gunTextureShooting : gunTextureStill;
		int gunWidth = 500;
		int gunHeight = 500;
		int screenW = GetScreenWidth();
		int screenH = GetScreenHeight();
		Rectangle destRec = { (float)(screenW - gunWidth - 120), (float)(screenH - gunHeight + 150), (float)gunWidth, (float)gunHeight };
		DrawTexturePro(gunTex,
			{ 0, 0, (float)gunTex.width, (float)gunTex.height },
			destRec,
			{ 0, 0 }, 0.0f, WHITE);

		// Draw Crosshair (center of screen)
		int cx = GetScreenWidth() / 2;
		int cy = GetScreenHeight() / 2;
		int chSize = 10;
		DrawLine(cx - chSize, cy, cx + chSize, cy, BLACK);
		DrawLine(cx, cy - chSize, cx, cy + chSize, BLACK);
		DrawCircleLines(cx, cy, chSize, BLACK);

		// Hurt cam flash
		if (hurtTimer > 0) {
			DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RED, 0.25f));
		}

		if (currentState == DEATH_SCREEN) {
			DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.8f));
			
			const char* deathText = "YOU HAVE BEEN PURGED";
			int fontSize = 60;
			int textWidth = MeasureText(deathText, fontSize);
			DrawText(deathText, GetScreenWidth() / 2 - textWidth / 2, GetScreenHeight() / 2 - 100, fontSize, RED);

			const char* subText = "Press 'R' to Restart Legacy";
			int subFontSize = 20;
			int subTextWidth = MeasureText(subText, subFontSize);
			DrawText(subText, GetScreenWidth() / 2 - subTextWidth / 2, GetScreenHeight() / 2 + 20, subFontSize, RAYWHITE);
		}
		else {
			// UI - Player Stats
		DrawRectangle(10, GetScreenHeight() - 110, 200, 100, Fade(SKYBLUE, 0.5f));
		DrawRectangleLines(10, GetScreenHeight() - 110, 200, 100, BLUE);
		DrawText("PLAYER STATUS", 20, GetScreenHeight() - 100, 10, DARKGRAY);
		DrawText(TextFormat("Health: %d", health), 20, GetScreenHeight() - 80, 20, BLACK);
		DrawText(isGrounded ? "Grounded" : "In Air", 20, GetScreenHeight() - 50, 15, isGrounded ? DARKGREEN : MAROON);

		// UI - Ammo / Gun Status
		DrawRectangle(GetScreenWidth() - 160, GetScreenHeight() - 60, 150, 50, Fade(DARKGRAY, 0.5f));
		DrawRectangleLines(GetScreenWidth() - 160, GetScreenHeight() - 60, 150, 50, GRAY);
		if (isReloading) {
			float reloadPercent = 1.0f - (reloadTimer / reloadTimeMax);
			DrawText("RELOADING...", GetScreenWidth() - 150, GetScreenHeight() - 45, 15, YELLOW);
			// Reload progress bar
			DrawRectangle(GetScreenWidth() - 150, GetScreenHeight() - 25, 130 * reloadPercent, 10, GREEN);
		} else {
			DrawText(TextFormat("Ammo: %d / %d", ammo, maxAmmo), GetScreenWidth() - 150, GetScreenHeight() - 45, 20, ammo > 0 ? WHITE : RED);
		}

		// UI - Instructions
		DrawRectangle(10, 10, 280, 100, Fade(DARKGRAY, 0.3f));
		DrawText("WASD to Move", 20, 20, 10, BLACK);
		DrawText("SPACE to Jump", 20, 35, 10, BLACK);
		DrawText("LMB: Shoot | R: Reload", 20, 50, 10, BLACK);
		DrawText("F10: Toggle Hitbox | F11: Fullscreen", 20, 65, 10, BLACK);
		}

		EndDrawing();
	}
};

int main() {
	LegacyPurgeCore game;
	game.Run();
	return 0;
}