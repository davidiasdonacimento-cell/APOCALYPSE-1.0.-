#pragma once
#include <vector>

namespace apocalypse {
struct Zombie {
    float x=0.0f, z=0.0f;
    int health=3;
    int state=0; // 0 patrol, 1 chase, 2 defeated
    float attackCooldown=0.0f;
    float patrolPhase=0.0f;
};
struct Pickup {
    float x=0.0f, z=0.0f;
    int type=0; // 0 water, 1 food, 2 materials, 3 medkit
    bool active=true;
};
class SurvivalSystem {
public:
    void reset();
    void update(float dt, float playerX, float playerZ);
    bool usePulse(float playerX, float playerZ);
    int health() const { return playerHealth; }
    int water() const { return waterCount; }
    int food() const { return foodCount; }
    int materials() const { return materialCount; }
    int medkits() const { return medkitCount; }
    int aliveZombies() const;
    const std::vector<Zombie>& zombies() const { return zombieList; }
    const std::vector<Pickup>& pickups() const { return pickupList; }
private:
    int playerHealth=100;
    int waterCount=0, foodCount=0, materialCount=0, medkitCount=0;
    float pulseCooldown=0.0f;
    std::vector<Zombie> zombieList;
    std::vector<Pickup> pickupList;
};
}
