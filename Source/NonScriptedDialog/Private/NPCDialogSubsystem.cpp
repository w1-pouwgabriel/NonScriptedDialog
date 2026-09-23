#include "NPCDialogSubsystem.h"
#include "Engine/Engine.h"
#include "LlamaSubsystem.h"

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

void UNPCDialogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (ULlamaSubsystem* Llama = GEngine->GetEngineSubsystem<ULlamaSubsystem>())
	{
		Llama->ModelParams.PathToModel = TEXT("./qwen2.5-1.5b-instruct-q8_0.gguf");
		Llama->ModelParams.SystemPrompt = TEXT("You are a helpful assistant.");
		//Llama->OnModelLoaded.AddDynamic(this, &UNPCDialogSubsystem::OnModelLoaded);
		Llama->ModelParams.StopSequences = { TEXT("\nPlayer:") };
		Llama->OnResponseGenerated.AddDynamic(this, &UNPCDialogSubsystem::HandleLlamaResponseGenerated);
		Llama->LoadModel();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UNPCDialogSubsystem::Initialize - ULlamaSubsystem not found. Is the LlamaCore plugin enabled?"));
	}
}

void UNPCDialogSubsystem::Deinitialize()
{
	if (GEngine)
	{
		if (ULlamaSubsystem* Llama = GEngine->GetEngineSubsystem<ULlamaSubsystem>())
		{
			Llama->OnResponseGenerated.RemoveDynamic(this, &UNPCDialogSubsystem::HandleLlamaResponseGenerated);
			// Deliberately NOT calling Llama->UnloadModel() here - ULlamaSubsystem
			// is engine-scoped and may outlive this game instance (e.g. PIE
			// stop/start), so unloading is its own concern, not ours.
		}
	}

	Super::Deinitialize();
}

bool UNPCDialogSubsystem::IsNPCRegistered(FName NPCId) const
{
	return ContextRegistry.Contains(NPCId);
}

void UNPCDialogSubsystem::RequestGeneration(FName NPCId, const FString& PlayerInput, FOnDialogResponse OnComplete)
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

	ULlamaSubsystem* Llama = GEngine ? GEngine->GetEngineSubsystem<ULlamaSubsystem>() : nullptr;
	if (!Llama || !Llama->IsModelLoaded())
	{
		UE_LOG(LogTemp, Warning, TEXT("ProcessNextRequest - model not loaded yet, dropping request for '%s'."), *NextRequest.NPCId.ToString());
		return; // Could re-queue instead of dropping, depending on how you want to handle a cold-start race.
	}

	// Mark as generating and remember which request is in flight, so
	// HandleLlamaResponseGenerated (which carries no NPC id of its own)
	// knows who the response belongs to and which callback to fire.
	bIsGenerating = true;
	CurrentGeneratingNPCId = NextRequest.NPCId;
	CurrentCallback = NextRequest.Callback;

	// Switched away from InsertRawPrompt - StopSequences doesn't appear to be
	// honored by the local backend, so raw completion had no way to stop
	// itself. Using InsertTemplatedPrompt instead means each turn is tagged
	// with a proper chat-template role, and the model's own trained
	// end-of-turn token (e.g. <|im_end|> for Qwen) stops generation - no
	// custom stop string required.
	Llama->ResetContextHistory(false);
	Llama->InsertTemplatedPrompt(State->CachedBaseSystemPrompt, EChatTemplateRole::System, false, false);

	const int32 LastIndex = State->ConversationHistory.Num() - 1;
	for (int32 i = 0; i < State->ConversationHistory.Num(); ++i)
	{
		const FNPCConversationEntry& Entry = State->ConversationHistory[i];
		const EChatTemplateRole Role = (Entry.Speaker == ENPCDialogSpeaker::Player)
			? EChatTemplateRole::User
			: EChatTemplateRole::Assistant;

		// Only the LAST message (the newest player line) should trigger generation -
		// everything before it is just replaying history into context.
		const bool bIsLastMessage = (i == LastIndex);
		Llama->InsertTemplatedPrompt(Entry.Text, Role, bIsLastMessage, /*bGenerateReply=*/bIsLastMessage);

		//UE_LOG(LogTemp, Log, TEXT("%hs: %s"), ToString(Entry.Speaker), *Entry.Text);
	}

}

void UNPCDialogSubsystem::HandleGenerationComplete(FName NPCId, FString GeneratedText, FOnDialogResponse OriginalCallback)
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
	CurrentGeneratingNPCId = NAME_None;
	CurrentCallback = FOnDialogResponse();

	if (OriginalCallback.IsBound())
	{
		OriginalCallback.Execute(GeneratedText);
	}

	// More may have queued up while this one was running - advance immediately.
	ProcessNextRequest();
}

FString UNPCDialogSubsystem::SanitizeGeneratedResponse(const FString& RawResponse) const
{
	// Defensive backstop: even with a stop sequence configured on the model,
	// truncate anything past the point where the model starts hallucinating
	// the PLAYER'S side of the conversation - never store or forward that.
	int32 CutIndex = RawResponse.Find(TEXT("\nPlayer:"), ESearchCase::IgnoreCase);
	FString Result = (CutIndex != INDEX_NONE) ? RawResponse.Left(CutIndex) : RawResponse;

	Result = Result.TrimEnd();

	return Result;
}

void UNPCDialogSubsystem::HandleLlamaResponseGenerated(const FString& Response)
{
	const FString CleanedResponse = SanitizeGeneratedResponse(Response);

	HandleGenerationComplete(CurrentGeneratingNPCId, CleanedResponse, CurrentCallback);
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