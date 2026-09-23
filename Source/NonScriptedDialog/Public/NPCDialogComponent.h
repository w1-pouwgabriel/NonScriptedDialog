// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCCharacterSheet.h"
#include "NPCDialogTypes.h"
#include "NPCDialogComponent.generated.h"

UCLASS(ClassGroup = (Dialogue), meta = (BlueprintSpawnableComponent))
class NONSCRIPTEDDIALOG_API UNPCDialogComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNPCDialogComponent();

	// Unique ID this NPC is registered under in the subsystem's context
	// registry. Defaults to the owning actor's name if left blank.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FName NPCId;

	// The persona/data asset created for this NPC (e.g. DA_Bjorn).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	TObjectPtr<UNPCCharacterSheetAsset> CharacterSheetAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC", meta = (ClampMin = "1"))
	int32 MaxHistoryEntries = 12;

	/**
	 * Sends the player's line to the subsystem and queues a generation
	 * request. OnComplete fires once the shared model produces a response
	 * for THIS request specifically (the subsystem serializes all NPCs'
	 * requests, so this may not be immediate if others are ahead in queue).
	 */
	UFUNCTION(BlueprintCallable, Category = "NPC")
	void SendPlayerInput(const FString& PlayerInput, FOnDialogResponse OnComplete);

	/** Clears this NPC's history in the subsystem. */
	UFUNCTION(BlueprintCallable, Category = "NPC")
	void ResetConversation();

protected:
	virtual void BeginPlay() override;
};