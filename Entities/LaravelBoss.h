#ifndef LARAVEL_BOSS_H
#define LARAVEL_BOSS_H

#include "Entity.h"
#include "raymath.h"
#include <vector>

enum LARAVELState { UNACTIVE, CHASE, ATTACK_RETREAT, ATTACK_WAIT, HEAL, SUMMON };

class LARAVELBoss : public Entity {
private:
    Texture2D texture;
    float billboardScale;
    LARAVELState state = UNACTIVE;
    float stateTimer = 0.0f;
    float speed = 8.0f;
    float gravity = -22.0f;
    float friction = 0.9f;
    float health = 1000.0f;
public:
    LARAVELBoss(Vector3 pos, Texture2D tex, float scale = 2.0f)
        : Entity(pos, { scale * 2.5f, scale, scale * 2.5f }), texture(tex), billboardScale(scale) {}

    void TakeDamage(float damage) {
        health -= damage;
        if (health <= 0) { 
            health = 0;
            active = false;
        }
    } 

    float GetHealth() const { return health; }

    void StartBossFight() {
        state = CHASE;
        velocity = Vector3Scale(Vector3Normalize(velocity), 8.0f); // Dash forward.
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
}