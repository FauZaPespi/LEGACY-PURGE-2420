#ifndef BULLET_H
#define BULLET_H

#include "raylib.h"
#include "raymath.h"

class Bullet {
public:
    Vector3 position;
    Vector3 velocity;
    float lifetime;
    bool active;

    Bullet(Vector3 pos, Vector3 vel, float maxLifetime = 3.0f)
        : position(pos), velocity(vel), lifetime(maxLifetime), active(true) {}

    void Update(float dt) {
        if (!active) return;

        position = Vector3Add(position, Vector3Scale(velocity, dt));
        lifetime -= dt;

        if (lifetime <= 0) {
            active = false;
        }
    }

    BoundingBox GetBoundingBox() {
        float size = 0.2f;
        return {
            { position.x - size / 2, position.y - size / 2, position.z - size / 2 },
            { position.x + size / 2, position.y + size / 2, position.z + size / 2 }
        };
    }

    void Deactivate() {
        active = false;
    }
};

#endif // BULLET_H