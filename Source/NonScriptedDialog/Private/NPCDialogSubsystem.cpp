// NPCDialogSubsystem.cpp

#include "NPCDialogSubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UNPCDialogSubsystem::RegisterNPC(FName NPCId, UNPCCharacterSheetAsset* CharacterSheetAsset, int32 MaxHistoryEntries)
{
	if (ContextRegistry.Contains(NPCId))
	{
		return; // Already registered - safe to call again from BeginPlay on level reload etc.
	}

	if (!CharacterSheetAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("RegisterNPC called for '%s' with no CharacterSheetAsset."), *NPCId.ToString());
		return;
	}

	FNPCConversationState NewState;
	NewState.CharacterSheetAsset = CharacterSheetAsset;
	NewState.CachedBaseSystemPrompt = CharacterSheetAsset->BuildBaseSystemPrompt();
	NewState.MaxHistoryEntries = MaxHistoryEntries;

	ContextRegistry.Add(NPCId, MoveTemp(NewState));
}

bool UNPCDialogSubsystem::IsNPCRegistered(FName NPCId) const
{
	return ContextRegistry.Contains(NPCId);
}

void UNPCDialogSubsystem::RequestGeneration(FName NPCId, const FString& PlayerInput, FOnDialogueResponse OnComplete)
{
	FNPCConversationState* State = ContextRegistry.Find(NPCId);
	if (!State)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestGeneration called for unregistered NPC '%s'."), *NPCId.ToString());
		return;
	}

	// Record the player's line immediately so it's part of history even
	// while the request is still sitting in the queue.
	FNPCConversationEntry Entry;
	Entry.Speaker = ENPCDialogSpeaker::Player;
	Entry.Text = PlayerInput;
	State->ConversationHistory.Add(Entry);
	TrimHistoryIfNeeded(*State);

	FPendingDialogueRequest Request;
	Request.NPCId = NPCId;
	Request.PlayerInput = PlayerInput;
	Request.Callback = OnComplete;
	PendingRequests.Enqueue(MoveTemp(Request));

	ProcessNextRequest();
}

void UNPCDialogSubsystem::ProcessNextRequest()
{
	if (bIsGenerating)
	{
		return; // Something is already running against the shared model - it'll call this again when done.
	}

	FPendingDialogueRequest NextRequest;
	if (!PendingRequests.Dequeue(NextRequest))
	{
		return; // Nothing queued.
	}

	FNPCConversationState* State = ContextRegistry.Find(NextRequest.NPCId);
	if (!State)
	{
		// NPC was unregistered between request and processing - drop it and move on.
		ProcessNextRequest();
		return;
	}

	bIsGenerating = true;
	const FString FullPrompt = BuildFullPromptForNPC(*State);

	// ---------------------------------------------------------------
	// TODO: replace this block with the actual Llama Unreal call, e.g.
	// something like:
	//
	//   ULlamaComponent* Llama = GetLlamaComponent();
	//   Llama->OnResponseGenerated.AddUniqueDynamic(this, &UNPCDialogSubsystem::HandleGenerationComplete);
	//   Llama->Generate(FullPrompt);
	//
	// Since this subsystem only owns ONE shared model, that component
	// reference should live here (or be fetched from wherever your Llama
	// Unreal plugin exposes its single instance), not per-NPC.
	// The stub below fakes an async call with a timer so the queueing
	// logic is testable before the real model is wired in.
	FName NPCId = NextRequest.NPCId;
	FOnDialogueResponse Callback = NextRequest.Callback;
	FString StubResponse = FString::Printf(TEXT("[stub response to: %s]"), *FullPrompt.Right(60));

	FTimerHandle StubTimer;
	GetWorld()->GetTimerManager().SetTimer(
		StubTimer,
		[this, NPCId, StubResponse, Callback]()
		{
			HandleGenerationComplete(NPCId, StubResponse, Callback);
		},
		0.25f,
		false
	);
	// ---------------------------------------------------------------
}

void UNPCDialogSubsystem::HandleGenerationComplete(FName NPCId, FString GeneratedText, FOnDialogueResponse OriginalCallback)
{
	if (FNPCConversationState* State = ContextRegistry.Find(NPCId))
	{
		FNPCConversationEntry Entry;
		Entry.Speaker = ENPCDialogSpeaker::NPC;
		Entry.Text = GeneratedText;
		State->ConversationHistory.Add(Entry);
		TrimHistoryIfNeeded(*State);
	}

	bIsGenerating = false;

	if (OriginalCallback.IsBound())
	{
		OriginalCallback.Execute(GeneratedText);
	}

	// More may have queued up while this one was running - advance immediately.
	ProcessNextRequest();
}

FString UNPCDialogSubsystem::BuildFullPromptForNPC(const FNPCConversationState& State) const
{
	FString HistoryBlock;
	for (const FNPCConversationEntry& Entry : State.ConversationHistory)
	{
		const FString SpeakerLabel = (Entry.Speaker == ENPCDialogSpeaker::Player)
			? TEXT("Player")
			: (State.CharacterSheetAsset ? State.CharacterSheetAsset->CharacterSheet.CharacterName : TEXT("NPC"));

		HistoryBlock += FString::Printf(TEXT("%s: %s\n"), *SpeakerLabel, *Entry.Text);
	}

	return FString::Printf(
		TEXT("%s\n\nConversation so far:\n%s"),
		*State.CachedBaseSystemPrompt,
		*HistoryBlock
	);
}

void UNPCDialogSubsystem::TrimHistoryIfNeeded(FNPCConversationState& State)
{
	const int32 Overflow = State.ConversationHistory.Num() - State.MaxHistoryEntries;
	if (Overflow > 0)
	{
		State.ConversationHistory.RemoveAt(0, Overflow);
	}
}

void UNPCDialogSubsystem::ResetConversation(FName NPCId)
{
	if (FNPCConversationState* State = ContextRegistry.Find(NPCId))
	{
		State->ConversationHistory.Empty();
	}
}
