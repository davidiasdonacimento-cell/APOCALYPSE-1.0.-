#include "survival_system.h"
#include <cmath>
#include <algorithm>

namespace apocalypse {
static void updateProjectiles(std::vector<Projectile>& shots,std::vector<Zombie>& zombies,
                              std::vector<Impact>& impacts,float dt,int& money);
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
    updateProjectiles(projectileList,zombieList,impactList,dt,moneyCount);
    for(auto it=impactList.begin();it!=impactList.end();){it->life-=dt;if(it->life<=0.0f)it=impactList.erase(it);else ++it;}
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
    return fireWeapon(px,pz,0.0f,-1.0f);
}
bool SurvivalSystem::fireWeapon(float px,float pz,float dirX,float dirZ) {
    if(pulseCooldown>0.0f || playerHealth<=0) return false;
    const float len=std::sqrt(dirX*dirX+dirZ*dirZ);
    if(len<0.001f) return false;
    dirX/=len; dirZ/=len;
    int* ammo=nullptr;
    float range=2.3f, speed=0.0f, cooldown=0.55f;
    int damage=pulseDamage;
    if(activeWeapon==1) { range=10.0f; speed=14.0f; cooldown=0.72f; damage=2; ammo=&shurikenAmmo; }
    else if(activeWeapon==2) { range=18.0f; speed=42.0f; cooldown=0.22f; damage=2; ammo=&rifleAmmo; }
    else if(activeWeapon==3) { range=5.5f; speed=24.0f; cooldown=0.85f; damage=1; ammo=&shotgunAmmo; }
    if(ammo && *ammo<=0) return false;
    pulseCooldown=cooldown;
    if(ammo) --(*ammo);
    if(activeWeapon==0) {
        int target=-1; float best=range*range;
        for(size_t i=0;i<zombieList.size();++i) {
            const auto& z=zombieList[i]; if(z.state==2) continue;
            const float rx=z.x-px, rz=z.z-pz;
            const float along=rx*dirX+rz*dirZ;
            const float side=std::fabs(rx*dirZ-rz*dirX);
            if(along>=0.0f && along<=range && side<0.75f && along*along<best) {
                best=along*along; target=(int)i;
            }
        }
        if(target>=0) {
            auto& z=zombieList[(size_t)target]; z.health-=damage;
            if(z.health<=0) { z.state=2; moneyCount+=30; }
        }
        return true;
    }
    const int pelletCount=(activeWeapon==3)?5:1;
    for(int i=0;i<pelletCount;i++) {
        const float spread=pelletCount>1?((float)i-2.0f)*0.055f:0.0f;
        const float cs=std::cos(spread), sn=std::sin(spread);
        const float dx=dirX*cs-dirZ*sn, dz=dirX*sn+dirZ*cs;
        projectileList.push_back({px,pz,dx,dz,0.0f,range,speed,damage,activeWeapon});
    }
    return true;
}
static void updateProjectiles(std::vector<Projectile>& shots,std::vector<Zombie>& zombies,
                              std::vector<Impact>& impacts,float dt,int& money) {
    for(auto it=shots.begin();it!=shots.end();) {
        const float step=std::min(it->speed*dt,it->maxRange-it->travelled);
        const float oldX=it->x, oldZ=it->z;
        it->x+=it->dirX*step; it->z+=it->dirZ*step; it->travelled+=step;
        bool hit=false;
        for(auto& z:zombies) {
            if(z.state==2) continue;
            const float sx=it->x-oldX, sz=it->z-oldZ, seg2=sx*sx+sz*sz;
            const float t=seg2>0.000001f?std::max(0.0f,std::min(1.0f,((z.x-oldX)*sx+(z.z-oldZ)*sz)/seg2)):0.0f;
            const float dx=z.x-(oldX+sx*t), dz=z.z-(oldZ+sz*t);
            if(dx*dx+dz*dz<=0.62f*0.62f) {
                z.health-=it->damage;
                if(z.health<=0) { z.state=2; money+=30; }
                impacts.push_back({z.x,z.z,0.22f,it->weapon});
                hit=true; break;
            }
        }
        if(hit || it->travelled>=it->maxRange) it=shots.erase(it); else ++it;
    }
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
