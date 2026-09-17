// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NPCCharacterSheet.generated.h"

/**
 * Raw persona data for a single NPC.
 * Kept as a plain USTRUCT so it can also be used inline (e.g. in a table row)
 * if you later want to bulk-define NPCs via a DataTable instead of individual assets.
 */
USTRUCT(BlueprintType)
struct NONSCRIPTEDDIALOG_API FNPCCharacterSheet
{
	GENERATED_BODY()

	// Display name of the NPC, e.g. "Boran Ironhide"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString CharacterName;

	// Their function in the world, e.g. "Village blacksmith"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString Role;

	// 3-5 adjectives, kept as separate entries so designers don't need to
	// worry about comma formatting - we join them when building the prompt.
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

	// e.g. "Stranger", "Ally", "Rival" - affects tone toward the player.
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
 * Create one of these per NPC (e.g. DA_NPC_Boran, DA_NPC_Yselle) and assign it
 * to the NPC's dialogue component in the level or Blueprint.
 *
 * Being a UPrimaryDataAsset (rather than plain UDataAsset) means these can also
 * be enumerated in bulk later via the Asset Manager if you want to validate
 * all NPC sheets at once, or async-load them by PrimaryAssetId.
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
	 * input - those get appended separately (Week 3's next piece) so this
	 * block can be cached/reused across turns instead of rebuilt every time.
	 */
	UFUNCTION(BlueprintCallable, Category = "NPC")
	FString BuildBaseSystemPrompt() const;

#if WITH_EDITOR
	// Lets the Asset Manager categorize these distinctly from other data assets.
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
#endif
};
