#include "survival_system.h"
#include <cmath>
#include <algorithm>

namespace apocalypse {
static float distance2(float ax,float az,float bx,float bz) {
    const float dx=ax-bx, dz=az-bz;
    return dx*dx+dz*dz;
}
void SurvivalSystem::reset() {
    playerHealth=100; waterCount=0; foodCount=0; materialCount=0; medkitCount=0; pulseCooldown=0.0f;
    zombieList.clear();
    zombieList.push_back({-8.0f, 3.0f, 3, 0, 0.0f});
    zombieList.push_back({7.0f, -6.0f, 3, 0, 0.0f});
    zombieList.push_back({14.0f, 10.0f, 3, 0, 0.0f});
    zombieList.push_back({-15.0f, -12.0f, 3, 0, 0.0f});
    pickupList.clear();
    pickupList.push_back({3.0f, 2.0f, 0, true});
    pickupList.push_back({-5.0f, 8.0f, 1, true});
    pickupList.push_back({10.0f, 3.0f, 2, true});
    pickupList.push_back({-12.0f, 4.0f, 3, true});
    pickupList.push_back({17.0f, -14.0f, 0, true});
}
void SurvivalSystem::update(float dt,float px,float pz) {
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
            playerHealth=std::max(0,playerHealth-6);
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
}
bool SurvivalSystem::usePulse(float px,float pz) {
    if(pulseCooldown>0.0f || playerHealth<=0) return false;
    pulseCooldown=0.55f;
    int target=-1; float best=2.3f*2.3f;
    for(size_t i=0;i<zombieList.size();++i) {
        const auto& z=zombieList[i];
        if(z.state==2) continue;
        const float d=distance2(z.x,z.z,px,pz);
        if(d<best) { best=d; target=(int)i; }
    }
    if(target<0) return false;
    auto& z=zombieList[(size_t)target];
    --z.health;
    if(z.health<=0) z.state=2;
    return true;
}
int SurvivalSystem::aliveZombies() const {
    int count=0;
    for(const auto& z:zombieList) if(z.state!=2) ++count;
    return count;
}
}
