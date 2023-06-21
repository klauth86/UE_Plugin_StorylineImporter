// Copyright 2023 Pentangle Studio under EULA

#include "StorylineImporterEditor.h"
#include "StorylineImporterStyle.h"
#include "StorylineImporterCommands.h"
#include "StorylineServiceBFL.h"
#include "Misc/MessageDialog.h"
#include "Misc/ScopedSlowTask.h"
#include "ToolMenus.h"
#include "DesktopPlatformModule.h"
#include "EditorDirectories.h"
#include "XmlFile.h"
#include "Interfaces/IPluginManager.h"
#include "PackageTools.h"
#include "FileHelpers.h"
#include "DataTableEditorUtils.h"

#define LOCTEXT_NAMESPACE "FStorylineImporterEditorModule"

DEFINE_LOG_CATEGORY_STATIC(LogStorylineImporter, Log, All);

typedef FXmlNode FXmlElement;

namespace FStorylineDataTable
{
	const FString Characters("DT_CharacterM");
	const FString Items("DT_ItemM");
	const FString Quests("DT_QuestM");
	const FString Dialogs("DT_DialogM");
	const FString Replicas("DT_ReplicaM");
	const FString Nodes("DT_NodeM");
	const FString GameEvents("DT_GameEventM");
	const FString GameEventImpls("DT_GameEventMImpl");
	const FString Predicates("DT_PredicateM");
	const FString PredicateImpls("DT_PredicateMImpl");
	const FString RichTexts = "DT_RichTextM";
};

namespace FStorylineXmlSchema
{
	// --- VALUES ---
	
	const FString TrueValue = "true";

	// --- ATTRIBUTES ---

	const FString TypeAttribute = "xsi:type";

	// --- TAGS ---

	// StorylineM
	const FString StorylineMTag = "StorylineM";
	const FString LocationsTag = "locations";
	const FString CharactersTag = "characters";
	const FString ItemsTag = "items";
	const FString ActorsTag = "actors";
	const FString JournalTag = "journal";
	const FString DialogsTag = "dialogs";
	const FString ReplicasTag = "replicas";

	// Common
	const FString IdTag = "id";
	const FString NameTag = "name";
	const FString DescriptionTag = "description";

	// FolderM
	const FString ContentTag = "content";

	// ActorM
	const FString RtDescriptionTag = "rtDescription";
	const FString HasDescriptionFemaleTag = "hasDescriptionFemale";
	const FString RtDescriptionFemaleTag = "rtDescriptionFemale";

	// CharacterM
	const FString InitialRelationTag = "initialRelation";
	const FString InitialRelationFemaleTag = "initialRelationFemale";

	// ItemM
	const FString HasInternalDescriptionTag = "hasInternalDescription";
	const FString RtInternalDescriptionTag = "rtInternalDescription";
	const FString HasInternalDescriptionFemaleTag = "hasInternalDescriptionFemale";
	const FString RtInternalDescriptionFemaleTag = "rtInternalDescriptionFemale";

	// GraphM
	const FString GenderTag = "gender";
	const FString NodesTag = "nodes";
	const FString LinksTag = "links";
	const FString PredicatesTag = "predicates";
	const FString GameEventsTag = "gameEvents";

	// DialogM
	const FString NpcIdTag = "npcId";
	const FString LocationIdTag = "locationId";

	// NodeM
	const FString ResultTag = "result";
	const FString CharacterIdTag = "characterId";
	const FString OverrideNameTag = "overrideName";
	const FString ShortDescriptionTag = "shortDescription";

	// GateM
	const FString ExitNodeIdTag = "exitNodeId";

	// LinkM
	const FString FromNodeIdTag = "fromNodeId";
	const FString ToNodeIdTag = "toNodeId";

	// PredicateM
	const FString IsInversedTag = "isInversed";
	const FString CompositionTypeTag = "compositionType";
	const FString PredicateATag = "predicateA";
	const FString PredicateBTag = "predicateB";
	const FString DialogIdTag = "dialogId";
	const FString NodeIdTag = "nodeId";
	const FString CompareTypeTag = "compareType";
	const FString ItemIdTag = "itemId";
	const FString QuestIdTag = "questId";
	const FString ValueTag = "value";

	// GameEventM
	const FString ExecutionModeTag = "executionMode";
	// const FString ItemIdTag = "itemId"; Just using the same one under PredicateM
	// const FString QuestIdTag = "questId"; Just using the same one under PredicateM
	// const FString NodeIdTag = "nodeId"; Just using the same one under PredicateM
	// const FString NpcIdTag = "npcId"; Just using the same one under DialogM
	// const FString ValueTag = "value"; Just using the same one under PredicateM

	// RichTextM
	const FString IsNewLineTag = "isNewLine";
	const FString IsBoldTag = "isBold";
	const FString IsItalicTag = "isItalic";
	const FString IsUnderlineTag = "isUnderline";
	// const FString ContentTag = "content"; Just using the same one under FolderM
	const FString SubRangesTag = "subRanges";
};

TSet<FName> predicateTypes;
TSet<FName> gameEventTypes;
TSet<FName> richTextTypes;

FString GetDataTablePath(const FString& dataTableName) { return ("/Game/Storyline/" + dataTableName); }

UDataTable* GetDataTable(const FString& dataTableName) { return LoadObject<UDataTable>(nullptr, *GetDataTablePath(dataTableName)); }

void AddDataTablePackage(const FString& dataTableName, TArray<UPackage*>& outPackages)
{
	if (UDataTable* dataTable = GetDataTable(dataTableName))
	{
		if (outPackages.Contains(dataTable->GetPackage()))
		{
			outPackages.Add(dataTable->GetPackage());
		}
	}
}

void ImportDataTable(const FXmlElement* xmlElement, const FString& dataTableName, TFunctionRef<void(FXmlElement*, uint8*, FName rowName)> importFunc, FScopedSlowTask* scopedSlowTask, const int totalItemNum)
{		
	if (UDataTable* dataTable = GetDataTable(dataTableName))
	{
		for (FXmlElement* childXmlElement : xmlElement->GetChildrenNodes())
		{
			FString typeAttribute = childXmlElement->GetAttribute(FStorylineXmlSchema::TypeAttribute);
			
			if (!typeAttribute.IsEmpty() && dataTableName.Contains(typeAttribute)) // dataTableName is constructed from typeAttribute by prefixing with DT_
			{
				FName rowName = FName(childXmlElement->FindChildNode(FStorylineXmlSchema::IdTag)->GetContent());
				importFunc(childXmlElement, FDataTableEditorUtils::AddRow(dataTable, rowName), rowName);
			}
			else if (typeAttribute == StorylineTypeAttributes::FolderM.ToString())
			{
				if (FXmlElement* contentXmlElement = childXmlElement->FindChildNode(FStorylineXmlSchema::ContentTag))
				{
					ImportDataTable(contentXmlElement, dataTableName, importFunc, nullptr, 0);
				}
			}
			else
			{
				UE_LOG(LogStorylineImporter, Warning, TEXT("Unexpected XML element with TypeAttribute = %s while importing rows for %s... Skipped!"), *typeAttribute, *GetDataTablePath(dataTableName));
			}

			if (scopedSlowTask)
			{
				scopedSlowTask->EnterProgressFrame(100.f / totalItemNum);
				if (scopedSlowTask->ShouldCancel()) return;
			}

			FPlatformProcess::SleepNoStats(0.1f);
		}
	}
	else
	{
		UE_LOG(LogStorylineImporter, Warning, TEXT("DataTable %s is not found... Skipped!"), *GetDataTablePath(dataTableName));
	}
}

void ImportDataTableFromNamesSet(const FString& dataTableName, const TSet<FName>& namesSet)
{
	if (UDataTable* dataTable = GetDataTable(dataTableName))
	{
		for (const FName name : namesSet)
		{
			FDataTableEditorUtils::AddRow(dataTable, name);
		}
	}
	else
	{
		UE_LOG(LogStorylineImporter, Warning, TEXT("DataTable %s is not found... Skipped!"), *GetDataTablePath(dataTableName));
	}
}

void SavePackages()
{
	TArray<UPackage*> packagesToSave;

	AddDataTablePackage(FStorylineDataTable::Characters, packagesToSave);
	AddDataTablePackage(FStorylineDataTable::Items, packagesToSave);
	AddDataTablePackage(FStorylineDataTable::Quests, packagesToSave);
	AddDataTablePackage(FStorylineDataTable::Dialogs, packagesToSave);
	AddDataTablePackage(FStorylineDataTable::Replicas, packagesToSave);
	AddDataTablePackage(FStorylineDataTable::Nodes, packagesToSave);
	AddDataTablePackage(FStorylineDataTable::Predicates, packagesToSave);
	AddDataTablePackage(FStorylineDataTable::PredicateImpls, packagesToSave);
	AddDataTablePackage(FStorylineDataTable::GameEvents, packagesToSave);
	AddDataTablePackage(FStorylineDataTable::GameEventImpls, packagesToSave);
	AddDataTablePackage(FStorylineDataTable::RichTexts, packagesToSave);

	if (packagesToSave.Num() > 0)
	{
		FEditorFileUtils::PromptForCheckoutAndSave(packagesToSave, false, false);
	}
}

void ReloadPackages()
{
	TArray<UPackage*> packagesToReload;

	AddDataTablePackage(FStorylineDataTable::Characters, packagesToReload);
	AddDataTablePackage(FStorylineDataTable::Items, packagesToReload);
	AddDataTablePackage(FStorylineDataTable::Quests, packagesToReload);
	AddDataTablePackage(FStorylineDataTable::Dialogs, packagesToReload);
	AddDataTablePackage(FStorylineDataTable::Replicas, packagesToReload);
	AddDataTablePackage(FStorylineDataTable::Nodes, packagesToReload);
	AddDataTablePackage(FStorylineDataTable::Predicates, packagesToReload);
	AddDataTablePackage(FStorylineDataTable::PredicateImpls, packagesToReload);
	AddDataTablePackage(FStorylineDataTable::GameEvents, packagesToReload);
	AddDataTablePackage(FStorylineDataTable::GameEventImpls, packagesToReload);
	AddDataTablePackage(FStorylineDataTable::RichTexts, packagesToReload);

	if (packagesToReload.Num() > 0)
	{
		UPackageTools::ReloadPackages(packagesToReload);
	}
}

FName ImportPredicate(const FXmlElement* predicateXmlElement, UDataTable* dataTable)
{
	FName predicateId = NAME_None;

	const FString typeAttribute = predicateXmlElement->GetAttribute(FStorylineXmlSchema::TypeAttribute);

	if (!typeAttribute.IsEmpty())
	{
		predicateId = FName(predicateXmlElement->FindChildNode(FStorylineXmlSchema::IdTag)->GetContent());

		if (FPredicateM* predicateTableRow = reinterpret_cast<FPredicateM*>(FDataTableEditorUtils::AddRow(dataTable, predicateId)))
		{
			predicateTableRow->Id = predicateId;

			predicateTableRow->TypeAttribute = FName(typeAttribute);
			predicateTypes.FindOrAdd(predicateTableRow->TypeAttribute);

			if (const FXmlElement* isInversedXmlElement = predicateXmlElement->FindChildNode(FStorylineXmlSchema::IsInversedTag)) predicateTableRow->IsInversed = isInversedXmlElement->GetContent() == FStorylineXmlSchema::TrueValue;

			if (typeAttribute == StorylineTypeAttributes::Predicate_CompositeM.ToString())
			{
				if (const FXmlElement* compositionTypeXmlElement = predicateXmlElement->FindChildNode(FStorylineXmlSchema::CompositionTypeTag)) predicateTableRow->CompositionType = static_cast<ESCompositionType>(FCString::Atoi(*compositionTypeXmlElement->GetContent()));

				if (const FXmlElement* predicateAXmlElement = predicateXmlElement->FindChildNode(FStorylineXmlSchema::PredicateATag)) predicateTableRow->IdParam1 = ImportPredicate(predicateAXmlElement, dataTable);

				if (const FXmlElement* predicateBXmlElement = predicateXmlElement->FindChildNode(FStorylineXmlSchema::PredicateBTag)) predicateTableRow->IdParam2 = ImportPredicate(predicateBXmlElement, dataTable);
			}
			else
			{
				if (const FXmlElement* itemIdXmlElement = predicateXmlElement->FindChildNode(FStorylineXmlSchema::ItemIdTag))
				{
					if (!itemIdXmlElement->GetContent().IsEmpty())
					{
						predicateTableRow->IdParam1 = FName(itemIdXmlElement->GetContent());
					}
				}

				if (const FXmlElement* dialogIdXmlElement = predicateXmlElement->FindChildNode(FStorylineXmlSchema::DialogIdTag))
				{
					if (!dialogIdXmlElement->GetContent().IsEmpty())
					{
						predicateTableRow->IdParam1 = FName(dialogIdXmlElement->GetContent());
					}
				}

				if (const FXmlElement* questIdXmlElement = predicateXmlElement->FindChildNode(FStorylineXmlSchema::QuestIdTag))
				{
					if (!questIdXmlElement->GetContent().IsEmpty())
					{
						predicateTableRow->IdParam1 = FName(questIdXmlElement->GetContent());
					}
				}

				if (const FXmlElement* nodeIdXmlElement = predicateXmlElement->FindChildNode(FStorylineXmlSchema::NodeIdTag))
				{
					if (!nodeIdXmlElement->GetContent().IsEmpty())
					{
						predicateTableRow->IdParam2 = FName(nodeIdXmlElement->GetContent());
					}
				}

				if (const FXmlElement* compareTypeXmlElement = predicateXmlElement->FindChildNode(FStorylineXmlSchema::CompareTypeTag)) predicateTableRow->CompareType = static_cast<ESCompareType>(FCString::Atoi(*compareTypeXmlElement->GetContent()));

				if (const FXmlElement* npcIdXmlElement = predicateXmlElement->FindChildNode(FStorylineXmlSchema::NpcIdTag))
				{
					if (!npcIdXmlElement->GetContent().IsEmpty())
					{
						predicateTableRow->IdParam1 = FName(npcIdXmlElement->GetContent());
					}
				}

				if (const FXmlElement* valueXmlElement = predicateXmlElement->FindChildNode(FStorylineXmlSchema::ValueTag)) predicateTableRow->FloatParam = FCString::Atof(*valueXmlElement->GetContent());
			}
		}
	}
	else
	{
		UE_LOG(LogStorylineImporter, Warning, TEXT("Unexpected XML element with NO TypeAttribute while importing rows for %s... Skipped!"), *GetDataTablePath(FStorylineDataTable::Predicates));
	}

	return predicateId;
}

void ImportPredicates(FNodeM* nodeTableRow, const FXmlElement* nodeXmlElement)
{
	if (UDataTable* dataTable = GetDataTable(FStorylineDataTable::Predicates))
	{
		if (const FXmlElement* predicatesXmlElement = nodeXmlElement->FindChildNode(FStorylineXmlSchema::PredicatesTag))
		{
			for (const FXmlElement* predicateXmlElement : predicatesXmlElement->GetChildrenNodes())
			{
				const FName predicateId = ImportPredicate(predicateXmlElement, dataTable);
				if (predicateId != NAME_None)
				{
					nodeTableRow->PredicateIds.FindOrAdd(predicateId);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogStorylineImporter, Warning, TEXT("DataTable %s is not found... Skipped!"), *GetDataTablePath(FStorylineDataTable::Predicates));
	}
}

void ImportGameEvents(FNodeM* nodeTableRow, const FXmlElement* nodeXmlElement)
{
	if (UDataTable* dataTable = GetDataTable(FStorylineDataTable::GameEvents))
	{
		if (const FXmlElement* gameEventsXmlElement = nodeXmlElement->FindChildNode(FStorylineXmlSchema::GameEventsTag))
		{
			for (const FXmlElement* gameEventXmlElement : gameEventsXmlElement->GetChildrenNodes())
			{
				const FString typeAttribute = gameEventXmlElement->GetAttribute(FStorylineXmlSchema::TypeAttribute);

				if (!typeAttribute.IsEmpty())
				{
					const FName gameEventId = FName(gameEventXmlElement->FindChildNode(FStorylineXmlSchema::IdTag)->GetContent());

					if (FGameEventM* gameEventTableRow = reinterpret_cast<FGameEventM*>(FDataTableEditorUtils::AddRow(dataTable, gameEventId)))
					{
						gameEventTableRow->Id = gameEventId;

						gameEventTableRow->TypeAttribute = FName(typeAttribute);
						gameEventTypes.FindOrAdd(gameEventTableRow->TypeAttribute);

						if (const FXmlElement* executionModeXmlElement = gameEventXmlElement->FindChildNode(FStorylineXmlSchema::ExecutionModeTag)) gameEventTableRow->ExecutionMode = static_cast<ESExecutionMode>(FCString::Atoi(*executionModeXmlElement->GetContent()));

						if (const FXmlElement* itemIdXmlElement = gameEventXmlElement->FindChildNode(FStorylineXmlSchema::ItemIdTag))
						{
							if (!itemIdXmlElement->GetContent().IsEmpty())
							{
								gameEventTableRow->IdParam1 = FName(itemIdXmlElement->GetContent());
							}
						}

						if (const FXmlElement* questIdXmlElement = gameEventXmlElement->FindChildNode(FStorylineXmlSchema::QuestIdTag))
						{
							if (!questIdXmlElement->GetContent().IsEmpty())
							{
								gameEventTableRow->IdParam1 = FName(questIdXmlElement->GetContent());
							}
						}

						if (const FXmlElement* nodeIdXmlElement = gameEventXmlElement->FindChildNode(FStorylineXmlSchema::NodeIdTag))
						{
							if (!nodeIdXmlElement->GetContent().IsEmpty())
							{
								gameEventTableRow->IdParam2 = FName(nodeIdXmlElement->GetContent());
							}
						}

						if (const FXmlElement* npcIdXmlElement = gameEventXmlElement->FindChildNode(FStorylineXmlSchema::NpcIdTag))
						{
							if (!npcIdXmlElement->GetContent().IsEmpty())
							{
								gameEventTableRow->IdParam1 = FName(npcIdXmlElement->GetContent());
							}
						}

						if (const FXmlElement* valueXmlElement = gameEventXmlElement->FindChildNode(FStorylineXmlSchema::ValueTag)) gameEventTableRow->FloatParam = FCString::Atof(*valueXmlElement->GetContent());

						nodeTableRow->GameEventIds.FindOrAdd(gameEventId);
					}
				}
				else
				{
					UE_LOG(LogStorylineImporter, Warning, TEXT("Unexpected XML element with NO TypeAttribute while importing rows for %s... Skipped!"), *GetDataTablePath(FStorylineDataTable::GameEvents));
				}
			}
		}
	}
	else
	{
		UE_LOG(LogStorylineImporter, Warning, TEXT("DataTable %s is not found... Skipped!"), *GetDataTablePath(FStorylineDataTable::GameEvents));
	}
}

FString GetRichTextTypeString(bool isBold, bool isItalic, bool IsUnderline)
{
	FString result = "";

	if (isBold) result = result.Append("B");
	if (isItalic) result = result.Append("I");
	if (IsUnderline) result = result.Append("U");

	return result;
}

FText GetRichText(const FXmlElement* xmlElement)
{
	// Aproximately 4 pages of text, we suppose no one will do dialog or description bigger than 4 pages of text
	TStringBuilder<8192> stringBuilder;

	if (const FXmlElement* subRangesXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::SubRangesTag))
	{
		for (const FXmlElement* textRangeXmlElement : subRangesXmlElement->GetChildrenNodes())
		{
			bool isNewLine = false;
			bool isBold = false;
			bool isItalic = false;
			bool IsUnderline = false;

			if (const FXmlElement* isNewLineXmlElement = textRangeXmlElement->FindChildNode(FStorylineXmlSchema::IsNewLineTag)) isNewLine = isNewLineXmlElement->GetContent() == FStorylineXmlSchema::TrueValue;
			if (const FXmlElement* isBoldXmlElement = textRangeXmlElement->FindChildNode(FStorylineXmlSchema::IsBoldTag)) isBold = isBoldXmlElement->GetContent() == FStorylineXmlSchema::TrueValue;
			if (const FXmlElement* isItalicXmlElement = textRangeXmlElement->FindChildNode(FStorylineXmlSchema::IsItalicTag)) isItalic = isItalicXmlElement->GetContent() == FStorylineXmlSchema::TrueValue;
			if (const FXmlElement* IsUnderlineXmlElement = textRangeXmlElement->FindChildNode(FStorylineXmlSchema::IsUnderlineTag)) IsUnderline = IsUnderlineXmlElement->GetContent() == FStorylineXmlSchema::TrueValue;

			if (isNewLine)
			{
				stringBuilder.Append("\n");
			}

			if (const FXmlElement* contentXmlElement = textRangeXmlElement->FindChildNode(FStorylineXmlSchema::ContentTag))
			{
				const FString& content = contentXmlElement->GetContent();

				if (!content.IsEmpty())
				{
					if (isBold || isItalic || IsUnderline)
					{
						FString richTextTypeString = GetRichTextTypeString(isBold, isItalic, IsUnderline);

						richTextTypes.FindOrAdd(FName(richTextTypeString));

						// Tag text with corresponding Rich Text Type
						stringBuilder.Append("<").Append(richTextTypeString).Append(">");
						stringBuilder.Append(content);
						stringBuilder.Append("</").Append(richTextTypeString).Append(">");
					}
					else
					{
						stringBuilder.Append(content);
					}
				}
			}
		}
	}

	return FText::FromString(stringBuilder.ToString());
}

template<typename T>
void ImportNodes(const FXmlElement* xmlElement, T* itemTableRow)
{
	if (UDataTable* dataTable = GetDataTable(FStorylineDataTable::Nodes))
	{
		if (const FXmlElement* nodesXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::NodesTag))
		{
			if (nodesXmlElement->GetChildrenNodes().Num() > 0)
			{
				struct FChildNodes
				{
				public:
					TSet<FName> ChildNodeIds;
				};

				TSet<FString> nonRootNodeIds;

				TMap<FString, FChildNodes> links;

				if (const FXmlElement* linksXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::LinksTag))
				{
					for (const FXmlElement* linkXmlElement : linksXmlElement->GetChildrenNodes())
					{
						if (const FXmlElement* fromNodeIdXmlElement = linkXmlElement->FindChildNode(FStorylineXmlSchema::FromNodeIdTag))
						{
							if (const FXmlElement* toNodeIdXmlElement = linkXmlElement->FindChildNode(FStorylineXmlSchema::ToNodeIdTag))
							{
								const FString fromNodeId = fromNodeIdXmlElement->GetContent();

								const FString toNodeId = toNodeIdXmlElement->GetContent();

								if (!fromNodeId.IsEmpty() && !toNodeId.IsEmpty())
								{
									nonRootNodeIds.FindOrAdd(toNodeId);

									if (!links.Contains(fromNodeId))
									{
										links.Emplace(fromNodeId);
									}

									links[fromNodeId].ChildNodeIds.FindOrAdd(FName(toNodeId));
								}
							}
						}
					}
				}

				for (const FXmlElement* nodeXmlElement : nodesXmlElement->GetChildrenNodes())
				{
					const FString typeAttribute = nodeXmlElement->GetAttribute(FStorylineXmlSchema::TypeAttribute);

					if (!typeAttribute.IsEmpty())
					{
						const FString nodeId = nodeXmlElement->FindChildNode(FStorylineXmlSchema::IdTag)->GetContent();

						if (!nonRootNodeIds.Contains(nodeId))
						{
							itemTableRow->NodeIds.FindOrAdd(FName(nodeId));
						}

						if (FNodeM* nodeTableRow = reinterpret_cast<FNodeM*>(FDataTableEditorUtils::AddRow(dataTable, FName(nodeId))))
						{
							nodeTableRow->Id = FName(nodeId);

							nodeTableRow->TypeAttribute = FName(typeAttribute);

							// Nodes dont store any information in Name field, so here just skip
							// if (FXmlElement* nameXmlElement = nodeXmlElement->FindChildNode(FStorylineXmlSchema::NameTag)) nodeTableRow->Name = FText::FromString(nameXmlElement->GetContent());

							if (const FXmlElement* rtDescriptionXmlElement = nodeXmlElement->FindChildNode(FStorylineXmlSchema::RtDescriptionTag)) nodeTableRow->RtDescription = GetRichText(rtDescriptionXmlElement);

							if (const FXmlElement* genderXmlElement = nodeXmlElement->FindChildNode(FStorylineXmlSchema::GenderTag)) nodeTableRow->Gender = static_cast<ESGender>(FCString::Atoi(*genderXmlElement->GetContent()));

							if (links.Contains(nodeId))
							{
								nodeTableRow->ChildNodeIds = links[nodeId].ChildNodeIds;
							}

							// GateM nodes can jump to another ExitM node in other DialogM

							if (const FXmlElement* exitNodeIdXmlElement = nodeXmlElement->FindChildNode(FStorylineXmlSchema::ExitNodeIdTag))
							{
								const FString exitNodeId = exitNodeIdXmlElement->GetContent();

								if (!exitNodeId.IsEmpty())
								{
									nodeTableRow->ChildNodeIds.Add(FName(exitNodeId));
								}
							}

							if (const FXmlElement* resultXmlElement = nodeXmlElement->FindChildNode(FStorylineXmlSchema::ResultTag)) nodeTableRow->Result = FText::FromString(resultXmlElement->GetContent());

							ImportPredicates(nodeTableRow, nodeXmlElement);

							ImportGameEvents(nodeTableRow, nodeXmlElement);

							if (const FXmlElement* characterIdXmlElement = nodeXmlElement->FindChildNode(FStorylineXmlSchema::CharacterIdTag)) nodeTableRow->CharacterId = FName(characterIdXmlElement->GetContent());

							if (const FXmlElement* nameXmlElement = nodeXmlElement->FindChildNode(FStorylineXmlSchema::NameTag)) nodeTableRow->OverrideName = FText::FromString(nameXmlElement->GetContent());

							if (const FXmlElement* overrideNameXmlElement = nodeXmlElement->FindChildNode(FStorylineXmlSchema::OverrideNameTag)) nodeTableRow->OverrideName = FText::FromString(overrideNameXmlElement->GetContent());

							if (const FXmlElement* shortDescriptionXmlElement = nodeXmlElement->FindChildNode(FStorylineXmlSchema::ShortDescriptionTag)) nodeTableRow->ShortDescription = FText::FromString(shortDescriptionXmlElement->GetContent());
						}
					}
					else
					{
						UE_LOG(LogStorylineImporter, Warning, TEXT("Unexpected XML element with NO TypeAttribute while importing rows for %s... Skipped!"), *GetDataTablePath(FStorylineDataTable::Nodes));
					}
				}
			}
		}
	}
	else
	{
		UE_LOG(LogStorylineImporter, Warning, TEXT("DataTable %s is not found... Skipped!"), *GetDataTablePath(FStorylineDataTable::Nodes));
	}
}

void ImportStoryline(const FXmlElement* xmlElement)
{
	FScopedSlowTask importStorylineTask(100, LOCTEXT("ScopedSlowTaskMsg", "Importing from selected file"));
	importStorylineTask.MakeDialog(true);  // We display the Cancel button here

	TArray<TSharedRef<IPlugin>> enabledPlugins = IPluginManager::Get().GetEnabledPlugins();
	const TSharedRef<IPlugin>* storylineImporterPlugin = enabledPlugins.FindByPredicate([](const TSharedRef<IPlugin>& pluginRef) { return pluginRef->GetName() == "StorylineImporter"; });

	if (storylineImporterPlugin)
	{
		const FString fromPath = FPaths::ConvertRelativePathToFull(storylineImporterPlugin->Get().GetContentDir());
		const FString toPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());

		IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();

		FText errorMsg = FText::GetEmpty();

		if (platformFile.DirectoryExists(*fromPath))
		{
			if (!platformFile.DirectoryExists(*toPath))
			{
				if (!platformFile.CreateDirectory(*toPath))
				{
					errorMsg = LOCTEXT("TargetDirErrorMsg", "Target Directory (Project Content) has not been created!");
				}


			}

			if (errorMsg.IsEmpty())
			{
				////// TODO Was thinking to save before reloading, however it is not needed
				//////SavePackages();

				if (!platformFile.CopyDirectoryTree(*toPath, *fromPath, true))
				{
					////// TODO Although we get false from platformFile.CopyDirectoryTree in fact we have all work done
					//////errorMsg = LOCTEXT("CopyDirErrorMsg", "Source Directory (Plugin Content) has not been copied!");
				}

				ReloadPackages();
			}
		}
		else
		{
			errorMsg = LOCTEXT("SourceDirErrorMsg", "Source Directory (Plugin Content) has not been found!");
		}

		if (errorMsg.IsEmpty())
		{
			predicateTypes.Empty();
			gameEventTypes.Empty();
			richTextTypes.Empty();

			int totalItemNum = 0;

			if (const FXmlElement* charactersXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::CharactersTag))
			{
				totalItemNum += charactersXmlElement->GetChildrenNodes().Num();
			}

			if (const FXmlElement* itemsXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::ItemsTag))
			{
				totalItemNum += itemsXmlElement->GetChildrenNodes().Num();
			}

			if (const FXmlElement* questsXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::JournalTag))
			{
				totalItemNum += questsXmlElement->GetChildrenNodes().Num();
			}

			if (const FXmlElement* dialogsXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::DialogsTag))
			{
				totalItemNum += dialogsXmlElement->GetChildrenNodes().Num();
			}

			if (const FXmlElement* replicasXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::ReplicasTag))
			{
				totalItemNum += replicasXmlElement->GetChildrenNodes().Num();
			}

			totalItemNum += 1; // For predicateTypes, gameEventTypes, richTextTypes

			if (const FXmlElement* charactersXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::CharactersTag))
			{
				ImportDataTable(charactersXmlElement, FStorylineDataTable::Characters, [](FXmlElement* xmlElement, uint8* tableRowPtr, FName rowName)
					{
						if (FCharacterM* characterTableRow = reinterpret_cast<FCharacterM*>(tableRowPtr))
						{
							characterTableRow->Id = rowName;

							if (const FXmlElement* nameXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::NameTag)) characterTableRow->Name = FText::FromString(nameXmlElement->GetContent());

							if (const FXmlElement* rtDescriptionXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::RtDescriptionTag)) characterTableRow->RtDescription = GetRichText(rtDescriptionXmlElement);

							if (const FXmlElement* rtDescriptionFemaleXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::RtDescriptionFemaleTag)) characterTableRow->RtDescriptionFemale = GetRichText(rtDescriptionFemaleXmlElement);

							if (const FXmlElement* initialRelationXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::InitialRelationTag)) characterTableRow->InitialRelation = FCString::Atoi(*initialRelationXmlElement->GetContent());

							if (const FXmlElement* initialRelationFemaleXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::InitialRelationFemaleTag)) characterTableRow->InitialRelationFemale = FCString::Atoi(*initialRelationFemaleXmlElement->GetContent());

							if (const FXmlElement* hasDescriptionFemaleXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::HasDescriptionFemaleTag)) characterTableRow->HasDescriptionFemale = hasDescriptionFemaleXmlElement->GetContent() == FStorylineXmlSchema::TrueValue;
						}
					}, &importStorylineTask, totalItemNum);
			}

			if (importStorylineTask.ShouldCancel()) return;

			if (const FXmlElement* itemsXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::ItemsTag))
			{
				ImportDataTable(itemsXmlElement, FStorylineDataTable::Items, [](FXmlElement* xmlElement, uint8* tableRowPtr, FName rowName)
					{
						if (FItemM* itemTableRow = reinterpret_cast<FItemM*>(tableRowPtr))
						{
							itemTableRow->Id = rowName;

							if (const FXmlElement* nameXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::NameTag)) itemTableRow->Name = FText::FromString(nameXmlElement->GetContent());

							if (const FXmlElement* rtDescriptionXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::RtDescriptionTag)) itemTableRow->RtDescription = GetRichText(rtDescriptionXmlElement);

							if (const FXmlElement* rtDescriptionFemaleXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::RtDescriptionFemaleTag)) itemTableRow->RtDescriptionFemale = GetRichText(rtDescriptionFemaleXmlElement);

							if (const FXmlElement* rtInternalDescriptionXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::RtInternalDescriptionTag)) itemTableRow->RtInternalDescription = GetRichText(rtInternalDescriptionXmlElement);

							if (const FXmlElement* rtInternalDescriptionFemaleXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::RtInternalDescriptionFemaleTag)) itemTableRow->RtInternalDescriptionFemale = GetRichText(rtInternalDescriptionFemaleXmlElement);

							if (const FXmlElement* hasDescriptionFemaleXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::HasDescriptionFemaleTag)) itemTableRow->HasDescriptionFemale = hasDescriptionFemaleXmlElement->GetContent() == FStorylineXmlSchema::TrueValue;

							if (const FXmlElement* hasInternalDescriptionXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::HasInternalDescriptionTag)) itemTableRow->HasInternalDescription = hasInternalDescriptionXmlElement->GetContent() == FStorylineXmlSchema::TrueValue;

							if (const FXmlElement* hasInternalDescriptionFemaleXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::HasInternalDescriptionFemaleTag)) itemTableRow->HasInternalDescriptionFemale = hasInternalDescriptionFemaleXmlElement->GetContent() == FStorylineXmlSchema::TrueValue;
						}
					}, &importStorylineTask, totalItemNum);
			}

			if (importStorylineTask.ShouldCancel()) return;

			if (const FXmlElement* questsXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::JournalTag))
			{
				ImportDataTable(questsXmlElement, FStorylineDataTable::Quests, [](FXmlElement* xmlElement, uint8* tableRowPtr, FName rowName)
					{
						if (FQuestM* questTableRow = reinterpret_cast<FQuestM*>(tableRowPtr))
						{
							questTableRow->Id = rowName;

							if (const FXmlElement* nameXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::NameTag)) questTableRow->Name = FText::FromString(nameXmlElement->GetContent());

							if (const FXmlElement* descriptionXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::DescriptionTag)) questTableRow->Description = FText::FromString(descriptionXmlElement->GetContent());

							ImportNodes(xmlElement, questTableRow);
						}
					}, &importStorylineTask, totalItemNum);
			}

			if (importStorylineTask.ShouldCancel()) return;

			if (const FXmlElement* dialogsXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::DialogsTag))
			{
				ImportDataTable(dialogsXmlElement, FStorylineDataTable::Dialogs, [](FXmlElement* xmlElement, uint8* tableRowPtr, FName rowName)
					{
						if (FDialogM* dialogTableRow = reinterpret_cast<FDialogM*>(tableRowPtr))
						{
							dialogTableRow->Id = rowName;

							if (const FXmlElement* nameXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::NameTag)) dialogTableRow->Name = FText::FromString(nameXmlElement->GetContent());

							if (const FXmlElement* descriptionXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::DescriptionTag)) dialogTableRow->Description = FText::FromString(descriptionXmlElement->GetContent());

							ImportNodes(xmlElement, dialogTableRow);

							if (const FXmlElement* npcIdXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::NpcIdTag)) dialogTableRow->NpcId = FName(npcIdXmlElement->GetContent());

							if (const FXmlElement* locationIdXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::LocationIdTag)) dialogTableRow->LocationId = FName(locationIdXmlElement->GetContent());
						}
					}, &importStorylineTask, totalItemNum);
			}

			if (importStorylineTask.ShouldCancel()) return;

			if (const FXmlElement* replicasXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::ReplicasTag))
			{
				ImportDataTable(replicasXmlElement, FStorylineDataTable::Replicas, [](FXmlElement* xmlElement, uint8* tableRowPtr, FName rowName)
					{
						if (FDialogM* replicaTableRow = reinterpret_cast<FDialogM*>(tableRowPtr))
						{
							replicaTableRow->Id = rowName;

							if (const FXmlElement* nameXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::NameTag)) replicaTableRow->Name = FText::FromString(nameXmlElement->GetContent());

							if (const FXmlElement* descriptionXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::DescriptionTag)) replicaTableRow->Description = FText::FromString(descriptionXmlElement->GetContent());

							ImportNodes(xmlElement, replicaTableRow);

							if (const FXmlElement* locationIdXmlElement = xmlElement->FindChildNode(FStorylineXmlSchema::LocationIdTag)) replicaTableRow->LocationId = FName(locationIdXmlElement->GetContent());
						}
					}, &importStorylineTask, totalItemNum);
			}

			if (importStorylineTask.ShouldCancel()) return;

			predicateTypes.Sort([](const FName& predicateTypeA, const FName& predicateTypeB) { return predicateTypeA.FastLess(predicateTypeB); });
			ImportDataTableFromNamesSet(FStorylineDataTable::PredicateImpls, predicateTypes);

			gameEventTypes.Sort([](const FName& gameEventTypeA, const FName& gameEventTypeB) { return gameEventTypeA.FastLess(gameEventTypeB); });
			ImportDataTableFromNamesSet(FStorylineDataTable::GameEventImpls, gameEventTypes);

			richTextTypes.FindOrAdd("Default");
			richTextTypes.Sort([](const FName& richTextTypeA, const FName& richTextTypeB) { return richTextTypeA.FastLess(richTextTypeB); });
			ImportDataTableFromNamesSet(FStorylineDataTable::RichTexts, richTextTypes);

			SavePackages();
		}
		else
		{
			FMessageDialog::Open(EAppMsgType::Ok, errorMsg);
		}
	}
}

void FStorylineImporterEditorModule::StartupModule()
{
	FStorylineImporterStyle::Initialize();
	FStorylineImporterStyle::ReloadTextures();

	FStorylineImporterCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FStorylineImporterCommands::Get().ImportStoryline,
		FExecuteAction::CreateRaw(this, &FStorylineImporterEditorModule::OnImportStoryline),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FStorylineImporterEditorModule::RegisterMenus));
}

void FStorylineImporterEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FStorylineImporterStyle::Shutdown();

	FStorylineImporterCommands::Unregister();
}

void FStorylineImporterEditorModule::OnImportStoryline()
{
	FString AllExtensions = "*.xml";
	FString FileTypes = "eXtensible Markup Language (*.xml)|*.xml";
	FileTypes = FString::Printf(TEXT("All Files (%s)|%s|%s"), *AllExtensions, *AllExtensions, *FileTypes);

	// Prompt the user for the filenames
	TArray<FString> OpenFilenames;
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	bool bOpened = false;
	int32 FilterIndex = -1;

	if (DesktopPlatform)
	{
		const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

		bOpened = DesktopPlatform->OpenFileDialog(
			ParentWindowWindowHandle,
			LOCTEXT("ImportDialogTitle", "Import Storyline").ToString(),
			FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_IMPORT),
			TEXT(""),
			FileTypes,
			EFileDialogFlags::None,
			OpenFilenames,
			FilterIndex
		);
	}

	if (bOpened)
	{
		if (OpenFilenames.Num() > 0)
		{
			FEditorDirectories::Get().SetLastDirectory(ELastDirectory::GENERIC_IMPORT, OpenFilenames[0]);

			FXmlFile xmlFile;

			xmlFile.LoadFile(OpenFilenames[0]);

			if (xmlFile.IsValid())
			{
				const FXmlElement* rootXmlElement = xmlFile.GetRootNode();

				if (rootXmlElement->GetTag() == FStorylineXmlSchema::StorylineMTag)
				{
					ImportStoryline(rootXmlElement);
				}
				else
				{
					FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("IsNotStorylineFileMsg", "Selected file is not a Storyline XML document!"));
				}
			}
			else
			{
				FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("InvalidFileMsg", "Selected file is not a valid XML document!"));
			}
		}
	}
}

void FStorylineImporterEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("GetContent");
		Section.AddMenuEntryWithCommandList(FStorylineImporterCommands::Get().ImportStoryline, PluginCommands);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FStorylineImporterEditorModule, StorylineImporterEditor)