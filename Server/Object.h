#pragma once
#include "pch.h"

class Component;
class Session;
class SendBuffer;
class EntityMovement;
class HP;

struct PositionInfo
{
	glm::vec3 m_vPos = glm::zero<glm::vec3>();
	glm::vec3 m_vVelocity = glm::zero<glm::vec3>();
	glm::vec3 m_vAccelation = glm::zero<glm::vec3>();
	glm::vec3 m_cameraAngleAxisSmooth= glm::zero<glm::vec3>();
	float m_rendererBodyAngleY = 0.f;
};

class Object
{
public:
	Object();
	Object(std::shared_ptr<Session> session);
	~Object()noexcept;
public:
	const auto& GetSession()const noexcept { return m_session; }
	const auto GetObjectID()const noexcept { return (uint32_t)m_obj_id; }
	const auto GetObjectType()const noexcept { return m_eObjType; }
	
	void SetPos(const glm::vec3& pos)noexcept { m_posInfo.m_vPos = pos; }
	const auto& GetPos()const noexcept { return m_posInfo.m_vPos; }

	void SetVelocity(const glm::vec3& vel_)noexcept { m_posInfo.m_vVelocity = vel_; }
	const auto& GetVelocity()const noexcept { return m_posInfo.m_vVelocity; }

	void SetAccel(const glm::vec3& accel_)noexcept { m_posInfo.m_vAccelation = accel_; }
	const auto& GetAccel()const noexcept { return m_posInfo.m_vAccelation; }

	void SetInvalid()noexcept { m_bIsValid = false; }
	const bool IsValid()const noexcept { return m_bIsValid; }
	template<typename T>
	T* GetComp()const noexcept {
		auto b = m_vecComp.data();
		const auto e = b + m_vecComp.size();
		while (e != b) {
			const auto comp = (*b++).get();
			if (const auto target = dynamic_cast<T*>(comp))
				return target;
		}
		return nullptr;
	}
	
	auto& GetPosInfo()noexcept { return m_posInfo; }
public:
	void Init()noexcept;
	void Update(const float DT)noexcept;
	void SetDirtyFlag()noexcept { m_bDirtyFlag = true; }
	template<typename T = Component>
	const auto AddComp(Component* const pComp)noexcept { return (T*)m_vecComp.emplace_back(pComp).get(); }
	void SetObjectType(const MC_OBJECT_TYPE eType) { m_eObjType = eType; }
public:
	void SetHP(const int hp_)noexcept;
	const int GetHP()const noexcept;
	void IncHP(const int inc_)noexcept;
	void DecHP(const int dec_)noexcept;
public:
	template<typename T>
	void InitHP(const int hp_)noexcept { m_HP = std::make_unique<T>(); SetUpHP(hp_); }
private:
	void SendRemovePacket()const noexcept;
	void SetUpHP(const int hp_)noexcept;
public:
	float m_accAtkTime = 0.f;
	bool flag = true;
private:
	bool m_bIsValid = true;
	bool m_bDirtyFlag = false;
	MC_OBJECT_TYPE m_eObjType;
	const uint32_t m_obj_id;
	const std::shared_ptr<Session> m_session;
	std::unique_ptr<HP> m_HP;
	std::vector<std::unique_ptr<Component>> m_vecComp;
	PositionInfo m_posInfo;
};

