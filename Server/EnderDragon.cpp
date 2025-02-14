#include "pch.h"
#include "EnderDragon.h"
#include "MCTilemap.h"
#include "Object.h"
#include <random>
#include <algorithm>
#include "MCWorld.h"
#include "PacketBase.hpp"
#include "MCObjectFactory.h"
#include "IOExecutor.h"

static std::default_random_engine dre{ };
static std::uniform_int_distribution uid{ 0,19 };

void EnderDragon::SetBezierPoints() noexcept
{
    m_points[0] = GetOwner()->GetPos();
    const auto num = uid(dre);

    const auto b = m_randomPoints.data();
    m_points[1] = *(b + num);
    m_points[2] = *(b + (19 - num));
    const auto& players = Mgr(MCWorld)->GetWorldObjects(MC_OBJECT_TYPE::PLAYER);
    if (const auto num = (int)players.size())
    {
        const auto y = m_points[2].y;
        glm::vec3 temp{};
        auto b = players.data();
        const auto e = b + num;
        while (e != b) { temp += (*b++)->GetPos(); }
        temp /= num;
        m_points[2] = temp;
        m_points[2].y = y;
    }
    GenerateArcLengthTable(m_points[0], m_points[1], m_points[2]);
    m_curveLength = m_arcLengthTable.back().second;
}

void EnderDragon::Init()
{
    constexpr const float BASE_X = MCTilemap::MAP_WIDTH / 2;
    constexpr const float BASE_Y = MCTilemap::MAP_HEIGHT - 10;
    constexpr const float BASE_Z = MCTilemap::MAP_WIDTH / 2;

    for (int i = 0; i < 20; ++i)
    {
        const float x = BASE_X + static_cast<float>(std::rand()) / RAND_MAX * 120.0f - 60.0f;
        const float y = BASE_Y + static_cast<float>(std::rand()) / RAND_MAX * 10.0f - 5.0f;
        const float z = BASE_Z + static_cast<float>(std::rand()) / RAND_MAX * 120.0f - 60.0f;
        m_randomPoints.emplace_back(x, y, z);
    }

    SetBezierPoints(); 
    m_traveledDistance = 0.0f;
    m_bezierT = 0.0f;
}

void EnderDragon::Update(const float DT)
{
    const auto owner = GetOwner();
    auto& pos_info = owner->GetPosInfo();
    m_oldPos = pos_info.m_vPos;

    m_accTime += DT;
    if (3.f <= m_accTime)
    {
        m_accTime = 0.f;
        const auto& players = Mgr(MCWorld)->GetWorldObjects(MC_OBJECT_TYPE::PLAYER);
        const auto mc_world = Mgr(MCWorld);
        s2c_BOSS_PROJECTILE pkt;
        pkt.pos_x = m_oldPos.x;
        pkt.pos_y = m_oldPos.y;
        pkt.pos_z = m_oldPos.z;

        std::vector<S_ptr<Object>> fire_ball;
        fire_ball.reserve(players.size());

        for (const auto& player : players)
        {
            const auto v = glm::normalize(player->GetPos() - m_oldPos) * 32.f;

            ProjFireBallBuilder b;
            b.pos = m_oldPos;
            b.vel = v;

            auto fb = MCObjectFactory::CreateProjFireBall(b);

            pkt.projectile_id = fb->GetObjectID();
            pkt.vel_x = v.x;
            pkt.vel_y = v.y;
            pkt.vel_z = v.z;
            mc_world->AppendToWorldSendBuffer(pkt);
            fire_ball.emplace_back(std::move(fb));
        }

        Mgr(MCWorld)->PostWorldEvent([fire_ball = std::move(fire_ball)]()mutable noexcept
            {
                for (auto& fb : fire_ball)
                    Mgr(MCWorld)->AddObject(std::move(fb), MC_OBJECT_TYPE::BOSS_PROJ);
            });
    }
    const float distanceToTravel = m_speed * DT;
    m_traveledDistance = glm::clamp(m_traveledDistance + distanceToTravel, 0.f, m_curveLength);

    m_bezierT = MapDistanceToT(m_traveledDistance);

   
    pos_info.m_vPos = (bezier(m_points[0], m_points[1], m_points[2], m_bezierT));

    pos_info.m_vVelocity = glm::normalize(pos_info.m_vPos - m_oldPos) * m_speed;

    owner->SetDirtyFlag();

    if (m_traveledDistance >= m_curveLength)
    {
        m_traveledDistance = 0.0f;
        m_bezierT = 0.0f;
        SetBezierPoints();
    }
}