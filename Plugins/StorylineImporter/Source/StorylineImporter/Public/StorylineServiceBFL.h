// Copyright 2023 Pentangle Studio under EULA

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/Interface.h"
#include "Engine/DataTable.h"
#include "Templates/SubclassOf.h"
#include "StorylineServiceBFL.generated.h"

class USoundBase;

namespace StorylineTypeAttributes
{
	const FName FolderM = "FolderM";

	const FName Node_RandomM = "Node_RandomM";
	const FName Node_TransitM = "Node_TransitM";
	const FName Node_GateM = "Node_GateM";
	const FName Node_ExitM = "Node_ExitM";
	const FName Node_StepM = "Node_StepM";
	const FName Node_AlternativeM = "Node_AlternativeM";
	const FName Node_ReplicaM = "Node_ReplicaM";
	const FName Node_DialogM = "Node_DialogM";

	const FName Predicate_CompositeM = "P_CompositeM";
}

UENUM(BlueprintType)
enum class ESCompareType : uint8
{
	UNSET = 0,
	LESS = 1,
	LESS_OR_EQUAL = 2,
	EQUAL = 3,
	EQUAL_OR_GREATER = 4,
	GREATER = 5,
};

UENUM(BlueprintType)
enum class ESCompositionType : uint8
{
	UNSET = 0,
	AND = 1,
	OR = 2,
	XOR = 3,
};

UENUM(BlueprintType)
enum class ESExecutionMode : uint8
{
	UNSET = 0,
	ON_ENTER = 1,
	ON_LEAVE = 2,
};

UENUM(BlueprintType)
enum class ESGender : uint8
{
	UNSET = 0,
	MALE = 1,
	FEMALE = 2
};

//------------------------------------------------------------------------
// FCharacterM
//------------------------------------------------------------------------

USTRUCT(BlueprintType)
struct STORYLINEIMPORTER_API FCharacterM : public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

public:

	inline static const FName PlayerId = "PLAYER";

	FCharacterM()
	{
		Reset();
	}

	void Reset(const FCharacterM& character)
	{
		Id = character.Id;
		ActorClass = character.ActorClass;
		Name = character.Name;
		RtDescription = character.RtDescription;
		RtDescriptionFemale = character.RtDescriptionFemale;
		InitialRelation = character.InitialRelation;
		InitialRelationFemale = character.InitialRelationFemale;
		HasDescriptionFemale = character.HasDescriptionFemale;
	}

	void Reset()
	{
		Id = NAME_None;
		ActorClass = nullptr;
		Name = FText::GetEmpty();
		RtDescription = FText::GetEmpty();
		RtDescriptionFemale = FText::GetEmpty();
		InitialRelation = 0;
		InitialRelationFemale = 0;
		HasDescriptionFemale = 0;
	}

	UPROPERTY(BlueprintReadOnly, Category = "CharacterM")
		FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 1), Category = "CharacterM")
		TSoftClassPtr<AActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 2), Category = "CharacterM")
		FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 3), Category = "CharacterM")
		FText RtDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 5, EditCondition = "HasDescriptionFemale", EditConditionHides), Category = "CharacterM")
		FText RtDescriptionFemale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 6), Category = "CharacterM")
		float InitialRelation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 7), Category = "CharacterM")
		float InitialRelationFemale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 4), Category = "CharacterM")
		uint8 HasDescriptionFemale : 1;
};

//------------------------------------------------------------------------
// FDialogM
//------------------------------------------------------------------------

USTRUCT(BlueprintType)
struct STORYLINEIMPORTER_API FDialogM : public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

public:

	FDialogM()
	{
		Reset();
	}

	void Reset(const FDialogM& dialog)
	{
		Id = dialog.Id;
		LocationId = dialog.LocationId;
		NpcId = dialog.NpcId;
		Name = dialog.Name;
		Description = dialog.Description;
		NodeIds = dialog.NodeIds;
	}

	void Reset()
	{
		Id = NAME_None;
		LocationId = NAME_None;
		NpcId = NAME_None;
		Name = FText::GetEmpty();
		Description = FText::GetEmpty();
		NodeIds.Empty();
	}

	UPROPERTY(BlueprintReadOnly, Category = "DialogM")
		FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 1), Category = "DialogM")
		FName LocationId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 2), Category = "DialogM")
		FName NpcId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 3), Category = "DialogM")
		FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 4), Category = "DialogM")
		FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 5), Category = "DialogM")
		TSet<FName> NodeIds;
};

//------------------------------------------------------------------------
// FGameEventM
//------------------------------------------------------------------------

USTRUCT(BlueprintType)
struct STORYLINEIMPORTER_API FGameEventM : public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

public:

	FGameEventM()
	{
		Id = NAME_None;

		TypeAttribute = NAME_None;
		ExecutionMode = ESExecutionMode::UNSET;

		IdParam1 = NAME_None;
		IdParam2 = NAME_None;
		StringParam = "";
		FloatParam = 0;
	}

	void Reset(const FGameEventM& gameEvent)
	{
		Id = gameEvent.Id;

		TypeAttribute = gameEvent.TypeAttribute;
		ExecutionMode = gameEvent.ExecutionMode;

		IdParam1 = gameEvent.IdParam1;
		IdParam2 = gameEvent.IdParam2;
		StringParam = gameEvent.StringParam;
		FloatParam = gameEvent.FloatParam;
	}

	UPROPERTY(BlueprintReadOnly, Category = "GameEventM")
		FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 1), Category = "GameEventM")
		FName TypeAttribute;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 2), Category = "GameEventM")
		ESExecutionMode ExecutionMode;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 3), Category = "GameEventM")
		FName IdParam1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 4), Category = "GameEventM")
		FName IdParam2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 5), Category = "GameEventM")
		FString StringParam;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 6), Category = "GameEventM")
		float FloatParam;
};

//------------------------------------------------------------------------
// UGE_Base
//------------------------------------------------------------------------

UCLASS(BlueprintType, Blueprintable, Abstract)
class STORYLINEIMPORTER_API UGE_Base : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Game Event")
		void Execute(const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FGameEventM& gameEvent) const;

	virtual void Execute_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FGameEventM& gameEvent) const { check(0); }
};

//------------------------------------------------------------------------
// FGameEventMImpl
//------------------------------------------------------------------------

USTRUCT(BlueprintType)
struct STORYLINEIMPORTER_API FGameEventMImpl : public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

public:

	FGameEventMImpl()
	{
		GameEventClass = nullptr;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameEventMImpl")
		TSubclassOf<UGE_Base> GameEventClass;
};

//------------------------------------------------------------------------
// FItemM
//------------------------------------------------------------------------

USTRUCT(BlueprintType)
struct STORYLINEIMPORTER_API FItemM : public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

public:

	FItemM()
	{
		Id = NAME_None;
		ActorClass = nullptr;
		Name = FText::GetEmpty();
		RtDescription = FText::GetEmpty();
		RtDescriptionFemale = FText::GetEmpty();
		RtInternalDescription = FText::GetEmpty();
		RtInternalDescriptionFemale = FText::GetEmpty();
		HasDescriptionFemale = 0;
		HasInternalDescription = 0;
		HasInternalDescriptionFemale = 0;
	}

	void Reset(const FItemM& item)
	{
		Id = item.Id;
		ActorClass = item.ActorClass;
		Name = item.Name;
		RtDescription = item.RtDescription;
		RtDescriptionFemale = item.RtDescriptionFemale;
		RtInternalDescription = item.RtInternalDescription;
		RtInternalDescriptionFemale = item.RtInternalDescriptionFemale;
		HasDescriptionFemale = item.HasDescriptionFemale;
		HasInternalDescription = item.HasInternalDescription;
		HasInternalDescriptionFemale = item.HasInternalDescriptionFemale;
	}

	UPROPERTY(BlueprintReadOnly, Category = "ItemM")
		FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 1), Category = "ItemM")
		TSoftClassPtr<AActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 2), Category = "ItemM")
		FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 3), Category = "ItemM")
		FText RtDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 5, EditCondition = "HasDescriptionFemale", EditConditionHides), Category = "ItemM")
		FText RtDescriptionFemale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 7, EditCondition = "HasInternalDescription", EditConditionHides), Category = "ItemM")
		FText RtInternalDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 9, EditCondition = "HasInternalDescription && HasInternalDescriptionFemale", EditConditionHides), Category = "ItemM")
		FText RtInternalDescriptionFemale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 4), Category = "ItemM")
		uint8 HasDescriptionFemale : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 6), Category = "ItemM")
		uint8 HasInternalDescription : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 8, EditCondition = "HasInternalDescription", EditConditionHides), Category = "ItemM")
		uint8 HasInternalDescriptionFemale : 1;
};

//------------------------------------------------------------------------
// FNodeM
//------------------------------------------------------------------------

USTRUCT(BlueprintType)
struct STORYLINEIMPORTER_API FNodeM : public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

public:

	FNodeM()
	{
		Reset();
	}

	void Reset(const FNodeM& node)
	{
		Id = node.Id;

		SoundAsset = node.SoundAsset;
		TypeAttribute = node.TypeAttribute;

		Gender = node.Gender;
		RtDescription = node.RtDescription;

		Result = node.Result;

		CharacterId = node.CharacterId;
		OverrideName = node.OverrideName;
		ShortDescription = node.ShortDescription;

		ChildNodeIds = node.ChildNodeIds;
		PredicateIds = node.PredicateIds;
		GameEventIds = node.GameEventIds;
	}

	void Reset()
	{
		Id = NAME_None;

		SoundAsset = nullptr;
		TypeAttribute = NAME_None;

		Gender = ESGender::UNSET;
		RtDescription = FText::GetEmpty();

		Result = FText::GetEmpty();

		CharacterId = NAME_None;
		OverrideName = FText::GetEmpty();
		ShortDescription = FText::GetEmpty();

		ChildNodeIds.Empty();
		PredicateIds.Empty();
		GameEventIds.Empty();
	}

	UPROPERTY(BlueprintReadOnly, Category = "NodeM")
		FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 1), Category = "NodeM")
		TSoftObjectPtr<USoundBase> SoundAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 2), Category = "NodeM")
		FName TypeAttribute;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 3), Category = "NodeM")
		ESGender Gender;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 4), Category = "NodeM")
		FText RtDescription;

	// Journal

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 5), Category = "NodeM")
		FText Result;

	// Regular

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 6), Category = "NodeM")
		FName CharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 7), Category = "NodeM")
		FText OverrideName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 8), Category = "NodeM")
		FText ShortDescription;

	// Other

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 9), Category = "NodeM")
		TSet<FName> ChildNodeIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 10), Category = "NodeM")
		TSet<FName> PredicateIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 11), Category = "NodeM")
		TSet<FName> GameEventIds;
};

//------------------------------------------------------------------------
// FPredicateM
//------------------------------------------------------------------------

USTRUCT(BlueprintType)
struct STORYLINEIMPORTER_API FPredicateM : public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

public:

	FPredicateM()
	{
		Id = NAME_None;

		TypeAttribute = NAME_None;

		IdParam1 = NAME_None;
		IdParam2 = NAME_None;
		StringParam = "";
		FloatParam = 0;
		IntParam = 0;

		CompareType = ESCompareType::UNSET;
		CompositionType = ESCompositionType::UNSET;

		IsInversed = 0;
	}

	void Reset(const FPredicateM& predicate)
	{
		Id = predicate.Id;

		TypeAttribute = predicate.TypeAttribute;

		IdParam1 = predicate.IdParam1;
		IdParam2 = predicate.IdParam2;
		StringParam = predicate.StringParam;
		FloatParam = predicate.FloatParam;
		IntParam = predicate.IntParam;

		CompareType = predicate.CompareType;
		CompositionType = predicate.CompositionType;

		IsInversed = predicate.IsInversed;
	}

	UPROPERTY(BlueprintReadOnly, Category = "PredicateM")
		FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 1), Category = "PredicateM")
		FName TypeAttribute;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 3), Category = "PredicateM")
		FName IdParam1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 4), Category = "PredicateM")
		FName IdParam2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 5), Category = "PredicateM")
		FString StringParam;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 6), Category = "PredicateM")
		float FloatParam;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 7), Category = "PredicateM")
		int32 IntParam;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 8), Category = "PredicateM")
		ESCompareType CompareType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 9), Category = "PredicateM")
		ESCompositionType CompositionType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 2), Category = "PredicateM")
		uint8 IsInversed : 1;
};

//------------------------------------------------------------------------
// UP_Base
//------------------------------------------------------------------------

UCLASS(BlueprintType, Blueprintable, Abstract)
class STORYLINEIMPORTER_API UP_Base : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Predicate")
		bool Execute(const TScriptInterface<IStorylineSource>& storylineSource, const TScriptInterface<IStorylineContext>& storylineContext, const FPredicateM& gameEvent) const;

	virtual bool Execute_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, const TScriptInterface<IStorylineContext>& storylineContext, const FPredicateM& gameEvent) const { check(0); return false; }
};

//------------------------------------------------------------------------
// FPredicateMImpl
//------------------------------------------------------------------------

USTRUCT(BlueprintType)
struct STORYLINEIMPORTER_API FPredicateMImpl : public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

public:

	FPredicateMImpl()
	{
		PredicateClass = nullptr;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PredicateMImpl")
		TSubclassOf<UP_Base> PredicateClass;
};

//------------------------------------------------------------------------
// FQuestM
//------------------------------------------------------------------------

USTRUCT(BlueprintType)
struct STORYLINEIMPORTER_API FQuestM : public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

public:

	FQuestM()
	{
		Id = NAME_None;
		Name = FText::GetEmpty();
		Description = FText::GetEmpty();
		NodeIds.Empty();
	}

	void Reset(const FQuestM& quest)
	{
		Id = quest.Id;
		Name = quest.Name;
		Description = quest.Description;
		NodeIds = quest.NodeIds;
	}

	UPROPERTY(BlueprintReadOnly, Category = "QuestM")
		FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 1), Category = "QuestM")
		FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 2), Category = "QuestM")
		FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 3), Category = "QuestM")
		TSet<FName> NodeIds;
};

//------------------------------------------------------------------------
// FNodePath
//------------------------------------------------------------------------

USTRUCT(BlueprintType)
struct FNodePath
{
	GENERATED_USTRUCT_BODY();

public:

	FNodePath()
	{
		Reset();
	}

	void Reset()
	{
		StartNodeId = NAME_None;
		NodeIds.Reset();
		TargetNodeId = NAME_None;

		LastNodeId = NAME_None;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 1), Category = "NodePath")
		FName StartNodeId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 1), Category = "NodePath")
		TSet<FName> NodeIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayPriority = 1), Category = "NodePath")
		FName TargetNodeId;

	FName LastNodeId;
};

//------------------------------------------------------------------------
// UStorylineSource
//------------------------------------------------------------------------

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UStorylineSource : public UInterface
{
	GENERATED_BODY()
};

class STORYLINEIMPORTER_API IStorylineSource
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineSource")
		UDataTable* GetCharacters() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineSource")
		UDataTable* GetDialogs() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineSource")
		UDataTable* GetGameEvents() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineSource")
		UDataTable* GetGameEventImpls() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineSource")
		UDataTable* GetItems() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineSource")
		UDataTable* GetNodes() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineSource")
		UDataTable* GetPredicates() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineSource")
		UDataTable* GetPredicateImpls() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineSource")
		UDataTable* GetQuests() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineSource")
		UDataTable* GetReplicas() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineSource")
		UDataTable* GetRichTexts() const;
};

//------------------------------------------------------------------------
// UStorylineContext
//------------------------------------------------------------------------
// 
// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UStorylineContext : public UInterface
{
	GENERATED_BODY()
};

class STORYLINEIMPORTER_API IStorylineContext
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		ESGender GetPlayerGender() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		void OnStartDialog(const FDialogM& dialog);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		void OnEndDialog();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		void OnEnterNode(const FNodeM& node);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		void OnLeaveNode(bool IsBeforePlayerChoice);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		void OnPlayNode();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		void OnPauseNode();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		void OnResumeNode();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		void OnSkipNode();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		void GetNodePaths(TArray<FNodePath>& outNodePaths) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		void SetNodePaths(const TScriptInterface<IStorylineSource>& storylineSource, const TArray<FNodePath>& nodePaths);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		void OnPlayerChoice();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		bool HasDialogNodeInPrevSessions(FName nodeId) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		bool HasItem(TSubclassOf<AActor> actorClass) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		bool HasQuestNode(FName nodeId) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		int32 DropItem(TSubclassOf<AActor> actorClass);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		int32 PickUpItem(TSubclassOf<AActor> actorClass);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		void AddQuestNode(FName questId, FName nodeId);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StorylineContext")
		void PassQuestNode(FName questId, FName nodeId);
};

//------------------------------------------------------------------------
// UStorylineServiceBFL
//------------------------------------------------------------------------

UCLASS(meta = (ScriptName = "StorylineServiceLibrary"))
class STORYLINEIMPORTER_API UStorylineServiceBFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "StorylineService")
		static bool GetCharacterM(const TScriptInterface<IStorylineSource>& storylineSource, const FName id, FCharacterM& outCharacter);

	UFUNCTION(BlueprintCallable, Category = "StorylineService")
		static bool GetCharacterMForActorClass(const TScriptInterface<IStorylineSource>& storylineSource, const UClass* actorClass, FCharacterM& outCharacter);

	UFUNCTION(BlueprintCallable, Category = "StorylineService")
		static bool GetItemM(const TScriptInterface<IStorylineSource>& storylineSource, const FName id, FItemM& outItem);

	UFUNCTION(BlueprintCallable, Category = "StorylineService")
		static bool GetQuestM(const TScriptInterface<IStorylineSource>& storylineSource, const FName id, FQuestM& outQuest);

	UFUNCTION(BlueprintCallable, Category = "StorylineService")
		static bool GetDialogM(const TScriptInterface<IStorylineSource>& storylineSource, const FName id, FDialogM& outDialog);

	UFUNCTION(BlueprintCallable, Category = "StorylineService")
		static TArray<FDialogM> GetDialogMsForCharacter(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FName characterId, bool onlyAvailable);

	UFUNCTION(BlueprintCallable, Category = "StorylineService")
		static bool GetReplicaM(const TScriptInterface<IStorylineSource>& storylineSource, const FName id, FDialogM& outReplica);

	UFUNCTION(BlueprintCallable, Category = "StorylineService")
		static bool GetNodeM(const TScriptInterface<IStorylineSource>& storylineSource, const FName id, FNodeM& outNode);

	UFUNCTION(BlueprintCallable, Category = "StorylineService")
		static bool GetGameEventM(const TScriptInterface<IStorylineSource>& storylineSource, const FName id, FGameEventM& outGameEvent);

	UFUNCTION(BlueprintCallable, Category = "StorylineService")
		static TSubclassOf<UGE_Base> GetGameEventMImpl(const TScriptInterface<IStorylineSource>& storylineSource, const FName gameEventType);

	UFUNCTION(BlueprintCallable, Category = "StorylineService")
		static bool GetPredicateM(const TScriptInterface<IStorylineSource>& storylineSource, const FName id, FPredicateM& outPredicate);

	UFUNCTION(BlueprintCallable, Category = "StorylineService")
		static TSubclassOf<UP_Base> GetPredicateMImpl(const TScriptInterface<IStorylineSource>& storylineSource, const FName predicateType);

	UFUNCTION(BlueprintCallable, Category = "StorylineService: Dialogs")
		static void GetNextNodePaths(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FName startNodeId, TArray<FNodePath>& outNodePaths, bool& outIsPlayerChoice);

	UFUNCTION(BlueprintCallable, Category = "StorylineService: Dialogs")
		static void EnterNode(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FName nodeId);

	UFUNCTION(BlueprintCallable, Category = "StorylineService: Dialogs")
		static void LeaveNode(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FName nodeId);

	static const FCharacterM* GetCharacterMPtr(const TScriptInterface<IStorylineSource>& storylineSource, const FName id);

	static const FCharacterM* GetCharacterMPtr(const TScriptInterface<IStorylineSource>& storylineSource, const UClass* actorClass);

	static const FItemM* GetItemMPtr(const TScriptInterface<IStorylineSource>& storylineSource, const FName id);

	static const FQuestM* GetQuestMPtr(const TScriptInterface<IStorylineSource>& storylineSource, const FName id);

	static const FDialogM* GetDialogMPtr(const TScriptInterface<IStorylineSource>& storylineSource, const FName id);

	static TArray<const FDialogM*> GetDialogMPtrsForCharacter(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FName characterId, bool onlyAvailable);

	static const FDialogM* GetReplicaMPtr(const TScriptInterface<IStorylineSource>& storylineSource, const FName id);

	static const FNodeM* GetNodeMPtr(const TScriptInterface<IStorylineSource>& storylineSource, const FName id);

	static const FGameEventM* GetGameEventMPtr(const TScriptInterface<IStorylineSource>& storylineSource, const FName id);

	static const FPredicateM* GetPredicateMPtr(const TScriptInterface<IStorylineSource>& storylineSource, const FName id);

	static void EnterNodePtr(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FNodeM* node);

	static void LeaveNodePtr(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FNodeM* node);

	static const FNodeM* GetNextNodePtr(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, int32 nextNodeIndex);

	static void ProcessNodeEvents(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FNodeM* node, ESExecutionMode executionMode);

	static void ProcessNodePathEvents(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FNodePath& nodePath);

	static bool IsAvailableDialog(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FDialogM* dialog);

	static bool IsAvailableNode(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FNodeM* node);
};