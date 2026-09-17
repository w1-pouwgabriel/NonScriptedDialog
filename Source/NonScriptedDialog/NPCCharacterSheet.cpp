#include "NPCCharacterSheet.h"

FString UNPCCharacterSheetAsset::BuildBaseSystemPrompt() const
{
	// Joins list fields into a readable format and assembles the fixed "who this NPC is" block. 
	// Conversation history and the live player input get appended after this by the dialogue component at runtime.

	const FString Traits = FString::Join(CharacterSheet.PersonalityTraits, TEXT(", "));
	const FString ConstraintsList = FString::Join(CharacterSheet.Constraints, TEXT(" "));

	return FString::Printf(
		TEXT(
			"You are %s, %s.\n"
			"Personality: %s\n"
			"Speech style: %s\n"
			"Background: %s\n"
			"Knowledge boundaries: %s\n"
			"Goals and motivations: %s\n"
			"Relationship to the player: %s\n"
			"Constraints: %s\n"
			"Stay in character at all times and respond only as %s would."
		),
		*CharacterSheet.CharacterName,
		*CharacterSheet.Role,
		*Traits,
		*CharacterSheet.SpeechStyle,
		*CharacterSheet.Background,
		*CharacterSheet.KnowledgeBoundaries,
		*CharacterSheet.GoalsAndMotivations,
		*CharacterSheet.RelationshipToPlayer,
		*ConstraintsList,
		*CharacterSheet.CharacterName
	);
}

#if WITH_EDITOR
FPrimaryAssetId UNPCCharacterSheetAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("NPCCharacterSheet"), GetFName());
}
#endif