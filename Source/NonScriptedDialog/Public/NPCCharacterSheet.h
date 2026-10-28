// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NPCCharacterSheet.generated.h"

/**
 * Current emotional state, layered on top of the static personality below.
 * Lets the same NPC feel different turn to turn without needing a separate
 * character sheet per mood - e.g. Bjorn after a bad trade vs. Bjorn on a good day.
 */
UENUM(BlueprintType)
enum class ENPCMood : uint8
{
	Neutral,
	Cheerful,
	Grumpy,
	Suspicious,
	Melancholic,
	Excited,
	Tired
};

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

	// Optional - leave blank if age doesn't matter for this NPC.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString Age;

	// e.g. "he/him", "she/her", "they/them" - used when the model refers to the NPC in the third person.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString Pronouns;

	// 3-5 adjectives, kept as separate entries so designers don't need to
	// worry about comma formatting, they get joined when building the promt.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persona")
	TArray<FString> PersonalityTraits;

	// Small physical/verbal habits, e.g. "taps hammer twice before speaking", "never makes eye contact".
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persona")
	TArray<FString> Quirks;

	// Vocabulary level, sentence length, verbal tics, accent notes, etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persona", meta = (MultiLine = "true"))
	FString SpeechStyle;

	// Dry, slapstick, sarcastic, none at all - shapes how (or if) the NPC jokes around.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persona")
	FString HumorStyle;

	// Recurring phrases the NPC tends to say, e.g. "Mind the edge, it's sharp."
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persona")
	TArray<FString> Catchphrases;

	// Topics or things the NPC enjoys talking about or lights up around.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persona")
	TArray<FString> Likes;

	// Topics or things that visibly annoy or upset the NPC.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persona")
	TArray<FString> Dislikes;

	// What makes the NPC uneasy or defensive - good for believable reactions to certain topics.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persona")
	TArray<FString> Fears;

	// What the NPC believes in / won't compromise on, even under pressure.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persona", meta = (MultiLine = "true"))
	FString Values;

	// A believable weakness or flaw - makes the NPC feel human rather than a perfect vendor-bot.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persona", meta = (MultiLine = "true"))
	FString Flaws;

	// Just enough backstory for the model to reference naturally.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persona", meta = (MultiLine = "true"))
	FString Background;

	// What the NPC wants, shaping how they respond to different topics.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persona", meta = (MultiLine = "true"))
	FString GoalsAndMotivations;

	// What the NPC does and does NOT know about. Critical for stopping the
	// model from inventing lore or breaking world consistency.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge", meta = (MultiLine = "true"))
	FString KnowledgeBoundaries;

	// Something true about the NPC they wouldn't volunteer unless pushed, trusted, or tricked.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge", meta = (MultiLine = "true"))
	FString Secret;

	// Topics to avoid, refusals, safety rails specific to this NPC.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge", meta = (MultiLine = "true"))
	TArray<FString> Constraints;

	// "Stranger", "Ally", "Rival", affects tone toward the player.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
	FString RelationshipToPlayer;

	// 0 = completely guarded/distrustful, 1 = fully open - nudges how forthcoming the NPC is.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TrustLevel = 0.5f;

	// Current emotional state layered on top of the static personality above.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
	ENPCMood CurrentMood = ENPCMood::Neutral;

	FNPCCharacterSheet()
		: CharacterName(TEXT(""))
		, Role(TEXT(""))
		, Age(TEXT(""))
		, Pronouns(TEXT(""))
		, SpeechStyle(TEXT(""))
		, HumorStyle(TEXT(""))
		, Values(TEXT(""))
		, Flaws(TEXT(""))
		, Background(TEXT(""))
		, GoalsAndMotivations(TEXT(""))
		, KnowledgeBoundaries(TEXT(""))
		, Secret(TEXT(""))
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