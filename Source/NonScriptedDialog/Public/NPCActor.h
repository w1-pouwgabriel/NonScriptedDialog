// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NPCDialogComponent.h"
#include "NPCChatWidget.h"
#include "NPCActor.generated.h"

UCLASS()
class NONSCRIPTEDDIALOG_API ANPCActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANPCActor();

	// Attached in the constructor, so every NPC actor automatically has one.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC")
	TObjectPtr<UNPCDialogComponent> DialogComponent;

	/** Call this from wherever the player interacts with the NPC (e.g. an interact input or overlap event). */
	UFUNCTION(BlueprintCallable, Category = "NPC")
	void TalkTo(const FString& PlayerLine);

	// Assign your WBP_ChatWindow Blueprint (child of UNPCChatWidget) here in the editor.
	UPROPERTY(EditDefaultsOnly, Category = "NPC|Chat")
	TSubclassOf<UNPCChatWidget> ChatWidgetClass;

	/**
	 * Opens the chat window UI for this NPC - this is what your overlap event
	 * should call now, instead of firing a hardcoded TalkTo line. Switches
	 * input to UI-only so the player can type, and shows the cursor.
	 */
	UFUNCTION(BlueprintCallable, Category = "NPC")
	void OpenDialogueUI();

private:
	// Must be a UFUNCTION with this exact signature to bind to the dynamic
	// delegate - this is what fires once the shared model finishes generating.
	UFUNCTION()
	void OnDialogueResponseReceived(const FString& Response);

};
