#ifndef LARAVEL_BOSS_H
#define LARAVEL_BOSS_H

#include "Entity.h"
#include "raymath.h"
#include <vector>

enum LARAVEL_STATE { LARAVEL_UNACTIVE, LARAVEL_CHASE, LARAVEL_ATTACK_RETREAT, LARAVEL_ATTACK_WAIT, LARAVEL_HEAL, LARAVEL_SUMMON };

class LARAVELBoss : public Entity {
private:
    Texture2D texture;
    float billboardScale;
    LARAVEL_STATE state = LARAVEL_UNACTIVE;
    float stateTimer = 0.0f;
    float attackTimer = 0.0f;
    float speed = 6.0f;
    float gravity = -22.0f;
    float friction = 0.9f;
    float health = 1000.0f;
    float maxHealth = 1000.0f;
    float shootCooldown = 0.0f;
    float shootCooldownMax = 1.5f;
    bool isSummoning = false;
    float summonCooldown = 0.0f;
    float summonCooldownMax = 30.0f;

public:
    LARAVELBoss(Vector3 pos, Texture2D tex, float scale = 2.0f)
        : Entity(pos, { scale * 3.0f, scale * 2.0f, scale * 3.0f }), texture(tex), billboardScale(scale) {}

    void TakeDamage(float damage) {
        health -= damage;
        if (health <= 0) {
            health = 0;
            active = false;
        }
    }

    float GetHealth() const { return health; }
    float GetMaxHealth() const { return maxHealth; }
    bool IsSummoning() const { return isSummoning; }

    void StartBossFight() {
        state = LARAVEL_CHASE;
        active = true;
    }

    // Main Update for boss state machine (with additional parameters)
    void UpdateWithEnemies(float dt, Vector3 playerPos, std::vector<class PHPEmployee*>& enemies, bool& bossRoomCleared) {
        if (!active) return;

        // --- AI Logic ---
        Vector3 direction = Vector3Subtract(playerPos, position);
        direction.y = 0;
        float dist = Vector3Length(direction);

        if (dist > 0) direction = Vector3Normalize(direction);

        // State Machine
        if (state == LARAVEL_CHASE) {
            // Chase player
            velocity.x = direction.x * speed;
            velocity.z = direction.z * speed;

            // If close enough, prepare to attack
            if (dist < 15.0f) {
                state = LARAVEL_ATTACK_WAIT;
                stateTimer = 1.0f; // Pause for 1 second before attacking
            }

            // Check for summoning (at 50% HP)
            if (health <= maxHealth * 0.5f && summonCooldown <= 0) {
                isSummoning = true;
                summonCooldown = summonCooldownMax;
                // Summon 2-4 PHP minions
                int summonCount = GetRandomValue(2, 4);
                for (int i = 0; i < summonCount; i++) {
                    Vector3 spawnPos = {
                        position.x + (float)GetRandomValue(-5, 5),
                        1.5f,
                        position.z + (float)GetRandomValue(-5, 5)
                    };
                    enemies.push_back(new PHPEmployee(spawnPos, texture, 2.0f));
                }
                isSummoning = false;
            }
        }
        else if (state == LARAVEL_ATTACK_WAIT) {
            stateTimer -= dt;
            if (stateTimer <= 0) {
                // Attack!
                state = LARAVEL_ATTACK_RETREAT;
                stateTimer = 3.0f; // Retreat for 3 seconds
                shootCooldown = shootCooldownMax;
            }
        }
        else if (state == LARAVEL_ATTACK_RETREAT) {
            // Move away from player
            Vector3 awayDir = Vector3Scale(direction, -1.0f);
            velocity.x = awayDir.x * (speed * 0.8f);
            velocity.z = awayDir.z * (speed * 0.8f);

            stateTimer -= dt;
            if (stateTimer <= 0) {
                state = LARAVEL_CHASE;
            }

            // Shoot projectiles while retreating
            if (shootCooldown > 0) shootCooldown -= dt;
            if (shootCooldown <= 0 && dist < 20.0f) {
                shootCooldown = shootCooldownMax;
                // Shoot 3 red projectiles at player
                for (int i = -1; i <= 1; i++) {
                    Vector3 bulletVel = {
                        direction.x * 8.0f + (float)i * 2.0f,
                        0.0f,
                        direction.z * 8.0f + (float)i * 2.0f
                    };
                    // Add to boss bullet list (handled by main game)
                }
            }
        }

        // Update summon cooldown
        if (summonCooldown > 0) summonCooldown -= dt;

        // --- Physics ---
        velocity.y += gravity * dt;
        position.x += velocity.x * dt;
        position.z += velocity.z * dt;
        position.y += velocity.y * dt;

        velocity.x *= friction;
        velocity.z *= friction;

        // Ground collision
        if (position.y < size.y / 2) {
            position.y = size.y / 2;
            velocity.y = 0;
        }
    }

    void Update(float dt, Vector3 playerPos) override {
        if (!active) return;

        // Basic update without enemy summoning logic
        // State machine simplified for base class override
        Vector3 direction = Vector3Subtract(playerPos, position);
        direction.y = 0;
        float dist = Vector3Length(direction);

        if (dist > 0) direction = Vector3Normalize(direction);

        // Chase player
        velocity.x = direction.x * speed;
        velocity.z = direction.z * speed;

        // --- Physics ---
        velocity.y += gravity * dt;
        position.x += velocity.x * dt;
        position.z += velocity.z * dt;
        position.y += velocity.y * dt;

        velocity.x *= friction;
        velocity.z *= friction;

        // Ground collision
        if (position.y < size.y / 2) {
            position.y = size.y / 2;
            velocity.y = 0;
        }
    }

    void Draw(Camera camera, bool showHitbox) override {
        if (!active) return;

        // Draw boss as billboard
        DrawBillboard(camera, texture, position, billboardScale, WHITE);

        // Draw health bar above boss
        float healthPercent = health / maxHealth;
        Vector3 barPos = { position.x, position.y + billboardScale + 1.0f, position.z };

        // Bar background
        DrawCube(barPos, 3.0f, 0.3f, 0.5f, RED);
        // Health fill
        DrawCube({ barPos.x - 1.5f + (3.0f * healthPercent) / 2, barPos.y, barPos.z },
                 3.0f * healthPercent, 0.3f, 0.5f, healthPercent > 0.3f ? GREEN : YELLOW);

        if (showHitbox) {
            DrawCubeWires(position, size.x, size.y, size.z, RED);
        }
    }

    void Unload() {
        // Texture managed by Core
    }
};

#endif // LARAVEL_BOSS_H
