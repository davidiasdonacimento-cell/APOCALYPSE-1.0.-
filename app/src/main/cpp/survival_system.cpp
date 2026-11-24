#include "survival_system.h"
#include <cmath>
#include <algorithm>

namespace apocalypse {
static float distance2(float ax,float az,float bx,float bz) {
    const float dx=ax-bx, dz=az-bz;
    return dx*dx+dz*dz;
}
void SurvivalSystem::reset() {
    playerHealth=100; moneyCount=0; pulseDamage=1; armorLevel=0;
    waveNumber=1; activeWeapon=0;
    weaponOwned[0]=true; weaponOwned[1]=false; weaponOwned[2]=false; weaponOwned[3]=false;
    shurikenAmmo=0; rifleAmmo=0; shotgunAmmo=0;
    waterCount=0; foodCount=0; materialCount=0; medkitCount=0; pulseCooldown=0.0f;
    zombieList.clear();
    spawnWave();
    pickupList.clear();
    pickupList.push_back({3.0f, 2.0f, 0, true});
    pickupList.push_back({-5.0f, 8.0f, 1, true});
    pickupList.push_back({10.0f, 3.0f, 2, true});
    pickupList.push_back({-12.0f, 4.0f, 3, true});
    pickupList.push_back({17.0f, -14.0f, 0, true});
}
void SurvivalSystem::update(float dt,float px,float pz) {
    lastPlayerX=px; lastPlayerZ=pz;
    dt=std::max(0.0f,std::min(dt,0.05f));
    pulseCooldown=std::max(0.0f,pulseCooldown-dt);
    for(auto& z:zombieList) {
        if(z.state==2) continue;
        z.attackCooldown=std::max(0.0f,z.attackCooldown-dt);
        const float d2=distance2(z.x,z.z,px,pz);
        if(d2<100.0f) z.state=1; else z.state=0;
        z.patrolPhase+=dt*(z.state==1?2.8f:0.7f);
        if(z.state==0) {
            z.x+=std::sin(z.patrolPhase*0.7f)*0.22f*dt;
            z.z+=std::cos(z.patrolPhase*0.7f)*0.22f*dt;
        }
        if(z.state==1 && d2>1.35f*1.35f) {
            const float d=std::sqrt(std::max(d2,0.0001f));
            const float speed=0.85f;
            z.x+=(px-z.x)/d*speed*dt;
            z.z+=(pz-z.z)/d*speed*dt;
        } else if(z.state==1 && z.attackCooldown<=0.0f && playerHealth>0) {
            const int incomingDamage=std::max(1,6-armorLevel*2);
            playerHealth=std::max(0,playerHealth-incomingDamage);
            z.attackCooldown=1.25f;
        }
    }
    for(auto& p:pickupList) {
        if(!p.active || distance2(p.x,p.z,px,pz)>1.15f*1.15f) continue;
        p.active=false;
        switch(p.type) {
            case 0: ++waterCount; break;
            case 1: ++foodCount; break;
            case 2: ++materialCount; break;
            case 3: ++medkitCount; playerHealth=std::min(100,playerHealth+25); break;
        }
    }
    if(!zombieList.empty() && aliveZombies()==0) {
        ++waveNumber;
        moneyCount += 100 + waveNumber*25;
        ++medkitCount;
        shurikenAmmo += 5;
        spawnWave();
    }
}
void SurvivalSystem::spawnWave() {
    zombieList.clear();
    const int count=std::min(12,3+waveNumber);
    for(int i=0;i<count;i++) {
        const float angle=(float)i*6.2831853f/(float)count + (float)waveNumber*0.37f;
        const float radius=10.0f+(float)((i*7+waveNumber*3)%7);
        const float x=lastPlayerX+std::cos(angle)*radius;
        const float z=lastPlayerZ+std::sin(angle)*radius;
        zombieList.push_back({x,z,3+(waveNumber-1)/3,0,0.0f,0.0f});
    }
}
bool SurvivalSystem::usePulse(float px,float pz) {
    if(pulseCooldown>0.0f || playerHealth<=0) return false;
    int damage=pulseDamage;
    float range=2.3f;
    float cooldown=0.55f;
    int* ammo=nullptr;
    if(activeWeapon==1) { damage=2; range=10.0f; cooldown=0.72f; ammo=&shurikenAmmo; }
    else if(activeWeapon==2) { damage=2; range=18.0f; cooldown=0.22f; ammo=&rifleAmmo; }
    else if(activeWeapon==3) { damage=4; range=5.5f; cooldown=0.85f; ammo=&shotgunAmmo; }
    if(ammo && *ammo<=0) return false;
    pulseCooldown=cooldown;
    if(ammo) --(*ammo);
    int target=-1; float best=range*range;
    for(size_t i=0;i<zombieList.size();++i) {
        const auto& z=zombieList[i];
        if(z.state==2) continue;
        const float d=distance2(z.x,z.z,px,pz);
        if(d<best) { best=d; target=(int)i; }
    }
    if(target<0) return true;
    auto& z=zombieList[(size_t)target];
    z.health-=damage;
    if(z.health<=0) { z.state=2; moneyCount+=30; }
    return true;
}
bool SurvivalSystem::purchase(int item) {
    if(item==0) {
        if(moneyCount<50 || playerHealth>=100) return false;
        moneyCount-=50; playerHealth=std::min(100,playerHealth+40); return true;
    }
    if(item==1) {
        if(moneyCount<120 || pulseDamage>=3) return false;
        moneyCount-=120; ++pulseDamage; return true;
    }
    if(item==2) {
        if(moneyCount<90 || armorLevel>=3) return false;
        moneyCount-=90; ++armorLevel; return true;
    }
    if(item==3) {
        if(moneyCount<100 || weaponOwned[1]) return false;
        moneyCount-=100; weaponOwned[1]=true; shurikenAmmo+=12; activeWeapon=1; return true;
    }
    if(item==4) {
        if(moneyCount<260 || weaponOwned[2]) return false;
        moneyCount-=260; weaponOwned[2]=true; rifleAmmo+=30; activeWeapon=2; return true;
    }
    if(item==5) {
        if(moneyCount<220 || weaponOwned[3]) return false;
        moneyCount-=220; weaponOwned[3]=true; shotgunAmmo+=12; activeWeapon=3; return true;
    }
    if(item==6) {
        if(moneyCount<45 || !weaponOwned[1]) return false;
        moneyCount-=45; shurikenAmmo+=12; return true;
    }
    if(item==7) {
        if(moneyCount<80 || !weaponOwned[2]) return false;
        moneyCount-=80; rifleAmmo+=30; return true;
    }
    if(item==8) {
        if(moneyCount<70 || !weaponOwned[3]) return false;
        moneyCount-=70; shotgunAmmo+=12; return true;
    }
    return false;
}
bool SurvivalSystem::selectWeapon(int weapon) {
    if(weapon<0 || weapon>3 || !weaponOwned[weapon]) return false;
    if(weapon==1 && shurikenAmmo<=0) return false;
    if(weapon==2 && rifleAmmo<=0) return false;
    if(weapon==3 && shotgunAmmo<=0) return false;
    activeWeapon=weapon; return true;
}
int SurvivalSystem::weaponAmmo() const {
    if(activeWeapon==1) return shurikenAmmo;
    if(activeWeapon==2) return rifleAmmo;
    if(activeWeapon==3) return shotgunAmmo;
    return -1;
}
int SurvivalSystem::aliveZombies() const {
    int count=0;
    for(const auto& z:zombieList) if(z.state!=2) ++count;
    return count;
}
}
