#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Queue.h"
#include "NPCCharacterSheet.h"
#include "NPCDialogTypes.h"
#include "LlamaSubsystem.h"
#include "NPCDialogSubsystem.generated.h"

/** Everything the subsystem needs to remember about one NPC. */
USTRUCT()
struct FNPCConversationState
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UNPCCharacterSheetAsset> CharacterSheetAsset = nullptr;

	// Built once when the NPC registers, since persona data doesn't change
	// mid-conversation - avoids re-formatting it on every single turn.
	UPROPERTY()
	FString CachedBaseSystemPrompt;

	UPROPERTY()
	TArray<FNPCConversationEntry> ConversationHistory;

	UPROPERTY()
	int32 MaxHistoryEntries = 12;
};

/** One queued generation request, waiting its turn against the shared model. */
struct FPendingDialogueRequest
{
	FName NPCId;
	FString PlayerInput;
	FOnDialogResponse Callback;
};

/**
 * Game-instance-lifetime subsystem. All NPC context lives here, keyed by
 * NPCId, instead of scattered across per-actor components. NPC actors only
 * hold an ID and forward requests here.
 */
UCLASS()
class NONSCRIPTEDDIALOG_API UNPCDialogSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Binds to ULlamaSubsystem::OnResponseGenerated. Model loading itself is
	// triggered here too, since this is the first point anything needs it -
	// but the load call and the model instance both belong to ULlamaSubsystem.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Call once per NPC, e.g. from UNPCDialogueComponent::BeginPlay. Safe to call again (no-op if already registered). */
	UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
	void RegisterNPC(FName NPCId, UNPCCharacterSheetAsset* CharacterSheetAsset, int32 MaxHistoryEntries = 12);

	/**
	 * Records the player's line for NPCId and enqueues a generation request.
	 * OnComplete fires with the generated text once this request reaches the
	 * front of the queue and the model finishes. Requests are processed one
	 * at a time in submission order, across ALL NPCs, since they share one model.
	 */
	UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
	void RequestGeneration(FName NPCId, const FString& PlayerInput, FOnDialogResponse OnComplete);

	/** Clears one NPC's history, e.g. when a conversation session ends. */
	UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
	void ResetConversation(FName NPCId);

	UFUNCTION(BlueprintPure, Category = "NPC|Dialogue")
	bool IsNPCRegistered(FName NPCId) const;

	UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
	void SaveConversation(FName NPCId);

	UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
	TArray<FNPCConversationEntry> GetConversationHistory(FName NPCId) const;

	FString GetHistoryFilePath(FName NPCId) const;
	void SaveHistoryToDisk(FName NPCId, const FNPCConversationState& State); 
	bool LoadHistoryFromDisk(FName NPCId, const UNPCCharacterSheetAsset* CharacterSheetAsset, TArray<FNPCConversationEntry>& OutHistory) const;
	FString GetHistoryFilePath(FName NPCId, const UNPCCharacterSheetAsset* CharacterSheetAsset) const;
	FString SanitizeForFilename(const FString& Input) const;

	UFUNCTION(BlueprintCallable, Category = "NPC|Benchmark")
	void StartBenchmarkSweep(FName NPCId);

	UPROPERTY(BlueprintAssignable, Category = "NPC|Dialogue")
	FOnNPCTokenStreamed OnNPCTokenStreamed;

private:
	// Fixed battery run against every model, so results are directly comparable.
	TArray<FString> BenchmarkModelPaths = {
		TEXT("./qwen2.5-1.5b-instruct-q2_k.gguf"),
		TEXT("./qwen2.5-1.5b-instruct-q3_k_m.gguf"),
		TEXT("./qwen2.5-1.5b-instruct-q4_0.gguf"),
		TEXT("./qwen2.5-1.5b-instruct-q4_k_m.gguf"),
		TEXT("./qwen2.5-1.5b-instruct-q5_0.gguf"),
		TEXT("./qwen2.5-1.5b-instruct-q5_k_m.gguf"),
		TEXT("./qwen2.5-1.5b-instruct-q6_k.gguf"),
		TEXT("./qwen2.5-1.5b-instruct-q8_0.gguf"),
		TEXT("./qwen2.5-1.5b-instruct-fp16.gguf"),
	};

	TArray<FString> BenchmarkPrompts = {
		TEXT("What do you sell?"),
		TEXT("Do you have any weapons?"),
		TEXT("How much does a sword cost?"),
		TEXT("Can you make something custom for me?"),
		TEXT("Thank you, goodbye."),
	};

	FName BenchmarkNPCId;
	int32 BenchmarkModelIndex = 0;
	int32 BenchmarkPromptIndex = 0;
	bool bBenchmarkRunning = false;

	// Bound to ULlamaSubsystem::OnModelLoaded - verify FModelNameSignature's
	// actual parameter type in LlamaDataTypes.h and adjust if needed.
	UFUNCTION()
	void HandleBenchmarkModelLoaded(const FString& ModelName);

	void LoadNextBenchmarkModel();
	void RunNextBenchmarkPrompt();

	UFUNCTION()
	void HandleBenchmarkPromptResponse(const FString& Response);

	// The context registry: single source of truth for every NPC's state.
	UPROPERTY()
	TMap<FName, FNPCConversationState> ContextRegistry;

	TQueue<FPendingDialogueRequest> PendingRequests;
	bool bIsGenerating = false;

	UPROPERTY()
	TObjectPtr<ULlamaSubsystem> LlamaModelInstance;

	// Which NPC's request is currently in flight against ULlamaSubsystem, and
	// the callback to fire when it completes. ULlamaSubsystem's own delegate
	// doesn't carry an NPC id, so we track this ourselves. This is safe because
	// bIsGenerating guarantees only one request is ever in flight at a time.
	FName CurrentGeneratingNPCId;
	FOnDialogResponse CurrentCallback;

	// Pops the next request (if any) and kicks off generation, provided
	// nothing else is currently running against the shared model.
	void ProcessNextRequest();

	// Builds persona + rolling history into the final prompt string for one NPC.
	FString BuildFullPromptForNPC(const FNPCConversationState& State) const;

	void TrimHistoryIfNeeded(FNPCConversationState& State);

	// Called once the model finishes; records the response, fires the
	// caller's callback, then advances the queue.
	void HandleGenerationComplete(FName NPCId, FString GeneratedText, FOnDialogResponse OriginalCallback);
	FString SanitizeGeneratedResponse(const FString& RawResponse) const;

	// Bound to ULlamaSubsystem::OnResponseGenerated. Double check this
	// signature against FOnResponseGeneratedSignature in LlamaDataTypes.h.
	UFUNCTION()
	void HandleLlamaResponseGenerated(const FString& Response);

	double RequestStartTimeSeconds = 0.0;
	int32 CurrentGeneratedTokenCount = 0;

	UFUNCTION()
	void HandleLlamaTokenGenerated(const FString& Token);

	void LogPerformanceSample(FName NPCId, int32 TokenCount, double ElapsedSeconds) const;
	FString GetBenchmarkFilePath() const;
};
