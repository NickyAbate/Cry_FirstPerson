#pragma once

#include <DefaultComponents\Input\InputComponent.h>
#include "Types\Flags.h"

#define TAGGAMEFLOW(NAME, Id)											\
namespace Gameflow														\
{																		\
	namespace Gameflows													\
	{																	\
		enum															\
		{																\
			NAME = Id,													\
		};																\
	}																	\
}
TAGGAMEFLOW(NONE, 0)

#define GAMEFLOW_STANDARD_INIT()							\
m_GameflowId = GAMEFLOWID;									\
m_GameflowName = GAMEFLOWNAME;								\
InitializeKeyBinds();

class CGameflowPhase;
class CGameflowManager;

class CGameflow
{
	friend class CGameflowManager;
public:

	CGameflow() {}
	virtual ~CGameflow() {}

#define GAMEFLOW_BIT(x) BIT64(x)
	using _FlagNumeric_ = int;
	using FlagType = CFlags<_FlagNumeric_>;
	enum EGameflowFlags
	{
		NONE = 0,
		UPDATE_IN_BACKGROUND = GAMEFLOW_BIT(1),
	};

	// Must include GAMEFLOW_STANDARD_INIT()
	virtual void Initialize() = 0;
	virtual void InitializeKeyBinds() = 0;
	// Must include UpdatePhases(fDeltaTime)
	virtual void Update(float fDeltaTime) = 0;

	const int GetGameflowId() { return m_GameflowId; }
	const string GetGameflowName() { return m_GameflowName; }

	virtual FlagType GetDefaultFlagMask() const = 0;
	FlagType& GetFlags() { return m_Flags; }

	enum Phases {};

	CGameflowPhase* GetActivePhase() { return m_Phases[m_ActivePhase]; }
	CGameflowPhase* GetPhase(int Id);

	void StartNextPhase(int PhaseId);

	CGameflowManager* GetGameflowManager();

protected:
	int m_GameflowId;
	string m_GameflowName;

	std::vector<CGameflowPhase*> m_Phases;
	int m_ActivePhase = 0;
	void UpdatePhases(float fDeltaTime);
	// Adds an already initialized phase
	int AddPhase(CGameflowPhase *pPhase);
	template<typename T>
	CGameflowPhase* AddNewPhase()
	{
		T *pPhase = new T;
		pPhase->SetParentGameflow(this);
		pPhase->SetActivePhase(false); // Sets to not be the active phase
		pPhase->Initialize();
		AddPhase(pPhase); // Adds to m_Phases
		return pPhase;
	}
	// Sets by the index of m_Phases (*NOT THE PHASE ID*)
	void SetActivePhaseIndex(int Index);
	// Sets by phase Id
	void SetActivePhase(int PhaseId);
	void SetActivePhase(CGameflowPhase *pPhase);
	// Send to debug whenever the phase is set
	void SetActivePhase_Debug(CGameflowPhase *pPhase);
	

	void SetActiveGameflow(bool bActive);

	Cry::DefaultComponents::CInputComponent* GetInputComponent();

	FlagType m_Flags;
};
