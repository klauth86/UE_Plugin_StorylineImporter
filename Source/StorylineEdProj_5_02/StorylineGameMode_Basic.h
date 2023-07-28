// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "Engine/StaticMeshActor.h"
#include "StorylineServiceBFL.h"
#include "StorylineGameMode_Basic.generated.h"

class USphereComponent;

UENUM(BlueprintType)
enum class EInteractionStatus
{
	UNSET = 0,
	STARTING,
	LOOP,
	ENDING
};

//------------------------------------------------------------------------
// UInteractionSource
//------------------------------------------------------------------------

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UInteractionSource : public UInterface
{
	GENERATED_BODY()
};

class STORYLINEEDPROJ_5_02_API IInteractionSource
{
	GENERATED_BODY()

public:

	virtual void StartInteraction() { check(0); }

	virtual void EndInteraction() { check(0); }

	virtual EInteractionStatus GetInteractionStatus() const { return EInteractionStatus::UNSET; }
};

//------------------------------------------------------------------------
// AStorylineSource_Basic
//------------------------------------------------------------------------

UCLASS()
class STORYLINEEDPROJ_5_02_API AStorylineSource_Basic : public AActor, public IStorylineSource
{
	GENERATED_BODY()

public:

	virtual UDataTable* GetCharacters_Implementation() const override { return Characters; }

	virtual UDataTable* GetDialogs_Implementation() const override { return Dialogs; }

	virtual UDataTable* GetGameEvents_Implementation() const override { return GameEvents; }

	virtual UDataTable* GetGameEventImpls_Implementation() const override { return GameEventImpls; }

	virtual UDataTable* GetItems_Implementation() const override { return Items; }

	virtual UDataTable* GetNodes_Implementation() const override { return Nodes; }

	virtual UDataTable* GetPredicates_Implementation() const override { return Predicates; }

	virtual UDataTable* GetPredicateImpls_Implementation() const override { return PredicateImpls; }

	virtual UDataTable* GetQuests_Implementation() const override { return Quests; }

	virtual UDataTable* GetReplicas_Implementation() const override { return Replicas; }

	virtual UDataTable* GetRichTexts_Implementation() const override { return RichTexts; }

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StorylineSource")
		UDataTable* Characters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StorylineSource")
		UDataTable* Dialogs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StorylineSource")
		UDataTable* GameEvents;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StorylineSource")
		UDataTable* GameEventImpls;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StorylineSource")
		UDataTable* Items;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StorylineSource")
		UDataTable* Nodes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StorylineSource")
		UDataTable* Predicates;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StorylineSource")
		UDataTable* PredicateImpls;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StorylineSource")
		UDataTable* Quests;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StorylineSource")
		UDataTable* Replicas;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StorylineSource")
		UDataTable* RichTexts;
};

class UAudioComponent;
enum class EAudioComponentPlayState : uint8;

//------------------------------------------------------------------------
// FQuestEntry
//------------------------------------------------------------------------

USTRUCT(BlueprintType)
struct STORYLINEEDPROJ_5_02_API FQuestAlternativeEntry
{
	GENERATED_USTRUCT_BODY()

public:

	FQuestAlternativeEntry()
	{
		Reset();
	}

	void Reset()
	{
		AlternativeNode.Reset();
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QuestAlternativeEntry")
		FNodeM AlternativeNode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QuestStepEntry")
		uint8 bIsPassed : 1;
};

inline uint32 GetTypeHash(const FQuestAlternativeEntry& questStepEntry) { return GetTypeHash(questStepEntry.AlternativeNode.Id); }

inline bool operator==(const FQuestAlternativeEntry& A, const FQuestAlternativeEntry& B) { return A.AlternativeNode.Id == B.AlternativeNode.Id; }

//------------------------------------------------------------------------
// FQuestEntry
//------------------------------------------------------------------------

USTRUCT(BlueprintType)
struct STORYLINEEDPROJ_5_02_API FQuestStepEntry
{
	GENERATED_USTRUCT_BODY()

public:

	FQuestStepEntry()
	{
		Reset();
	}

	void Reset()
	{
		StepNode.Reset();
		Alternatives.Reset();
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QuestStepEntry")
		FNodeM StepNode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QuestStepEntry")
		TSet<FQuestAlternativeEntry> Alternatives;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QuestStepEntry")
		uint8 bIsPassed : 1;
};

inline uint32 GetTypeHash(const FQuestStepEntry& questStepEntry) { return GetTypeHash(questStepEntry.StepNode.Id); }

inline bool operator==(const FQuestStepEntry& A, const FQuestStepEntry& B) { return A.StepNode.Id == B.StepNode.Id; }

//------------------------------------------------------------------------
// AStorylineContext_Basic
//------------------------------------------------------------------------

UCLASS(Blueprintable)
class STORYLINEEDPROJ_5_02_API AStorylineContext_Basic : public AActor, public IStorylineContext
{
	GENERATED_UCLASS_BODY()

public:

	virtual ESGender GetPlayerGender_Implementation() const override;

	UFUNCTION(BlueprintImplementableEvent)
		void BIE_OnStartDialog();

	virtual void OnStartDialog_Implementation(const FDialogM& dialog) override;

	UFUNCTION(BlueprintImplementableEvent)
		void BIE_OnEndDialog();

	virtual void OnEndDialog_Implementation() override;

	UFUNCTION(BlueprintImplementableEvent)
		void BIE_OnEnterNode();

	virtual void OnEnterNode_Implementation(const FNodeM& node) override;

	UFUNCTION(BlueprintImplementableEvent)
		void BIE_OnLeaveNode(bool IsBeforePlayerChoice);

	virtual void OnLeaveNode_Implementation(bool IsBeforePlayerChoice) override;

	virtual void OnPlayNode_Implementation() override;

	virtual void OnPauseNode_Implementation() override;

	virtual void OnResumeNode_Implementation() override;

	virtual void OnSkipNode_Implementation() override;

	virtual void GetNodePaths_Implementation(TArray<FNodePath>& outNodePaths) const override;

	virtual void SetNodePaths_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, const TArray<FNodePath>& nodePaths) override;

	UFUNCTION(BlueprintImplementableEvent)
		void BIE_OnPlayerChoice();

	virtual void OnPlayerChoice_Implementation() override { BIE_OnPlayerChoice(); }

	UFUNCTION(BlueprintCallable)
		void StartDialogWith(AActor* actor);

	UFUNCTION(BlueprintCallable)
		void SelectPlayerChoice(int32 nextNodeIndex);

protected:

	void OnSoundAssetLoaded(UObject* loadedObject);

	void OnAudioPlayStateChangedNative(const UAudioComponent* InAudioComponent, EAudioComponentPlayState NewPlayState);

	void OnTimerComplete();

	void Reset();

protected:

	UPROPERTY()
		TObjectPtr<UAudioComponent> AudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StorylineContext")
		float DefaultNodeDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StorylineContext")
		ESGender PlayerGender;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StorylineContext")
		FDialogM ActiveDialog;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StorylineContext")
		FDialogM CurrentDialog;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StorylineContext")
		FNodeM CurrentNode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StorylineContext")
		FCharacterM SpeakingCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StorylineContext")
		TArray<FNodeM> PlayerChoices;

	TWeakObjectPtr<AActor> SpeakingActor;

	TArray<FNodePath> NodePaths;

	FTimerHandle TimerHandle;

	int32 UUID;
};

//------------------------------------------------------------------------
// AStorylineGameMode_Basic
//------------------------------------------------------------------------

UCLASS()
class STORYLINEEDPROJ_5_02_API AStorylineGameMode_Basic : public AGameModeBase
{
	GENERATED_UCLASS_BODY()

public:

	virtual void BeginPlay() override;

	IStorylineSource* GetStorylineSource() const { return StorylineSource; }

	IStorylineContext* GetStorylineContext() const { return StorylineContext; }

	void AddDialogNode(FName nodeId) { DialogNodes.FindOrAdd(nodeId); }

	bool HasDialogNodeInPrevSessions(FName nodeId) const { return DialogNodes.Contains(nodeId); }

	bool HasItem(TSubclassOf<AActor> actorClass) const { return Inventory.Contains(actorClass); }

	bool HasQuest(FName questId) const { return Quests.Contains(questId); }

	bool HasQuestNode(FName nodeId) const { return QuestNodes.Contains(nodeId); }

	bool HasPassedQuestNode(FName nodeId) const { return QuestNodes.Contains(nodeId) && QuestNodes[nodeId]; }

	UFUNCTION(BlueprintImplementableEvent)
		void BIE_OnInventoryChanged();

	int32 DropItem(TSubclassOf<AActor> actorClass) { int32 result = Inventory.Remove(actorClass); BIE_OnInventoryChanged(); return result; }

	int32 PickUpItem(TSubclassOf<AActor> actorClass) { int32 result = Inventory.Add(actorClass); BIE_OnInventoryChanged(); return result; }

	UFUNCTION(BlueprintImplementableEvent)
		void BIE_OnQuestChanged();

	UFUNCTION(BlueprintCallable)
		void AddQuestNode(FName questId, FName nodeId);

	UFUNCTION(BlueprintCallable)
		void PassQuestNode(FName questId, FName nodeId);

	bool HasActiveDialog() const { return CurrentDialogId != NAME_None; }

	void SetActiveDialog(FName dialogId) { CurrentDialogId = dialogId; }

protected:

	UPROPERTY()
		AStorylineSource_Basic* StorylineSource;

	UPROPERTY()
		AStorylineContext_Basic* StorylineContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<AActor>> Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSet<FName> DialogNodes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSet<FName> Quests;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<FName, bool> QuestNodes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<FQuestStepEntry> QuestEntries;

	FName CurrentDialogId;
};

//------------------------------------------------------------------------
// ADialogCharacter
//------------------------------------------------------------------------

UCLASS()
class STORYLINEEDPROJ_5_02_API ADialogCharacter : public ACharacter, public IInteractionSource
{
	GENERATED_UCLASS_BODY()

public:

	virtual void Tick(float DeltaSeconds) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	virtual void StartInteraction() override;

	virtual EInteractionStatus GetInteractionStatus() const override;

protected:

	UFUNCTION(BlueprintCallable)
		void Talk();

	void SetInteractionActor(AActor* interactionActor);

	void SetInteractionActor_Internal(AActor* interactionActor);

	UFUNCTION(BlueprintImplementableEvent)
		void BIE_OnStartInteraction();

	UFUNCTION(BlueprintImplementableEvent)
		void BIE_OnEndInteraction();

	UFUNCTION(BlueprintImplementableEvent)
		void BIE_OnEnterInteraction();

	UFUNCTION(BlueprintImplementableEvent)
		void BIE_OnLeaveInteraction();

protected:

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY()
		TObjectPtr<AActor> InteractionActor;

	UPROPERTY()
		TObjectPtr<AActor> PendingInteractionActor;
};

//------------------------------------------------------------------------
// AStaticMeshActor
//------------------------------------------------------------------------

UCLASS()
class STORYLINEEDPROJ_5_02_API AInventoryActor : public AStaticMeshActor
{
	GENERATED_UCLASS_BODY()

protected:

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<USphereComponent> SphereComponent;
};

//------------------------------------------------------------------------
// UP_Dialog_Node_Has_PrevSessionsM
//------------------------------------------------------------------------

UCLASS(BlueprintType, Blueprintable)
class STORYLINEEDPROJ_5_02_API UP_Dialog_Node_Has_PrevSessionsM : public UP_Base
{
	GENERATED_BODY()

public:

	virtual bool Execute_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, const TScriptInterface<IStorylineContext>& storylineContext, const FPredicateM& predicate) const override;
};

//------------------------------------------------------------------------
// UP_Item_HasM
//------------------------------------------------------------------------

UCLASS(BlueprintType, Blueprintable)
class STORYLINEEDPROJ_5_02_API UP_Item_HasM : public UP_Base
{
	GENERATED_BODY()

public:

	virtual bool Execute_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, const TScriptInterface<IStorylineContext>& storylineContext, const FPredicateM& predicate) const override;
};

//------------------------------------------------------------------------
// UP_Quest_AddedM
//------------------------------------------------------------------------

UCLASS(BlueprintType, Blueprintable)
class STORYLINEEDPROJ_5_02_API UP_Quest_AddedM : public UP_Base
{
	GENERATED_BODY()

public:

	virtual bool Execute_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, const TScriptInterface<IStorylineContext>& storylineContext, const FPredicateM& predicate) const override;
};

//------------------------------------------------------------------------
// UP_Quest_Node_Passed
//------------------------------------------------------------------------

UCLASS(BlueprintType, Blueprintable)
class STORYLINEEDPROJ_5_02_API UP_Quest_Node_Passed : public UP_Base
{
	GENERATED_BODY()

public:

	virtual bool Execute_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, const TScriptInterface<IStorylineContext>& storylineContext, const FPredicateM& predicate) const override;
};

//------------------------------------------------------------------------
// UGE_Item_DropM
//------------------------------------------------------------------------

UCLASS(BlueprintType, Blueprintable)
class STORYLINEEDPROJ_5_02_API UGE_Item_DropM : public UGE_Base
{
	GENERATED_BODY()

public:

	virtual void Execute_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FGameEventM& gameEvent) const override;
};

//------------------------------------------------------------------------
// UGE_Quest_Node_AddM
//------------------------------------------------------------------------

UCLASS(BlueprintType, Blueprintable)
class STORYLINEEDPROJ_5_02_API UGE_Quest_Node_AddM : public UGE_Base
{
	GENERATED_BODY()

public:

	virtual void Execute_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FGameEventM& gameEvent) const override;
};

//------------------------------------------------------------------------
// UGE_Quest_Node_PassM
//------------------------------------------------------------------------

UCLASS(BlueprintType, Blueprintable)
class STORYLINEEDPROJ_5_02_API UGE_Quest_Node_PassM : public UGE_Base
{
	GENERATED_BODY()

public:

	virtual void Execute_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FGameEventM& gameEvent) const override;
};

//------------------------------------------------------------------------
// UGE_Quest_AddM
//------------------------------------------------------------------------

UCLASS(BlueprintType, Blueprintable)
class STORYLINEEDPROJ_5_02_API UGE_Quest_AddM : public UGE_Base
{
	GENERATED_BODY()

public:

	virtual void Execute_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FGameEventM& gameEvent) const override;
};