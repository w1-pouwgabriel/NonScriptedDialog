#include "NPCDialogComponent.h"
#include "NPCDialogSubsystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UNPCDialogComponent::UNPCDialogComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNPCDialogComponent::BeginPlay()
{
	Super::BeginPlay();

	if (NPCId.IsNone() && GetOwner())
	{
		NPCId = GetOwner()->GetFName();
	}

	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UNPCDialogSubsystem* Subsystem = GI->GetSubsystem<UNPCDialogSubsystem>())
		{
			Subsystem->RegisterNPC(NPCId, CharacterSheetAsset, MaxHistoryEntries);
		}
	}
}

void UNPCDialogComponent::SendPlayerInput(const FString& PlayerInput, FOnDialogueResponse OnComplete)
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI)
	{
		return;
	}

	if (UNPCDialogSubsystem* Subsystem = GI->GetSubsystem<UNPCDialogSubsystem>())
	{
		Subsystem->RequestGeneration(NPCId, PlayerInput, OnComplete);
	}
}

void UNPCDialogComponent::ResetConversation()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI)
	{
		return;
	}

	if (UNPCDialogSubsystem* Subsystem = GI->GetSubsystem<UNPCDialogSubsystem>())
	{
		Subsystem->ResetConversation(NPCId);
	}
}
