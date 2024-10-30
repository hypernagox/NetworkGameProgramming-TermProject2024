#include "pch.h"
#include "Particle.h"
#include "PannelUI.h"
#include "Transform.h"
#include "RigidBody.h"
#include "InstancingMgr.h"
#include "SceneMgr.h"
#include "Scene.h"
#include "EventMgr.h"
#include "MeshRenderer.h"
#include "Shader.h"

std::unordered_map<string, shared_ptr<MeshRenderer>> Particle::g_mapMeshRenderer;

Particle::Particle()noexcept
{
	AddComponent<RigidBody>();
}

Particle::~Particle()
{
}

void Particle::Update()
{
	GameObj::Update();
	m_fLife -= DT;
	if (0.f >= m_fLife)
	{
		DestroyObj(this->shared_from_this());
		m_bIsActivate = false;
	}
}

void Particle::ActivateParticle(shared_ptr<MeshRenderer> pMeshRenderer,string_view strResName_, const glm::vec3& worldPos_, const glm::vec3& scale_)noexcept
{
	auto shared_this = this->shared_from_this();
	auto iter = g_mapMeshRenderer.find(strResName_.data());
	if (g_mapMeshRenderer.end() == iter)
	{
		iter = g_mapMeshRenderer.emplace_hint(iter, strResName_, make_shared<MeshRenderer>(*pMeshRenderer));
	}
	AddComponent(iter->second);
	m_bIsActivate = true;
	const auto pTrans = GetTransform();
	SetResName(strResName_);
	pTrans->SetLocalPosition(worldPos_);
	pTrans->SetLocalScale(scale_);
	const glm::vec3 yVel = glm::sphericalRand(g_particleSpeed) * 0.1f;
	m_fLife = 2.f + yVel.y * 0.1f;
	GetComp<RigidBody>()->SetVelocity(yVel);
	Mgr(InstancingMgr)->AddInstancingList(shared_this);
	Mgr(SceneMgr)->GetCurScene()->AddObject(std::move(shared_this), GROUP_TYPE::PARTICLE);
}