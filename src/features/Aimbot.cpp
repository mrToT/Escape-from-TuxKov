#include "Aimbot.h"
#include "Glow.h"
#include "../utils/Logger.h"

#include "../utils/Math.h"
#include "../utils/Wrappers.h"

static void RecoilCompensation(QAngle &angle) {
    angle -= localPlayer.aimPunch;
}

static void SwayCompensation(QAngle &viewAngle, QAngle &angle) {
    QAngle dynamic = localPlayer.viewAngles;
    QAngle sway = dynamic - viewAngle;

    angle -= sway;
}

static void NoSpread(uintptr_t weapon) {
    process->Write<float>(weapon + 0x1330, 0.0f);
    process->Write<float>(weapon + 0x1340, 0.0f);
}


void Aimbot::Aimbot() {
    if (!localPlayer)
        return;


    int state = inputSystem->Read<int>(inputBase + 0x3388);
    if (!state) {
        return;
    }
    sendpacket = true; // want to send packets when aiming
    QAngle viewAngle = localPlayer.viewAngles;

    CBaseEntity* finalEntity = nullptr;

    Vector localOrigin = localPlayer.origin;
    /*Vector localOrigin = process->Read<Vector>(localPlayer + 0x50);
    Vector abslocalOrigin = process->Read<Vector>(localPlayer + 0x4);
    Vector viewOffset = process->Read<Vector>(localPlayer + 0x30);
    Vector localAngles = process->Read<Vector>(localPlayer + 0x414);
    Vector pusherOrigin = process->Read<Vector>(localPlayer + 0x24);*/
    Vector eyepos = localPlayer.eyePos;
    eyepos->x = localOrigin->x;
    eyepos->y = localOrigin->y;

    /*Logger::Log("Origin: (%f, %f, %f)\n", localOrigin.x, localOrigin.y, localOrigin.z);
    Logger::Log("absOrigin: (%f, %f, %f)\n", abslocalOrigin.x, abslocalOrigin.y, abslocalOrigin.z);
    Logger::Log("Offset: (%f, %f, %f)\n", viewOffset.x, viewOffset.y, viewOffset.z);
    Logger::Log("localAngles: (%f, %f, %f)\n", localAngles.x, localAngles.y, localAngles.z);
    Logger::Log("pusher: (%f, %f, %f)\n", pusherOrigin.x, pusherOrigin.y, pusherOrigin.z);
    Logger::Log("eyepos: (%f, %f, %f)\n", eyepos.x, eyepos.y, eyepos.z);*/
    //Vector pos = process->Read<Vector>(localPlayer + 0x3AA0);
    //Vector pos = GetBonePos(localPlayer, 12, localOrigin);
    //Vector pos = process->Read<Vector>(localPlayer + 0x3AA0);
    Vector pos = eyepos;

    int localTeam = localPlayer.teamNum;


    std::sort(sortedEntities.begin(), sortedEntities.end(), [viewAngle, &pos](const auto &a, const auto &b) {
        Vector a_pos = GetBonePos(entities[a], 12, entities[a].origin);
        Vector b_pos = GetBonePos(entities[b], 12, entities[b].origin);
        return Math::DistanceFOV(viewAngle, Math::CalcAngle(pos, a_pos), pos.DistTo(a_pos)) < Math::DistanceFOV(viewAngle, Math::CalcAngle(pos, b_pos), pos.DistTo(b_pos));
    });

    for (size_t ent = 0; ent < sortedEntities.size(); ent++) {
        CBaseEntity& entity = entities[sortedEntities[ent]];
        if (!entity || entity == localPlayer) {
            continue;
        }

        if (entity.teamNum == localTeam) {
            continue;
        }

        int lifeState = entity.lifeState;

        if (lifeState != 0)
            continue;
        finalEntity = &entity;
        break;
    }
    if (!finalEntity)
        return;

    Vector enemyHeadPosition = GetBonePos(*finalEntity, 12, finalEntity->origin);

    float dist = pos.DistTo(enemyHeadPosition);
    dist *= 0.01905f;

    uintptr_t weapon = GetActiveWeapon(localPlayer);

    if (!weapon) {
        return;
    }

    NoSpread(weapon);

    float bulletVel = process->Read<float>(weapon + 0x1bac);
    if (bulletVel == 0.0f)
        return;

    bulletVel *= 0.01905f;

    Vector enemyVelocity = finalEntity->velocity;
    Vector targetVelocity = enemyVelocity;
    //targetVelocity *= 0.01905f;

    //float interval_per_tick = process->Read<float>(0x1713CA8 + 0x44);



    float xTime = dist / bulletVel;
    float yTime = xTime;

    enemyHeadPosition->x += xTime * targetVelocity->x;
    enemyHeadPosition->y += yTime * targetVelocity->y;
    enemyHeadPosition->z += yTime * targetVelocity->z + 375.0f * powf(xTime, 2.0f);

    QAngle aimAngle = Math::CalcAngle(pos, enemyHeadPosition);

    aimAngle.Normalize();
    Math::Clamp(aimAngle);

    //int lifeState = process->Read<int>(finalEntity + 0x718);

    if ((aimAngle->x == 0 && aimAngle->y == 0 && aimAngle->z == 0) || !aimAngle.IsValid()) {
        return;
    }


    SwayCompensation(viewAngle, aimAngle);
    RecoilCompensation(aimAngle);
    aimAngle.Normalize();
    Math::Clamp(aimAngle);

    localPlayer.swayAngles = aimAngle;
    //process->Write(localPlayer + 0x20A8, aimAngle);
    //process->Write(localPlayer + 0x20BC, aimAngle.y);

    //Unused at the moment
    //static float col[3] = {0.0f, 0.0f, 255.0f};
    //Glow::GlowPlayer(finalEntity, col);
}
