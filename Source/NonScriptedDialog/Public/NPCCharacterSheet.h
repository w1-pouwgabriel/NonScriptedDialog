// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NPCCharacterSheet.generated.h"

/**
 * Raw persona data for a single NPC.
 * Kept as a plain USTRUCT so it can also be used inline (e.g. in a table row)
 * if I later want to bulk-define NPCs via a DataTable instead of individual assets.
 */
USTRUCT(BlueprintType)
struct NONSCRIPTEDDIALOG_API FNPCCharacterSheet
{
	GENERATED_BODY()

	// Display name of the NPC, e.g. "Bjorn Ironhide"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString CharacterName;

	// Their function in the world, e.g. "Blacksmith"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString Role;

	// 3-5 adjectives, kept as separate entries so designers don't need to
	// worry about comma formatting, they get joined when building the promt.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persona")
	TArray<FString> PersonalityTraits;

	// Vocabulary level, sentence length, verbal tics, accent notes, etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persona", meta = (MultiLine = "true"))
	FString SpeechStyle;

	// Just enough backstory for the model to reference naturally.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persona", meta = (MultiLine = "true"))
	FString Background;

	// What the NPC does and does NOT know about. Critical for stopping the
	// model from inventing lore or breaking world consistency.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge", meta = (MultiLine = "true"))
	FString KnowledgeBoundaries;

	// What the NPC wants, shaping how they respond to different topics.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persona", meta = (MultiLine = "true"))
	FString GoalsAndMotivations;

	// "Stranger", "Ally", "Rival", affects tone toward the player.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persona")
	FString RelationshipToPlayer;

	// Topics to avoid, refusals, safety rails specific to this NPC.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge", meta = (MultiLine = "true"))
	TArray<FString> Constraints;

	FNPCCharacterSheet()
		: CharacterName(TEXT(""))
		, Role(TEXT(""))
		, SpeechStyle(TEXT(""))
		, Background(TEXT(""))
		, KnowledgeBoundaries(TEXT(""))
		, GoalsAndMotivations(TEXT(""))
		, RelationshipToPlayer(TEXT(""))
	{
	}
};

/**
 * A single, creatable Content Browser asset wrapping one FNPCCharacterSheet.
 * Create one of these for each NPC.
 * 
 * Dynamic loading and unloading of DataAssets, this can be used later if we need to save some space
 * https://dev.epicgames.com/documentation/unreal-engine/asset-management-in-unreal-engine#registering-and-loading-dynamically-created-primary-assets
 */
UCLASS(BlueprintType)
class NONSCRIPTEDDIALOG_API UNPCCharacterSheetAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FNPCCharacterSheet CharacterSheet;

	/**
	 * Builds the static portion of the system prompt for this NPC.
	 * This does NOT include conversation history or the player's current
	 * input.
	 */
	UFUNCTION(BlueprintCallable, Category = "NPC")
	FString BuildBaseSystemPrompt() const;

#if WITH_EDITOR
	// Lets the Asset Manager categorize these distinctly from other data assets.
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
#endif
};
