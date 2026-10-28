#include "NPCCharacterSheet.h"

static FString MoodToString(ENPCMood Mood)
{
	switch (Mood)
	{
	case ENPCMood::Cheerful:    return TEXT("cheerful");
	case ENPCMood::Grumpy:      return TEXT("grumpy");
	case ENPCMood::Suspicious:  return TEXT("suspicious");
	case ENPCMood::Melancholic: return TEXT("melancholic");
	case ENPCMood::Excited:     return TEXT("excited");
	case ENPCMood::Tired:       return TEXT("tired");
	default:                    return TEXT("neutral");
	}
}

FString UNPCCharacterSheetAsset::BuildBaseSystemPrompt() const
{
	const FString Traits = FString::Join(CharacterSheet.PersonalityTraits, TEXT(", "));
	const FString Quirks = FString::Join(CharacterSheet.Quirks, TEXT(", "));
	const FString Catchphrases = FString::Join(CharacterSheet.Catchphrases, TEXT("\" / \""));
	const FString Likes = FString::Join(CharacterSheet.Likes, TEXT(", "));
	const FString Dislikes = FString::Join(CharacterSheet.Dislikes, TEXT(", "));
	const FString Fears = FString::Join(CharacterSheet.Fears, TEXT(", "));
	const FString ConstraintsList = FString::Join(CharacterSheet.Constraints, TEXT(" "));

	FString Identity = FString::Printf(TEXT("You are %s, %s"), *CharacterSheet.CharacterName, *CharacterSheet.Role);
	if (!CharacterSheet.Age.IsEmpty())
	{
		Identity += FString::Printf(TEXT(", age %s"), *CharacterSheet.Age);
	}
	Identity += TEXT(".\n");

	FString Prompt = Identity;

	auto AppendIfNotEmpty = [&Prompt](const FString& Label, const FString& Value)
		{
			if (!Value.IsEmpty())
			{
				Prompt += FString::Printf(TEXT("%s: %s\n"), *Label, *Value);
			}
		};

	AppendIfNotEmpty(TEXT("Personality"), Traits);
	AppendIfNotEmpty(TEXT("Quirks"), Quirks);
	AppendIfNotEmpty(TEXT("Speech style"), CharacterSheet.SpeechStyle);
	AppendIfNotEmpty(TEXT("Sense of humor"), CharacterSheet.HumorStyle);
	if (!Catchphrases.IsEmpty())
	{
		Prompt += FString::Printf(TEXT("Phrases you often say: \"%s\"\n"), *Catchphrases);
	}
	AppendIfNotEmpty(TEXT("Likes"), Likes);
	AppendIfNotEmpty(TEXT("Dislikes"), Dislikes);
	AppendIfNotEmpty(TEXT("Fears"), Fears);
	AppendIfNotEmpty(TEXT("Values"), CharacterSheet.Values);
	AppendIfNotEmpty(TEXT("Flaws"), CharacterSheet.Flaws);
	AppendIfNotEmpty(TEXT("Background"), CharacterSheet.Background);
	AppendIfNotEmpty(TEXT("Goals and motivations"), CharacterSheet.GoalsAndMotivations);
	AppendIfNotEmpty(TEXT("Knowledge boundaries"), CharacterSheet.KnowledgeBoundaries);
	AppendIfNotEmpty(TEXT("Secret (do not reveal unless the player earns enough trust)"), CharacterSheet.Secret);
	AppendIfNotEmpty(TEXT("Relationship to the player"), CharacterSheet.RelationshipToPlayer);
	AppendIfNotEmpty(TEXT("Constraints"), ConstraintsList);

	Prompt += FString::Printf(TEXT("Current mood: %s (let this color your tone without overriding your core personality).\n"), *MoodToString(CharacterSheet.CurrentMood));
	Prompt += FString::Printf(TEXT("Trust toward the player: %.0f%% - "), CharacterSheet.TrustLevel * 100.0f);
	Prompt += (CharacterSheet.TrustLevel < 0.3f)
		? TEXT("stay guarded and don't over-share.\n")
		: (CharacterSheet.TrustLevel > 0.7f)
		? TEXT("feel free to be open and forthcoming.\n")
		: TEXT("be polite but measured.\n");

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