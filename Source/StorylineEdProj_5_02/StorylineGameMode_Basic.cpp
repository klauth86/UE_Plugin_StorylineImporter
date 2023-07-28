// Copyright Epic Games, Inc. All Rights Reserved.

#include "StorylineGameMode_Basic.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

//--------------------------------------------------------------------------
// AStorylineContext_Basic
//--------------------------------------------------------------------------

AStorylineContext_Basic::AStorylineContext_Basic(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DefaultNodeDuration = 2;
}

ESGender AStorylineContext_Basic::GetPlayerGender_Implementation() const { return PlayerGender; }

void AStorylineContext_Basic::OnStartDialog_Implementation(const FDialogM& dialog)
{
	if (AStorylineGameMode_Basic* gameMode = Cast<AStorylineGameMode_Basic>(UGameplayStatics::GetGameMode(this)))
	{
		gameMode->SetActiveDialog(dialog.Id);
	}

	BIE_OnStartDialog();

	ActiveDialog.Reset(dialog);
	CurrentDialog.Reset(dialog);
}

void AStorylineContext_Basic::OnEndDialog_Implementation()
{
	if (AudioComponent)
	{
		AudioComponent->OnAudioPlayStateChangedNative.RemoveAll(this);
		AudioComponent->DestroyComponent();
		AudioComponent = nullptr;
	}

	if (TimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}

	CurrentDialog.Reset();
	ActiveDialog.Reset();

	BIE_OnEndDialog();

	if (AStorylineGameMode_Basic* gameMode = Cast<AStorylineGameMode_Basic>(UGameplayStatics::GetGameMode(this)))
	{
		gameMode->SetActiveDialog(NAME_None);
	}
}

void AStorylineContext_Basic::OnEnterNode_Implementation(const FNodeM& node)
{
	if (AStorylineGameMode_Basic* gameMode = Cast<AStorylineGameMode_Basic>(UGameplayStatics::GetGameMode(this)))
	{
		gameMode->AddDialogNode(node.Id);
	}

	CurrentNode.Reset(node);

	if (node.TypeAttribute == StorylineTypeAttributes::Node_ReplicaM ||
		node.TypeAttribute == StorylineTypeAttributes::Node_DialogM)
	{
		AStorylineGameMode_Basic* gameMode = Cast<AStorylineGameMode_Basic>(UGameplayStatics::GetGameMode(this));

		TScriptInterface<IStorylineSource> storylineSource;
		storylineSource.SetObject(gameMode->GetStorylineSource()->_getUObject());
		storylineSource.SetInterface(gameMode->GetStorylineSource());

		if (const FCharacterM* character = UStorylineServiceBFL::GetCharacterMPtr(storylineSource, node.CharacterId))
		{
			SpeakingCharacter.Reset(*character);
			SpeakingActor = UGameplayStatics::GetActorOfClass(this, character->ActorClass.Get());
		}

		BIE_OnEnterNode();
	}
}

void AStorylineContext_Basic::OnLeaveNode_Implementation(bool IsBeforePlayerChoice)
{
	if (CurrentNode.TypeAttribute == StorylineTypeAttributes::Node_ReplicaM ||
		CurrentNode.TypeAttribute == StorylineTypeAttributes::Node_DialogM)
	{
		BIE_OnLeaveNode(IsBeforePlayerChoice);
	}

	SpeakingActor = nullptr;
	SpeakingCharacter.Reset();

	CurrentNode.Reset();
}

void AStorylineContext_Basic::OnPlayNode_Implementation()
{
	UUID = 0;

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &AStorylineContext_Basic::OnTimerComplete), DefaultNodeDuration, false);

	if (!CurrentNode.SoundAsset.IsNull())
	{
		UUID = static_cast<int32>(FDateTime::Now().GetTicks());

		UKismetSystemLibrary::FOnAssetLoaded onAssetLoaded;
		onAssetLoaded.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(AStorylineContext_Basic, OnSoundAssetLoaded));

		FLatentActionInfo latentActionInfo;
		latentActionInfo.Linkage = 0;
		latentActionInfo.UUID = UUID;
		latentActionInfo.CallbackTarget = this;
		UKismetSystemLibrary::LoadAsset(this, CurrentNode.SoundAsset, onAssetLoaded, latentActionInfo);
	}
}

void AStorylineContext_Basic::OnPauseNode_Implementation() { if (AudioComponent) AudioComponent->SetPaused(true); }

void AStorylineContext_Basic::OnResumeNode_Implementation() { if (AudioComponent) AudioComponent->SetPaused(false); }

void AStorylineContext_Basic::OnSkipNode_Implementation() { if (AudioComponent) AudioComponent->Stop(); }

void AStorylineContext_Basic::GetNodePaths_Implementation(TArray<FNodePath>& outNodePaths) const { outNodePaths = NodePaths; }

void AStorylineContext_Basic::SetNodePaths_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, const TArray<FNodePath>& nodePaths)
{
	NodePaths = nodePaths;

	PlayerChoices.Reset(NodePaths.Num());

	for (const FNodePath& nodePath : NodePaths)
	{
		const FNodeM* node = UStorylineServiceBFL::GetNodeMPtr(storylineSource, nodePath.TargetNodeId);
		check(node); // Must be ok in all cases if data is consistent

		const int32 emplacedIndex = PlayerChoices.Emplace();
		PlayerChoices[emplacedIndex].Reset(*node);
	}
}

void AStorylineContext_Basic::StartDialogWith(AActor* actor)
{
	if (actor)
	{
		AStorylineGameMode_Basic* gameMode = Cast<AStorylineGameMode_Basic>(UGameplayStatics::GetGameMode(this));

		TScriptInterface<IStorylineSource> storylineSource;
		storylineSource.SetObject(gameMode->GetStorylineSource()->_getUObject());
		storylineSource.SetInterface(gameMode->GetStorylineSource());

		if (const FCharacterM* character = UStorylineServiceBFL::GetCharacterMPtr(storylineSource, actor->GetClass()))
		{
			TScriptInterface<IStorylineContext> storylineContext;
			storylineContext.SetObject(this);
			storylineContext.SetInterface(this);

			TArray<const FDialogM*> dialogMPtrs = UStorylineServiceBFL::GetDialogMPtrsForCharacter(this, storylineSource, storylineContext, character->Id, true);

			if (dialogMPtrs.Num() == 1 && dialogMPtrs[0]->NodeIds.Num() == 1) // At the moment no support for multiple root dialogs
			{
				Execute_OnStartDialog(this, *dialogMPtrs[0]);

				const FNodeM* node = UStorylineServiceBFL::GetNodeMPtr(storylineSource, *dialogMPtrs[0]->NodeIds.begin());
				UStorylineServiceBFL::EnterNodePtr(this, storylineSource, storylineContext, node);
			}
		}
	}
}

void AStorylineContext_Basic::SelectPlayerChoice(int32 nextNodeIndex)
{
	AStorylineGameMode_Basic* gameMode = Cast<AStorylineGameMode_Basic>(UGameplayStatics::GetGameMode(this));

	TScriptInterface<IStorylineSource> storylineSource;
	storylineSource.SetObject(gameMode->GetStorylineSource()->_getUObject());
	storylineSource.SetInterface(gameMode->GetStorylineSource());

	TScriptInterface<IStorylineContext> storylineContext;
	storylineContext.SetObject(this);
	storylineContext.SetInterface(this);

	if (const FNodeM* nextNode = UStorylineServiceBFL::GetNextNodePtr(this, storylineSource, storylineContext, nextNodeIndex))
	{
		UStorylineServiceBFL::EnterNodePtr(this, storylineSource, storylineContext, nextNode);
	}
}

void AStorylineContext_Basic::OnSoundAssetLoaded(UObject* loadedObject)
{
	if (USoundBase* soundBase = Cast<USoundBase>(loadedObject))
	{
		const FVector soundLocation = SpeakingActor.Get() ? SpeakingActor->GetActorLocation() : FVector::ZeroVector;

		if (!AudioComponent)
		{
			AudioComponent = UGameplayStatics::SpawnSoundAtLocation(this, soundBase, soundLocation,
				FRotator::ZeroRotator, 1, 1, 0, nullptr, nullptr, false);

			AudioComponent->OnAudioPlayStateChangedNative.AddUObject(this, &AStorylineContext_Basic::OnAudioPlayStateChangedNative);
		}
		else
		{
			AudioComponent->SetWorldLocation(soundLocation);
			AudioComponent->SetSound(soundBase);
		}
	}
}

void AStorylineContext_Basic::OnAudioPlayStateChangedNative(const UAudioComponent* InAudioComponent, EAudioComponentPlayState NewPlayState)
{
	if (NewPlayState == EAudioComponentPlayState::Stopped)
	{
		if (TimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		}

		AStorylineGameMode_Basic* gameMode = Cast<AStorylineGameMode_Basic>(UGameplayStatics::GetGameMode(this));

		TScriptInterface<IStorylineSource> storylineSource;
		storylineSource.SetObject(gameMode->GetStorylineSource()->_getUObject());
		storylineSource.SetInterface(gameMode->GetStorylineSource());

		TScriptInterface<IStorylineContext> storylineContext;
		storylineContext.SetObject(this);
		storylineContext.SetInterface(this);

		UStorylineServiceBFL::LeaveNodePtr(this, storylineSource, storylineContext, &CurrentNode);
	}
}

void AStorylineContext_Basic::OnTimerComplete()
{
	if (UUID == 0)
	{
		AStorylineGameMode_Basic* gameMode = Cast<AStorylineGameMode_Basic>(UGameplayStatics::GetGameMode(this));

		TScriptInterface<IStorylineSource> storylineSource;
		storylineSource.SetObject(gameMode->GetStorylineSource()->_getUObject());
		storylineSource.SetInterface(gameMode->GetStorylineSource());

		TScriptInterface<IStorylineContext> storylineContext;
		storylineContext.SetObject(this);
		storylineContext.SetInterface(this);

		UStorylineServiceBFL::LeaveNodePtr(this, storylineSource, storylineContext, &CurrentNode);
	}
}

void AStorylineContext_Basic::Reset()
{
	ActiveDialog.Reset();
	CurrentDialog.Reset();
	CurrentNode.Reset();
}

//--------------------------------------------------------------------------
// AStorylineGameMode_Basic
//--------------------------------------------------------------------------

AStorylineGameMode_Basic::AStorylineGameMode_Basic(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	StorylineSource = nullptr;
	StorylineContext = nullptr;
	Inventory.Reset();
	DialogNodes.Reset();
	Quests.Reset();
	QuestNodes.Reset();
	QuestEntries.Reset();
	CurrentDialogId = NAME_None;
}

void AStorylineGameMode_Basic::BeginPlay()
{
	Super::BeginPlay();

	StorylineSource = Cast<AStorylineSource_Basic>(UGameplayStatics::GetActorOfClass(this, AStorylineSource_Basic::StaticClass()));
	StorylineContext = Cast<AStorylineContext_Basic>(UGameplayStatics::GetActorOfClass(this, AStorylineContext_Basic::StaticClass()));
}

void AStorylineGameMode_Basic::AddQuestNode(FName questId, FName nodeId)
{
	bool questIsAlreadyInSet;
	Quests.FindOrAdd(questId, &questIsAlreadyInSet);
	QuestNodes.FindOrAdd(nodeId) = false;

	TScriptInterface<IStorylineSource> storylineSource;
	storylineSource.SetObject(StorylineSource->_getUObject());
	storylineSource.SetInterface(StorylineSource);

	TScriptInterface<IStorylineContext> storylineContext;
	storylineContext.SetObject(StorylineContext->_getUObject());
	storylineContext.SetInterface(StorylineContext);

	const FNodeM& node = *UStorylineServiceBFL::GetNodeMPtr(storylineSource, nodeId);

	const bool isAlternative = node.TypeAttribute == StorylineTypeAttributes::Node_AlternativeM;

	if (questIsAlreadyInSet)
	{
		FQuestStepEntry& questNodeEntry = QuestEntries.Last();

		if (isAlternative)
		{
			if (questNodeEntry.StepNode.ChildNodeIds.Contains(nodeId))
			{
				FQuestAlternativeEntry questAlternativeEntry;
				questAlternativeEntry.AlternativeNode.Reset(node);
				questNodeEntry.Alternatives.FindOrAdd(questAlternativeEntry);
			}
		}
		else
		{
			if (!QuestEntries.ContainsByPredicate([nodeId](const FQuestStepEntry& questStepEntry) { return questStepEntry.StepNode.Id == nodeId; }))
			{
				const int32 emplacedIndex = QuestEntries.Emplace();
				QuestEntries[emplacedIndex].StepNode.Reset(node);
			}
		}
	}
	else
	{
		check(!isAlternative); // Quest first node can not be Alternative!

		const int32 emplacedIndex = QuestEntries.Emplace();
		QuestEntries[emplacedIndex].StepNode.Reset(node);
	}

	BIE_OnQuestChanged();
}

void AStorylineGameMode_Basic::PassQuestNode(FName questId, FName nodeId)
{
	if (QuestNodes.Contains(nodeId))
	{
		QuestNodes[nodeId] = true;

		bool isFound = false;

		for (FQuestStepEntry& questStepEntry : QuestEntries)
		{
			if (questStepEntry.StepNode.Id == nodeId)
			{
				questStepEntry.bIsPassed = true;
				isFound = true;
			}

			for (FQuestAlternativeEntry& questAlternativeEntry : questStepEntry.Alternatives)
			{
				if (questStepEntry.StepNode.Id == nodeId)
				{
					questStepEntry.bIsPassed = true;
					isFound = true;
					break;
				}
			}

			if (isFound) break;
		}

		if (isFound) BIE_OnQuestChanged();
	}
}

//------------------------------------------------------------------------
// ADialogCharacter
//------------------------------------------------------------------------

ADialogCharacter::ADialogCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
	SphereComponent->SetSphereRadius(200);
	SphereComponent->SetGenerateOverlapEvents(true);
	SphereComponent->SetCollisionProfileName("OverlapAll");

	SphereComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	PrimaryActorTick.bCanEverTick = true;
}

void ADialogCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IInteractionSource* interactionSource = Cast<IInteractionSource>(InteractionActor))
	{
		if (interactionSource->GetInteractionStatus() == EInteractionStatus::UNSET)
		{
			InteractionActor = nullptr;
			
			BIE_OnEndInteraction();

			BIE_OnEnterInteraction();

			if (AActor* interactionActor = PendingInteractionActor)
			{
				PendingInteractionActor = nullptr;
				SetInteractionActor_Internal(interactionActor);
			}
		}
	}
}

void ADialogCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (IsPlayerControlled())
	{
		if (OtherActor == this) return;

		if (ADialogCharacter* dialogCharacter = Cast<ADialogCharacter>(OtherActor))
		{
			BIE_OnEnterInteraction();
		}
		else if (AInventoryActor* inventoryActor = Cast<AInventoryActor>(OtherActor))
		{
			if (AStorylineGameMode_Basic* gameMode = Cast<AStorylineGameMode_Basic>(UGameplayStatics::GetGameMode(this)))
			{
				gameMode->PickUpItem(OtherActor->GetClass());
				OtherActor->Destroy();
			}
		}
	}
}

void ADialogCharacter::NotifyActorEndOverlap(AActor* OtherActor)
{
	if (IsPlayerControlled())
	{
		if (OtherActor == this) return;

		if (ADialogCharacter* dialogCharacter = Cast<ADialogCharacter>(OtherActor))
		{
			BIE_OnLeaveInteraction();
		}
	}
}

void ADialogCharacter::StartInteraction()
{
	if (AStorylineContext_Basic* storylineContext = Cast<AStorylineContext_Basic>(UGameplayStatics::GetActorOfClass(this, AStorylineContext_Basic::StaticClass())))
	{
		storylineContext->StartDialogWith(this);
	}
}

EInteractionStatus ADialogCharacter::GetInteractionStatus() const
{
	if (AStorylineGameMode_Basic* gameMode = Cast<AStorylineGameMode_Basic>(UGameplayStatics::GetGameMode(this)))
	{
		return gameMode->HasActiveDialog() ? EInteractionStatus::LOOP : EInteractionStatus::UNSET;
	}

	return EInteractionStatus::UNSET;
}

void ADialogCharacter::Talk()
{
	TArray<AActor*> overlappingActors;
	GetOverlappingActors(overlappingActors);

	for (AActor* overlappingActor : overlappingActors)
	{
		if (ADialogCharacter* dialogCharacter = Cast<ADialogCharacter>(overlappingActor))
		{
			SetInteractionActor(dialogCharacter);
			break;
		}
	}
}

void ADialogCharacter::SetInteractionActor(AActor* interactionActor)
{
	if (InteractionActor != interactionActor)
	{
		if (InteractionActor)
		{
			PendingInteractionActor = interactionActor;

			if (IInteractionSource* interactionSource = Cast<IInteractionSource>(InteractionActor))
			{
				interactionSource->EndInteraction();
			}
		}
		else
		{
			SetInteractionActor_Internal(interactionActor);
		}
	}
}

void ADialogCharacter::SetInteractionActor_Internal(AActor* interactionActor)
{
	BIE_OnLeaveInteraction();

	BIE_OnStartInteraction();

	InteractionActor = interactionActor;

	if (IInteractionSource* interactionSource = Cast<IInteractionSource>(interactionActor))
	{
		interactionSource->StartInteraction();
	}
}

//------------------------------------------------------------------------
// AInventoryActor
//------------------------------------------------------------------------

AInventoryActor::AInventoryActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
	SphereComponent->SetSphereRadius(200);
	SphereComponent->SetGenerateOverlapEvents(true);
	SphereComponent->SetCollisionProfileName("OverlapAll");

	SphereComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

//------------------------------------------------------------------------
// UP_Dialog_Node_Has_PrevSessionsM
//------------------------------------------------------------------------

bool UP_Dialog_Node_Has_PrevSessionsM::Execute_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, const TScriptInterface<IStorylineContext>& storylineContext, const FPredicateM& predicate) const
{
	bool result = false;

	if (AStorylineGameMode_Basic* gameMode = Cast<AStorylineGameMode_Basic>(UGameplayStatics::GetGameMode(storylineSource->_getUObject()->GetWorld())))
	{
		result = gameMode->HasDialogNodeInPrevSessions(predicate.IdParam2);
	}

	return predicate.IsInversed ? (!result) : result;
}

//------------------------------------------------------------------------
// UP_Item_HasM
//------------------------------------------------------------------------

bool UP_Item_HasM::Execute_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, const TScriptInterface<IStorylineContext>& storylineContext, const FPredicateM& predicate) const
{
	bool result = false;

	if (const FItemM* item = UStorylineServiceBFL::GetItemMPtr(storylineSource, predicate.IdParam1))
	{
		TSubclassOf<AActor> actorClass = item->ActorClass.LoadSynchronous();
		
		if (AStorylineGameMode_Basic* gameMode = Cast<AStorylineGameMode_Basic>(UGameplayStatics::GetGameMode(storylineSource->_getUObject()->GetWorld())))
		{
			result = gameMode->HasItem(actorClass);
		}
	}

	return predicate.IsInversed ? (!result) : result;
}

//------------------------------------------------------------------------
// UP_Quest_AddedM
//------------------------------------------------------------------------

bool UP_Quest_AddedM::Execute_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, const TScriptInterface<IStorylineContext>& storylineContext, const FPredicateM& predicate) const
{
	bool result = false;

	if (AStorylineGameMode_Basic* gameMode = Cast<AStorylineGameMode_Basic>(UGameplayStatics::GetGameMode(storylineSource->_getUObject()->GetWorld())))
	{
		result = gameMode->HasQuest(predicate.IdParam1);
	}

	return predicate.IsInversed ? (!result) : result;
}

//------------------------------------------------------------------------
// UP_Quest_Node_Passed
//------------------------------------------------------------------------

bool UP_Quest_Node_Passed::Execute_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, const TScriptInterface<IStorylineContext>& storylineContext, const FPredicateM& predicate) const
{
	bool result = false;
	
	if (AStorylineGameMode_Basic* gameMode = Cast<AStorylineGameMode_Basic>(UGameplayStatics::GetGameMode(storylineSource->_getUObject()->GetWorld())))
	{
		result = gameMode->HasPassedQuestNode(predicate.IdParam2);
	}

	return predicate.IsInversed ? (!result) : result;
}

//------------------------------------------------------------------------
// UGE_Item_DropM
//------------------------------------------------------------------------

void UGE_Item_DropM::Execute_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FGameEventM& gameEvent) const
{
	if (const FItemM* item = UStorylineServiceBFL::GetItemMPtr(storylineSource, gameEvent.IdParam1))
	{
		TSubclassOf<AActor> actorClass = item->ActorClass.LoadSynchronous();

		if (AStorylineGameMode_Basic* gameMode = Cast<AStorylineGameMode_Basic>(UGameplayStatics::GetGameMode(storylineSource->_getUObject()->GetWorld())))
		{
			gameMode->DropItem(actorClass);
		}
	}
}

//------------------------------------------------------------------------
// UGE_Quest_Node_AddM
//------------------------------------------------------------------------

void UGE_Quest_Node_AddM::Execute_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FGameEventM& gameEvent) const
{
	if (AStorylineGameMode_Basic* gameMode = Cast<AStorylineGameMode_Basic>(UGameplayStatics::GetGameMode(storylineSource->_getUObject()->GetWorld())))
	{
		gameMode->AddQuestNode(gameEvent.IdParam1, gameEvent.IdParam2);
	}
}

//------------------------------------------------------------------------
// UGE_Quest_Node_PassM
//------------------------------------------------------------------------

void UGE_Quest_Node_PassM::Execute_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FGameEventM& gameEvent) const
{
	if (AStorylineGameMode_Basic* gameMode = Cast<AStorylineGameMode_Basic>(UGameplayStatics::GetGameMode(storylineSource->_getUObject()->GetWorld())))
	{
		gameMode->PassQuestNode(gameEvent.IdParam1, gameEvent.IdParam2);
	}
}

//------------------------------------------------------------------------
// UGE_Quest_AddM
//------------------------------------------------------------------------

void UGE_Quest_AddM::Execute_Implementation(const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FGameEventM& gameEvent) const
{
	if (const FQuestM* quest = UStorylineServiceBFL::GetQuestMPtr(storylineSource, gameEvent.IdParam1))
	{
		if (AStorylineGameMode_Basic* gameMode = Cast<AStorylineGameMode_Basic>(UGameplayStatics::GetGameMode(storylineSource->_getUObject()->GetWorld())))
		{
			for (const FName nodeId : quest->NodeIds)
			{
				gameMode->AddQuestNode(gameEvent.IdParam1, nodeId);
			}
		}
	}
}