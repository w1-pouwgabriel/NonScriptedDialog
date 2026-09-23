#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NPCDialogTypes.h"
#include "Types/SlateEnums.h"
#include "NPCChatWidget.generated.h"

class UScrollBox;
class UVerticalBox;
class UEditableTextBox;
class UButton;
class UNPCDialogComponent;

UCLASS()
class NONSCRIPTEDDIALOG_API UNPCChatWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Call right after creating/showing the widget to bind it to one NPC's conversation. */
	UFUNCTION(BlueprintCallable, Category = "NPC|Chat")
	void SetTargetNPC(UNPCDialogComponent* InDialogueComponent, const FText& InNPCDisplayName);

	/** Closes the window and restores normal game input/camera control. */
	UFUNCTION(BlueprintCallable, Category = "NPC|Chat")
	void CloseChatWindow();

protected:
	virtual void NativeConstruct() override;

	// In the UMG Designer: ScrollBox named "HistoryScrollBox" containing a
	// VerticalBox named "HistoryContainer" - each chat line becomes one
	// TextBlock added into the VerticalBox at runtime.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> HistoryScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> HistoryContainer;

	// The input field pinned along the bottom of the window.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> PlayerInputBox;

	// Optional - Enter key alone (via OnTextCommitted) also sends.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SendButton;

private:
	UPROPERTY()
	TObjectPtr<UNPCDialogComponent> TargetDialogComponent;

	FText NPCDisplayName;

	// Prevents queuing a second request while the shared model is still
	// working on the first - keeps the UI honest about what's in flight.
	bool bWaitingForResponse = false;

	UFUNCTION()
	void HandleSendClicked();

	UFUNCTION()
	void HandlePlayerInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void HandleNPCResponse(const FString& Response);

	void SubmitPlayerLine(const FString& PlayerLine);
	void AppendMessageToHistory(const FText& Speaker, const FString& MessageText);
};