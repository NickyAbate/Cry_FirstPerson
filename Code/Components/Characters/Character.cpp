#include "StdAfx.h"
#include "Character.h"

#include <CrySchematyc/Reflection/TypeDesc.h>
#include <CrySchematyc/Utils/EnumFlags.h>
#include <CrySchematyc/Env/IEnvRegistry.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/Elements/EnvFunction.h>
#include <CrySchematyc/Env/Elements/EnvSignal.h>
#include <CrySchematyc/ResourceTypes.h>
#include <CrySchematyc/MathTypes.h>
#include <CrySchematyc/Utils/SharedString.h>

#include "Utils\ConsoleCommands.h"
#include "UI\SpeechBubbles.h"
#include "GamePlugin.h"
#include "Game\UIController.h"
#include "Components\Game\GameplayEntity.h"
#include "Components\Game\Stats.h"

static void RegisterCharacterComponent(Schematyc::IEnvRegistrar& registrar)
{
	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CCharacterComponent));
		// Functions
		{
		}
	}
}
CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterCharacterComponent)

void CCharacterComponent::Initialize()
{
	// Create the camera component, will automatically update the viewport every frame
	//m_pCameraComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CCameraComponent>();

	// The character controller is responsible for maintaining player physics
	m_pCharacterController = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CCharacterControllerComponent>();
	// Offset the default character controller up by one unit
	m_pCharacterController->SetTransformMatrix(Matrix34::Create(Vec3(1.f), IDENTITY, Vec3(0, 0, 1.f)));
	//m_pEntity->RemoveComponent(m_pCharacterController);

	// Create the advanced animation component, responsible for updating Mannequin and animating the player
	m_pAnimationComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CAdvancedAnimationComponent>();

	// Set the player geometry, this also triggers physics proxy creation
	m_pAnimationComponent->SetMannequinAnimationDatabaseFile("Objects/Characters/Human_01/Mannequin/ADB/Human_01.adb");
	m_pAnimationComponent->SetCharacterFile("Objects/Characters/Human_01/Human_01.cdf");

	m_pAnimationComponent->SetControllerDefinitionFile("Objects/Characters/Human_01/Mannequin/ADB/Human_01ControllerDefinition.xml");
	m_pAnimationComponent->SetDefaultScopeContextName("Human_01Character");
	// Queue the idle fragment to start playing immediately on next update
	m_pAnimationComponent->SetDefaultFragmentName("Dab");

	// Disable movement coming from the animation (root joint offset), we control this entirely via physics
	m_pAnimationComponent->SetAnimationDrivenMotion(false);

	// Load the character and Mannequin data from file
	m_pAnimationComponent->LoadFromDisk();

	m_pGameplayEntityComponent = m_pEntity->GetOrCreateComponent<CGameplayEntityComponent>(false);
	//m_pGameplayEntityComponent->SetHealthStat(CGameplayEntityComponent::Stat<float>(0, 100, 85));

	m_pStatsComponent = m_pEntity->GetOrCreateComponent<CStatsComponent>();

	const Schematyc::CClassMemberDescArray &Members = GetClassDesc().GetMembers();
	for (int i = 0; i < Members.size(); i++)
	{
		const Schematyc::CClassMemberDesc &Member = Members[i];
		const string Name = Member.GetName();
		if (Name.find("PARAM_") != 0) continue;
		const CStatsComponent::Key Id = Member.GetId();
		//if (!m_pStatsComponent->HasParam(Id)) continue;
		const ptrdiff_t Offset = Member.GetOffset();
		UINT_PTR Ptr = ((UINT_PTR)this + Offset);
		m_pStatsComponent->BindParamPTR(Id, Ptr);
	}

}
#include "Components\Bullet.h"
void CCharacterComponent::ProcessEvent(SEntityEvent & event)
{
	switch (event.event)
	{
	default:
		break;
	case ENTITY_EVENT_COMPONENT_PROPERTY_CHANGED:
		UPDATEREFLECTEDPARAMS();
	case ENTITY_EVENT_UPDATE:
		break;
	case ENTITY_EVENT_COLLISION:
		{
			// Collision info can be retrieved using the event pointer
			EventPhysCollision *physCollision = reinterpret_cast<EventPhysCollision *>(event.nParam[0]);
			if (CBulletComponent *pBulletComponent = gEnv->pEntitySystem->GetEntityFromPhysics(physCollision->pEntity[1])->GetComponent<CBulletComponent>())
			{
				pBulletComponent->Test();
			}
		} 
		break;
	}
}

void CCharacterComponent::ReflectParams(Schematyc::CTypeDesc<CCharacterComponent>& desc)
{
	REFLECTPARAM(&CCharacterComponent::m_Health, Health, MinMaxVar<float>());
}

void CCharacterComponent::Speak(string Text)
{
	if (!m_pSpeechBubble)
	{
		const auto PosFunc = [this]() {return m_pAnimationComponent->GetCharacter()->GetIAttachmentManager()->GetInterfaceByName("Mouth_Attachment")->GetAttWorldAbsolute().t; };
		m_pSpeechBubble = CGamePlugin::gGamePlugin->m_pUIController->m_pSpeechBubbleManager->NewBubble(Text);
		m_pSpeechBubble->SetPosFunc(PosFunc);
	}
	else
	{
		m_pSpeechBubble->SetText(Text);
	}
}

void CCharacterComponent::Ragdollize()
{
	m_pCharacterController->Ragdollize();
}

void CC_RagdollTest(CC_Args pArgs)
{
	IEntity *pEntity = gEnv->pEntitySystem->FindEntityByName("Human 01");
	if (!pEntity) { gEnv->pLog->LogError("No 'Human 01' Entity"); return; }
	auto pCharacterComponent = pEntity->GetComponent<CCharacterComponent>();
	pCharacterComponent->Ragdollize();
}
CC_Info CC_RagdollTestInfo = CC_Info("RagdollTest", 0);
ADDCONSOLECOMMAND_WITHINFOCLASS(CC_RagdollTestInfo, CC_RagdollTest)