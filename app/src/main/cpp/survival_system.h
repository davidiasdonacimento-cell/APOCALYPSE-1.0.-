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
struct Projectile {
    float x=0.0f, z=0.0f;
    float dirX=0.0f, dirZ=-1.0f;
    float travelled=0.0f, maxRange=0.0f, speed=0.0f;
    int damage=1, weapon=0;
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
    bool usePulse(float playerX, float playerZ);\n    bool fireWeapon(float playerX, float playerZ, float dirX, float dirZ);
    bool purchase(int item);
    bool selectWeapon(int weapon);
    int money() const { return moneyCount; }
    int wave() const { return waveNumber; }
    int weapon() const { return activeWeapon; }
    int weaponAmmo() const;
    int health() const { return playerHealth; }
    int water() const { return waterCount; }
    int food() const { return foodCount; }
    int materials() const { return materialCount; }
    int medkits() const { return medkitCount; }
    int aliveZombies() const;
    const std::vector<Zombie>& zombies() const { return zombieList; }
    const std::vector<Pickup>& pickups() const { return pickupList; }\n    const std::vector<Projectile>& projectiles() const { return projectileList; }
private:
    int playerHealth=100;
    int moneyCount=0;
    int pulseDamage=1;
    int armorLevel=0;
    int waveNumber=1;
    int activeWeapon=0; // 0 pulse, 1 shuriken, 2 rifle, 3 shotgun
    bool weaponOwned[4]={true,false,false,false};
    int shurikenAmmo=0, rifleAmmo=0, shotgunAmmo=0;
    void spawnWave();
    float lastPlayerX=0.0f, lastPlayerZ=0.0f;
    int waterCount=0, foodCount=0, materialCount=0, medkitCount=0;
    float pulseCooldown=0.0f;
    std::vector<Zombie> zombieList;
    std::vector<Pickup> pickupList;\n    std::vector<Projectile> projectileList;
};
}
