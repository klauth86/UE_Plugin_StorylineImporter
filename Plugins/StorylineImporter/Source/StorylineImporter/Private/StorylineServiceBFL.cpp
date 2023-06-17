// Copyright 2023 Pentangle Studio

#include "StorylineServiceBFL.h"

//--------------------------------------------------------------------------
// UStorylineServiceBFL
//--------------------------------------------------------------------------

bool UStorylineServiceBFL::GetCharacterM(const TScriptInterface<IStorylineSource>& storylineSource, const FName id, FCharacterM& outCharacter)
{
	if (const FCharacterM* character = GetCharacterMPtr(storylineSource, id))
	{
		outCharacter.Reset(*character);
		return true;
	}

	return false;
}

bool UStorylineServiceBFL::GetCharacterMForActorClass(const TScriptInterface<IStorylineSource>& storylineSource, const UClass* actorClass, FCharacterM& outCharacter)
{
	if (const FCharacterM* character = GetCharacterMPtr(storylineSource, actorClass))
	{
		outCharacter.Reset(*character);
		return true;
	}

	return false;
}

bool UStorylineServiceBFL::GetItemM(const TScriptInterface<IStorylineSource>& storylineSource, const FName id, FItemM& outItem)
{
	if (const FItemM* item = GetItemMPtr(storylineSource, id))
	{
		outItem.Reset(*item);
		return true;
	}

	return false;
}

bool UStorylineServiceBFL::GetQuestM(const TScriptInterface<IStorylineSource>& storylineSource, const FName id, FQuestM& outQuest)
{
	if (const FQuestM* quest = GetQuestMPtr(storylineSource, id))
	{
		outQuest.Reset(*quest);
		return true;
	}

	return false;
}

bool UStorylineServiceBFL::GetDialogM(const TScriptInterface<IStorylineSource>& storylineSource, const FName id, FDialogM& outDialog)
{
	if (const FDialogM* dialog = GetDialogMPtr(storylineSource, id))
	{
		outDialog.Reset(*dialog);
		return true;
	}

	return false;
}

TArray<FDialogM> UStorylineServiceBFL::GetDialogMsForCharacter(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FName characterId, bool onlyAvailable)
{
	TArray<const FDialogM*> dialogMPtrs = GetDialogMPtrsForCharacter(WorldContextObject, storylineSource, storylineContext, characterId, onlyAvailable);
	return TArray<FDialogM>(*dialogMPtrs.GetData(), dialogMPtrs.Num());
}

bool UStorylineServiceBFL::GetReplicaM(const TScriptInterface<IStorylineSource>& storylineSource, const FName id, FReplicaM& outReplica)
{
	if (const FReplicaM* replica = GetReplicaMPtr(storylineSource, id))
	{
		outReplica.Reset(*replica);
		return true;
	}

	return false;
}

bool UStorylineServiceBFL::GetNodeM(const TScriptInterface<IStorylineSource>& storylineSource, const FName id, FNodeM& outNode)
{
	if (const FNodeM* node = GetNodeMPtr(storylineSource, id))
	{
		outNode.Reset(*node);
		return true;
	}

	return false;
}

bool UStorylineServiceBFL::GetGameEventM(const TScriptInterface<IStorylineSource>& storylineSource, const FName id, FGameEventM& outGameEvent)
{
	if (const FGameEventM* gameEvent = GetGameEventMPtr(storylineSource, id))
	{
		outGameEvent.Reset(*gameEvent);
		return true;
	}

	return false;
}

TSubclassOf<UGE_Base> UStorylineServiceBFL::GetGameEventMImpl(const TScriptInterface<IStorylineSource>& storylineSource, const FName gameEventType)
{
	check(storylineSource != nullptr);

	FString contextString;
	if (FGameEventMImpl* gameEventMImpl = storylineSource->Execute_GetGameEventImpls(storylineSource->_getUObject())->FindRow<FGameEventMImpl>(gameEventType, contextString))
	{
		return gameEventMImpl->GameEventClass;
	}

	return nullptr;
}

bool UStorylineServiceBFL::GetPredicateM(const TScriptInterface<IStorylineSource>& storylineSource, const FName id, FPredicateM& outPredicate)
{
	if (const FPredicateM* predicate = GetPredicateMPtr(storylineSource, id))
	{
		outPredicate.Reset(*predicate);
		return true;
	}

	return false;
}

TSubclassOf<UP_Base> UStorylineServiceBFL::GetPredicateMImpl(const TScriptInterface<IStorylineSource>& storylineSource, const FName predicateType)
{
	check(storylineSource != nullptr);

	FString contextString;
	if (FPredicateMImpl* predicateMImpl = storylineSource->Execute_GetPredicateImpls(storylineSource->_getUObject())->FindRow<FPredicateMImpl>(predicateType, contextString))
	{
		return predicateMImpl->PredicateClass;
	}

	return nullptr;
}

void UStorylineServiceBFL::GetNextNodePaths(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FName startNodeId, TArray<FNodePath>& outNodePaths, bool& outIsPlayerChoice)
{
	outNodePaths.Reset();

	outIsPlayerChoice = true;

	TArray<FNodePath> nodePaths;
	TArray<FNodePath> newNodePaths;

	// Init
	{
		const int32 emplacedIndex = nodePaths.Emplace();
		nodePaths[emplacedIndex].StartNodeId = startNodeId;
		nodePaths[emplacedIndex].NodeIds.Add(startNodeId);

		nodePaths[emplacedIndex].LastNodeId = startNodeId;
	}

	// Fill
	while (nodePaths.Num() > 0)
	{
		newNodePaths.Reset();

		for (const FNodePath& nodePath : nodePaths)
		{
			if (const FNodeM* node = GetNodeMPtr(storylineSource, nodePath.LastNodeId))
			{
				for (const FName childNodeId : node->ChildNodeIds)
				{
					if (const FNodeM* childNode = GetNodeMPtr(storylineSource, childNodeId))
					{
						if (IsAvailableNode(WorldContextObject, storylineSource, storylineContext, childNode))
						{
							if (childNode->TypeAttribute == FName(StorylineTypeAttributes::Node_TransitM))
							{
								const int32 emplacedIndex = newNodePaths.Emplace(nodePath);
								newNodePaths[emplacedIndex].NodeIds.Add(childNodeId);

								newNodePaths[emplacedIndex].LastNodeId = childNodeId;
							}
							else
							{
								const int32 emplacedIndex = outNodePaths.Emplace(nodePath);
								outNodePaths[emplacedIndex].NodeIds.Add(childNodeId);
								outNodePaths[emplacedIndex].TargetNodeId = childNodeId;

								outIsPlayerChoice &= childNode && childNode->CharacterId == FCharacterM::PlayerId; // All target nodes should be owned by Player to be Player choice
							}
						}
					}
				}
			}
		}

		nodePaths = newNodePaths;
	}

	outIsPlayerChoice &= outNodePaths.Num() > 1; // Should be more than one possible path to be Player choice
}

void UStorylineServiceBFL::EnterNode(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FName nodeId)
{
	if (const FNodeM* node = GetNodeMPtr(storylineSource, nodeId))
	{
		EnterNodePtr(WorldContextObject, storylineSource, storylineContext, node);
	}
}

void UStorylineServiceBFL::LeaveNode(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FName nodeId)
{
	if (const FNodeM* node = GetNodeMPtr(storylineSource, nodeId))
	{
		LeaveNodePtr(WorldContextObject, storylineSource, storylineContext, node);
	}
}

const FCharacterM* UStorylineServiceBFL::GetCharacterMPtr(const TScriptInterface<IStorylineSource>& storylineSource, const FName id)
{
	check(storylineSource != nullptr);
	FString contextString;
	return storylineSource->Execute_GetCharacters(storylineSource->_getUObject())->FindRow<FCharacterM>(id, contextString);
}

const FCharacterM* UStorylineServiceBFL::GetCharacterMPtr(const TScriptInterface<IStorylineSource>& storylineSource, const UClass* actorClass)
{
	check(storylineSource != nullptr);
	FString contextString;
	TArray<FCharacterM*> characters;
	storylineSource->Execute_GetCharacters(storylineSource->_getUObject())->GetAllRows(contextString, characters);

	for (const FCharacterM* character : characters)
	{
		if (character && actorClass && character->ActorClass.ToSoftObjectPath().GetAssetPathString() == actorClass->GetPathName()) return character;
	}

	return nullptr;
}

const FItemM* UStorylineServiceBFL::GetItemMPtr(const TScriptInterface<IStorylineSource>& storylineSource, const FName id)
{
	check(storylineSource != nullptr);
	FString contextString;
	return storylineSource->Execute_GetItems(storylineSource->_getUObject())->FindRow<FItemM>(id, contextString);
}

const FQuestM* UStorylineServiceBFL::GetQuestMPtr(const TScriptInterface<IStorylineSource>& storylineSource, const FName id)
{
	check(storylineSource != nullptr);
	FString contextString;
	return storylineSource->Execute_GetQuests(storylineSource->_getUObject())->FindRow<FQuestM>(id, contextString);
}

const FDialogM* UStorylineServiceBFL::GetDialogMPtr(const TScriptInterface<IStorylineSource>& storylineSource, const FName id)
{
	check(storylineSource != nullptr);
	FString contextString;
	return storylineSource->Execute_GetDialogs(storylineSource->_getUObject())->FindRow<FDialogM>(id, contextString);
}

TArray<const FDialogM*> UStorylineServiceBFL::GetDialogMPtrsForCharacter(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FName characterId, bool onlyAvailable)
{
	check(storylineSource != nullptr);

	FString contextString;
	TArray<const FDialogM*> dialogMPtrs;
	storylineSource->Execute_GetDialogs(storylineSource->_getUObject())->GetAllRows(contextString, dialogMPtrs);

	for (TArray<const FDialogM*>::TIterator it(dialogMPtrs); it; ++it)
	{
		if ((*it)->NpcId != characterId)
		{
			it.RemoveCurrent();
		}
		else if (onlyAvailable && !IsAvailableDialog(WorldContextObject, storylineSource, storylineContext, *it))
		{
			it.RemoveCurrent();
		}
	}

	return dialogMPtrs;
}

const FReplicaM* UStorylineServiceBFL::GetReplicaMPtr(const TScriptInterface<IStorylineSource>& storylineSource, const FName id)
{
	check(storylineSource != nullptr);
	FString contextString;
	return storylineSource->Execute_GetReplicas(storylineSource->_getUObject())->FindRow<FReplicaM>(id, contextString);
}

const FNodeM* UStorylineServiceBFL::GetNodeMPtr(const TScriptInterface<IStorylineSource>& storylineSource, const FName id)
{
	check(storylineSource != nullptr);
	FString contextString;
	return storylineSource->Execute_GetNodes(storylineSource->_getUObject())->FindRow<FNodeM>(id, contextString);
}

const FGameEventM* UStorylineServiceBFL::GetGameEventMPtr(const TScriptInterface<IStorylineSource>& storylineSource, const FName id)
{
	check(storylineSource != nullptr);
	FString contextString;
	return storylineSource->Execute_GetGameEvents(storylineSource->_getUObject())->FindRow<FGameEventM>(id, contextString);
}

const FPredicateM* UStorylineServiceBFL::GetPredicateMPtr(const TScriptInterface<IStorylineSource>& storylineSource, const FName id)
{
	check(storylineSource != nullptr);
	FString contextString;
	return storylineSource->Execute_GetPredicates(storylineSource->_getUObject())->FindRow<FPredicateM>(id, contextString);
}

void UStorylineServiceBFL::EnterNodePtr(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FNodeM* node)
{
	storylineContext->Execute_OnEnterNode(storylineContext->_getUObject(), *node);

	ProcessNodeEvents(WorldContextObject, storylineSource, storylineContext, node, ESExecutionMode::ON_ENTER);

	if (node->TypeAttribute == StorylineTypeAttributes::Node_TransitM ||
		node->TypeAttribute == StorylineTypeAttributes::Node_RandomM ||
		node->TypeAttribute == StorylineTypeAttributes::Node_GateM ||
		node->TypeAttribute == StorylineTypeAttributes::Node_ExitM)
	{
		LeaveNodePtr(WorldContextObject, storylineSource, storylineContext, node);
	}
	else if (node->TypeAttribute == StorylineTypeAttributes::Node_ReplicaM ||
		node->TypeAttribute == StorylineTypeAttributes::Node_DialogM)
	{
		storylineContext->Execute_OnPlayNode(storylineContext->_getUObject());
	}
	else
	{
		check(0);
	}
}

void UStorylineServiceBFL::LeaveNodePtr(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FNodeM* node)
{
	TArray<FNodePath> nodePaths;
	bool isPlayerChoice = false;
	GetNextNodePaths(WorldContextObject, storylineSource, storylineContext, node->Id, nodePaths, isPlayerChoice);

	ProcessNodeEvents(WorldContextObject, storylineSource, storylineContext, node, ESExecutionMode::ON_LEAVE);

	storylineContext->Execute_OnLeaveNode(storylineContext->_getUObject(), isPlayerChoice);

	const int32 nodePathsNum = nodePaths.Num();

	if (nodePathsNum > 0)
	{
		storylineContext->Execute_SetNodePaths(storylineContext->_getUObject(), storylineSource, nodePaths);

		if (isPlayerChoice) // Wait for player choice
		{
			storylineContext->Execute_OnPlayerChoice(storylineContext->_getUObject());
		}
		else
		{
			int32 nextNodeIndex = INDEX_NONE;

			if (node->TypeAttribute == StorylineTypeAttributes::Node_RandomM)
			{
				nextNodeIndex = FMath::Rand() % nodePathsNum;
			}

			if (const FNodeM* nextNode = GetNextNodePtr(WorldContextObject, storylineSource, storylineContext, nextNodeIndex))
			{
				EnterNodePtr(WorldContextObject, storylineSource, storylineContext, nextNode);
			}
		}
	}
	else
	{
		storylineContext->Execute_OnEndDialog(storylineContext->_getUObject());
	}
}

const FNodeM* UStorylineServiceBFL::GetNextNodePtr(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, int32 nextNodeIndex)
{
	TArray<FNodePath> nodePaths;
	storylineContext->Execute_GetNodePaths(storylineContext->_getUObject(), nodePaths);

	if (nodePaths.Num() == 1)
	{
		const FNodePath& nodePath = nodePaths[0];
		ProcessNodePathEvents(WorldContextObject, storylineSource, storylineContext, nodePath);
		return GetNodeMPtr(storylineSource, nodePath.TargetNodeId);
	}
	else if (nodePaths.IsValidIndex(nextNodeIndex))
	{
		const FNodePath& nodePath = nodePaths[nextNodeIndex];
		ProcessNodePathEvents(WorldContextObject, storylineSource, storylineContext, nodePath);
		return GetNodeMPtr(storylineSource, nodePath.TargetNodeId);
	}
	else
	{
		check(0);
	}

	return nullptr;
}

void UStorylineServiceBFL::ProcessNodeEvents(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FNodeM* node, ESExecutionMode executionMode)
{
	for (const FName gameEventId : node->GameEventIds)
	{
		if (const FGameEventM* gameEvent = UStorylineServiceBFL::GetGameEventMPtr(storylineSource, gameEventId))
		{
			if (gameEvent->ExecutionMode == executionMode)
			{
				if (const TSubclassOf<UGE_Base>& gameEventImplClass = UStorylineServiceBFL::GetGameEventMImpl(storylineSource, gameEvent->TypeAttribute))
				{
					gameEventImplClass->GetDefaultObject<UGE_Base>()->Execute(storylineSource, storylineContext, *gameEvent);
				}
			}
		}
	}
}

void UStorylineServiceBFL::ProcessNodePathEvents(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FNodePath& nodePath)
{
	for (const FName nodeId : nodePath.NodeIds)
	{
		if (nodePath.StartNodeId != nodeId && nodePath.TargetNodeId != nodeId)
		{
			if (const FNodeM* node = GetNodeMPtr(storylineSource, nodeId))
			{
				ProcessNodeEvents(WorldContextObject, storylineSource, storylineContext, node, ESExecutionMode::ON_ENTER);
				ProcessNodeEvents(WorldContextObject, storylineSource, storylineContext, node, ESExecutionMode::ON_LEAVE);
			}
		}
	}
}

bool UStorylineServiceBFL::IsAvailableDialog(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FDialogM* dialog)
{
	for (const FName nodeId : dialog->NodeIds)
	{
		if (IsAvailableNode(WorldContextObject, storylineSource, storylineContext, GetNodeMPtr(storylineSource, nodeId))) return true;
	}

	return false;
}

bool UStorylineServiceBFL::IsAvailableNode(UObject* WorldContextObject, const TScriptInterface<IStorylineSource>& storylineSource, TScriptInterface<IStorylineContext>& storylineContext, const FNodeM* node)
{
	bool result = true;

	result &= node->Gender == ESGender::UNSET || storylineContext->Execute_GetPlayerGender(storylineContext->_getUObject()) == node->Gender;

	if (result)
	{
		for (const FName predicateId : node->PredicateIds)
		{
			if (const FPredicateM* predicate = UStorylineServiceBFL::GetPredicateMPtr(storylineSource, predicateId))
			{
				if (const TSubclassOf<UP_Base>& predicateImplClass = UStorylineServiceBFL::GetPredicateMImpl(storylineSource, predicate->TypeAttribute))
				{
					if (!predicate->IsInversed && !predicateImplClass->GetDefaultObject<UP_Base>()->Execute(storylineSource, storylineContext, *predicate) ||
						predicate->IsInversed && predicateImplClass->GetDefaultObject<UP_Base>()->Execute(storylineSource, storylineContext, *predicate))
					{
						result = false;
						break;
					}
				}
			}
		}
	}

	return result;
}