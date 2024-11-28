#include "pch.h"
#include "MCObjectFactory.h"
#include "MCWorld.h"
#include "Object.h"
#include "PathFollower.h"
#include "FSM.h"
#include "State.h"
#include "EnderDragon.h"
#include "MCTilemap.h"
#include "ProjArrow.h"

S_ptr<Object> MCObjectFactory::CreateMonster(MCObjectBuilder& b_)
{
	auto mon = std::make_shared<Object>();
	mon->SetObjectType(MC_OBJECT_TYPE::MONSTER);
	
	const auto fsm = mon->AddComp<FSM>(new FSM);
	fsm->AddState(new Patrol);
	fsm->AddState(new Chase);
	fsm->Start(MON_STATE::PATROL);

	const auto m = mon->AddComp<PathFollower>(new PathFollower);
	fsm->SetPathFollower(m);
	mon->SetPos(b_.pos);
	
	
	return mon;
}

S_ptr<Object> MCObjectFactory::CreateItem(MCItemBuilder& b_)
{
	auto item = std::make_shared<Object>();
	item->SetObjectType(MC_OBJECT_TYPE::ITEM);
	//const auto m = item->AddComp<EntityMovement>(new EntityMovement);
	//item->SetEntityMovement(m);
	item->SetPos(b_.pos);
	//item->GetEntityMovement()->current_position = b_.pos;
	// TODO: 어떤아이템인가의 정보 추가하기
	return item;
}

S_ptr<Object> MCObjectFactory::CreateEnderDragon(EnderDragonBuilder& b_)
{
	auto ed = std::make_shared<Object>();
	ed->SetPos(b_.pos);
	const auto edc = ed->AddComp<EnderDragon>(new EnderDragon);

	return ed;
}

S_ptr<Object> MCObjectFactory::CreateProjArrow(ProjArrowBuilder& b_)
{
	auto a = std::make_shared<Object>();
	a->SetObjectType(MC_OBJECT_TYPE::ARROW);
	const auto proj = a->AddComp<ProjArrow>(new ProjArrow);
	const auto r = glm::rotate(glm::quat(glm::vec3(glm::radians(b_.rot_x), glm::radians(b_.rot_y), 0.0f)), glm::vec3(0.0f, 0.0f, 1.0f));
	const glm::vec3 p = b_.pos + glm::vec3(0.0f, 1.7f, 0.0f) + glm::normalize(r) * 0.75f;

	a->SetPos(p);
	a->SetVelocity(r * 32.0f);

	return a;
}
