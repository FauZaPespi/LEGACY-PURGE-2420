#ifndef PHP_EMPLOYEE_H
#define PHP_EMPLOYEE_H

#include "Entity.h"
#include "raymath.h"
#include <vector>

enum PHPState { CHASE, ATTACK_RETREAT, ATTACK_WAIT };

class PHPEmployee : public Entity {
private:
    Texture2D texture;
    float billboardScale;
    PHPState state = CHASE;
    float stateTimer = 0.0f;
    float speed = 4.0f;
    float gravity = -22.0f;
    float friction = 0.9f;
    float health = 100.0f;

public:
    PHPEmployee(Vector3 pos, Texture2D tex, float scale = 2.0f)
        : Entity(pos, { scale * 0.5f, scale, scale * 0.5f }), texture(tex), billboardScale(scale) {}

    void TakeDamage(float damage) {
        health -= damage;
        if (health <= 0) {
            health = 0;
            active = false;
        }
    }

    float GetHealth() const { return health; }

    void SetAttacking() {
        state = ATTACK_RETREAT;
        stateTimer = 0.5f; // Retreat for 0.5 seconds
        velocity = Vector3Scale(Vector3Normalize(velocity), -8.0f); // Kick back
    }

    void Update(float dt, Vector3 playerPos) override {
        if (!active) return;

        // --- AI Logic ---
        Vector3 direction = Vector3Subtract(playerPos, position);
        direction.y = 0; // Only horizontal movement for AI chase
        float dist = Vector3Length(direction);
        if (dist > 0) direction = Vector3Normalize(direction);

        if (state == CHASE) {
            velocity.x = direction.x * speed;
            velocity.z = direction.z * speed;
        } else if (state == ATTACK_RETREAT) {
            stateTimer -= dt;
            if (stateTimer <= 0) {
                state = CHASE;
            }
        }

        // --- Physics ---
        velocity.y += gravity * dt;
        position.x += velocity.x * dt;
        position.z += velocity.z * dt;
        position.y += velocity.y * dt;

        // Friction (simple horizontal dampening)
        velocity.x *= friction;
        velocity.z *= friction;

        // Ground collision
        if (position.y < size.y / 2) {
            position.y = size.y / 2;
            velocity.y = 0;
        }
    }

    void HandleWallCollision(Vector3 wallPos, float cellSize, float wallHeight) {
        BoundingBox myBox = GetBoundingBox();
        BoundingBox wallBox = {
            { wallPos.x - cellSize / 2.0f, 0, wallPos.z - cellSize / 2.0f },
            { wallPos.x + cellSize / 2.0f, wallHeight, wallPos.z + cellSize / 2.0f }
        };

        if (CheckCollisionBoxes(myBox, wallBox)) {
            // Proper AABB resolution: push out by the minimum overlap
            float overlapX = (size.x / 2.0f + cellSize / 2.0f) - fabsf(position.x - wallPos.x);
            float overlapZ = (size.z / 2.0f + cellSize / 2.0f) - fabsf(position.z - wallPos.z);

            if (overlapX < overlapZ) {
                if (position.x > wallPos.x) position.x += overlapX;
                else position.x -= overlapX;
                velocity.x = 0;
            } else {
                if (position.z > wallPos.z) position.z += overlapZ;
                else position.z -= overlapZ;
                velocity.z = 0;
            }
        }
    }

    void HandleEntityCollision(PHPEmployee* other) {
        if (other == this) return;
        float dist = Vector3Distance(position, other->position);
        float minDist = (size.x + other->size.x) * 0.5f;

        if (dist < minDist) {
            Vector3 pushDir = Vector3Subtract(position, other->position);
            pushDir.y = 0;
            if (Vector3Length(pushDir) == 0) pushDir = { (float)GetRandomValue(-10, 10) * 0.1f, 0, (float)GetRandomValue(-10, 10) * 0.1f };
            pushDir = Vector3Normalize(pushDir);
            
            float overlap = minDist - dist;
            position = Vector3Add(position, Vector3Scale(pushDir, overlap * 0.5f));
            other->position = Vector3Subtract(other->position, Vector3Scale(pushDir, overlap * 0.5f));
        }
    }

    Vector3 GetPosition() const { return position; }

    void Draw(Camera camera, bool showHitbox) override {
        if (!active) return;
        DrawBillboard(camera, texture, position, billboardScale, WHITE);

        if (showHitbox) {
            DrawCubeWires(position, size.x, size.y, size.z, RED);
        }
    }

    void Unload() {
        // Texture managed by Core
    }
};

#endif // PHP_EMPLOYEE_H
