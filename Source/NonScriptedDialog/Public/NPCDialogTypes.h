// Shared types used by both UNPCDialogueComponent and UNPCDialogueSubsystem.
// Split out to avoid a circular dependency between the two.

#pragma once

#include "CoreMinimal.h"
#include "NPCDialogTypes.generated.h"

UENUM(BlueprintType)
enum class ENPCDialogSpeaker : uint8
{
	Player,
	NPC
};

inline const char* ToString(ENPCDialogSpeaker v)
{
	switch (v)
	{
	case ENPCDialogSpeaker::Player:   return "Player";
	case ENPCDialogSpeaker::NPC:   return "NPC";
	default:      return "[Unknown Speaker type]";
	}
}

/** A single turn in a conversation. */
USTRUCT(BlueprintType)
struct NONSCRIPTEDDIALOG_API FNPCConversationEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	ENPCDialogSpeaker Speaker = ENPCDialogSpeaker::Player;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString Text;
};

/** Delegate fired once the shared model finishes generating a response. */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDialogResponse, const FString&, Response);
