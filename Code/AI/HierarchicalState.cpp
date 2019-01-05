#include "StdAfx.h"
#include "HierarchicalState.h"

void CAI_HierarchicalState::Update(float fDeltaTime)
{
	if (CheckTransitions())
		return;
	OnUpdate(fDeltaTime);
	if (m_ActiveSubState >= 0)
		m_SubStates[m_ActiveSubState]->Update(fDeltaTime);
}

void CAI_HierarchicalState::TransitionTo(const int StateId)
{
	for (int i = 0; i < m_SubStates.size(); i++)
	{
		if (m_SubStates[i]->GetStateId() == StateId)
		{
			m_ActiveSubState = i;
			m_SubStates[i]->OnEnter();
			return;
		}
	}
	OnExit();
	if (m_pParentState)
		m_pParentState->TransitionTo(StateId);
	else
		bool YOUSUCK = true;
}