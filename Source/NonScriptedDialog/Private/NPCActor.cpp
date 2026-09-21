#include "NPCActor.h"

ANPCActor::ANPCActor()
{
	PrimaryActorTick.bCanEverTick = false;

	DialogComponent = CreateDefaultSubobject<UNPCDialogComponent>(TEXT("DialogComponent"));
}

void ANPCActor::TalkTo(const FString& PlayerLine)
{
	if (!DialogComponent)
	{
		return;
	}

	// FOnDialogueResponse is a dynamic delegate, so it must bind to a
	// UFUNCTION() on a UObject, a raw lambda or std::function won't work here.
	FOnDialogueResponse Callback;
	Callback.BindDynamic(this, &ANPCActor::OnDialogueResponseReceived);

	DialogComponent->SendPlayerInput(PlayerLine, Callback);
}

void ANPCActor::OnDialogueResponseReceived(const FString& Response)
{
	// This fires once the shared model actually produces text for THIS NPC's
	// request - could be delayed if other NPCs' requests were ahead in queue.
	UE_LOG(LogTemp, Log, TEXT("%s says: %s"), *GetName(), *Response);

	// TODO: replace with your actual presentation - feed this into a dialogue
	// UI widget, subtitle text, or a text-to-speech call.
}