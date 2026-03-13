#ifndef ENTITY_H
#define ENTITY_H

#include "raylib.h"

class Entity {
public:
    Vector3 position;
    Vector3 velocity;
    Vector3 size;
    bool active;

    Entity(Vector3 pos = { 0, 0, 0 }, Vector3 sz = { 1, 1, 1 }) 
        : position(pos), velocity({0, 0, 0}), size(sz), active(true) {}

    virtual ~Entity() {}

    virtual void Update(float dt, Vector3 playerPos) = 0;
    virtual void Draw(Camera camera, bool showHitbox) = 0;

    virtual BoundingBox GetBoundingBox() {
        return {
            { position.x - size.x / 2, position.y - size.y / 2, position.z - size.z / 2 },
            { position.x + size.x / 2, position.y + size.y / 2, position.z + size.z / 2 }
        };
    }
};

#endif // ENTITY_H
