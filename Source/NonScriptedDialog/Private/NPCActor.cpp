#include "NPCActor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

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

	// FOnDialogResponse is a dynamic delegate, so it must bind to a
	// UFUNCTION() on a UObject, a raw lambda or std::function won't work here.
	FOnDialogResponse Callback;
	Callback.BindDynamic(this, &ANPCActor::OnDialogueResponseReceived);

	DialogComponent->SendPlayerInput(PlayerLine, Callback);
}

void ANPCActor::OpenDialogueUI()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC || !ChatWidgetClass || !DialogComponent)
	{
		return;
	}

	UNPCChatWidget* ChatWidget = CreateWidget<UNPCChatWidget>(PC, ChatWidgetClass);
	if (!ChatWidget)
	{
		return;
	}

	FText DisplayName = FText::FromString(GetName());
	if (DialogComponent->CharacterSheetAsset)
	{
		DisplayName = FText::FromString(DialogComponent->CharacterSheetAsset->CharacterSheet.CharacterName);
	}

	ChatWidget->SetTargetNPC(DialogComponent, DisplayName);
	ChatWidget->AddToViewport();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(ChatWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;
}

void ANPCActor::OnDialogueResponseReceived(const FString& Response)
{
	// This fires once the shared model actually produces text for THIS NPC's
	// request - could be delayed if other NPCs' requests were ahead in queue.
	UE_LOG(LogTemp, Log, TEXT("%s says: %s"), *GetName(), *Response);

	// TODO: replace with your actual presentation - feed this into a dialogue
	// UI widget, subtitle text, or a text-to-speech call.
}