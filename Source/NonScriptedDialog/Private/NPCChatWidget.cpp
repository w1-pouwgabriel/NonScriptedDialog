#include "NPCChatWidget.h"
#include "NPCDialogComponent.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"

void UNPCChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SendButton)
	{
		SendButton->OnClicked.AddDynamic(this, &UNPCChatWidget::HandleSendClicked);
	}

	if (PlayerInputBox)
	{
		PlayerInputBox->OnTextCommitted.AddDynamic(this, &UNPCChatWidget::HandlePlayerInputCommitted);
	}
}

void UNPCChatWidget::SetTargetNPC(UNPCDialogComponent* InDialogueComponent, const FText& InNPCDisplayName)
{
	TargetDialogComponent = InDialogueComponent;
	NPCDisplayName = InNPCDisplayName;

	if (HistoryContainer)
	{
		HistoryContainer->ClearChildren();
	}

	bWaitingForResponse = false;
	if (PlayerInputBox)
	{
		PlayerInputBox->SetIsEnabled(true);
		PlayerInputBox->SetKeyboardFocus();
	}
}

void UNPCChatWidget::HandleSendClicked()
{
	if (PlayerInputBox)
	{
		SubmitPlayerLine(PlayerInputBox->GetText().ToString());
	}
}

void UNPCChatWidget::HandlePlayerInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// Only submit on Enter - ignore focus-lost commits so clicking elsewhere
	// in the window doesn't accidentally send a half-typed message.
	if (CommitMethod == ETextCommit::OnEnter)
	{
		SubmitPlayerLine(Text.ToString());
	}
}

void UNPCChatWidget::SubmitPlayerLine(const FString& PlayerLine)
{
	if (PlayerLine.IsEmpty() || !TargetDialogComponent || bWaitingForResponse)
	{
		return;
	}

	AppendMessageToHistory(FText::FromString(TEXT("You")), PlayerLine);

	if (PlayerInputBox)
	{
		PlayerInputBox->SetText(FText::GetEmpty());
		PlayerInputBox->SetIsEnabled(false); // Disabled until the response comes back.
	}
	bWaitingForResponse = true;

	FOnDialogResponse Callback;
	Callback.BindDynamic(this, &UNPCChatWidget::HandleNPCResponse);
	TargetDialogComponent->SendPlayerInput(PlayerLine, Callback);
}

void UNPCChatWidget::HandleNPCResponse(const FString& Response)
{
	AppendMessageToHistory(NPCDisplayName, Response);

	bWaitingForResponse = false;
	if (PlayerInputBox)
	{
		PlayerInputBox->SetIsEnabled(true);
		PlayerInputBox->SetKeyboardFocus();
	}
}

void UNPCChatWidget::AppendMessageToHistory(const FText& Speaker, const FString& MessageText)
{
	if (!HistoryContainer)
	{
		return;
	}

	UTextBlock* Line = NewObject<UTextBlock>(this);
	Line->SetText(FText::Format(
		NSLOCTEXT("NPCChat", "ChatLineFormat", "{0}: {1}"),
		Speaker,
		FText::FromString(MessageText)
	));
	Line->SetAutoWrapText(true);

	HistoryContainer->AddChildToVerticalBox(Line);

	if (HistoryScrollBox)
	{
		HistoryScrollBox->ScrollToEnd();
	}
}

void UNPCChatWidget::CloseChatWindow()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}

	RemoveFromParent();
}