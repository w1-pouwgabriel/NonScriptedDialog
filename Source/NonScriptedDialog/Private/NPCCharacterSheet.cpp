#include "NPCCharacterSheet.h"

FString UNPCCharacterSheetAsset::BuildBaseSystemPrompt() const
{
	// Joins list fields into a readable format and assembles the fixed "who this NPC is" block. 
	// Conversation history and the live player input get appended after this by the dialogue component at runtime.
	// Fields left empty on the data asset are skipped entirely rather than
	// emitting a blank "Label: " makes sure that there a now tokens wasted at the start

	const FString Traits = FString::Join(CharacterSheet.PersonalityTraits, TEXT(", "));
	const FString ConstraintsList = FString::Join(CharacterSheet.Constraints, TEXT(" "));

	FString Prompt = FString::Printf(
		TEXT("You are %s, %s.\n"),
		*CharacterSheet.CharacterName,
		*CharacterSheet.Role
	);

	auto AppendIfNotEmpty = [&Prompt](const FString& Label, const FString& Value)
		{
			if (!Value.IsEmpty())
			{
				Prompt += FString::Printf(TEXT("%s: %s\n"), *Label, *Value);
			}
		};

	AppendIfNotEmpty(TEXT("Personality"), Traits);
	AppendIfNotEmpty(TEXT("Speech style"), CharacterSheet.SpeechStyle);
	AppendIfNotEmpty(TEXT("Background"), CharacterSheet.Background);
	AppendIfNotEmpty(TEXT("Knowledge boundaries"), CharacterSheet.KnowledgeBoundaries);
	AppendIfNotEmpty(TEXT("Goals and motivations"), CharacterSheet.GoalsAndMotivations);
	AppendIfNotEmpty(TEXT("Relationship to the player"), CharacterSheet.RelationshipToPlayer);
	AppendIfNotEmpty(TEXT("Constraints"), ConstraintsList);

	Prompt += FString::Printf(
		TEXT("Stay in character at all times and respond only as %s would."),
		*CharacterSheet.CharacterName
	);

	return Prompt;
}

#if WITH_EDITOR
FPrimaryAssetId UNPCCharacterSheetAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("NPCCharacterSheet"), GetFName());
}
#endif