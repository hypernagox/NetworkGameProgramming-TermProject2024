#pragma once
#include "Component.h"

class ProjArrow
	:public Component
{
	static constexpr float G_ARROW_REMAIN_TIME = 3.f;
public:
	void Update(const float DT)override;
private:
	float m_accArrowTime = 0.f;
	bool m_isPinned = false;
};

