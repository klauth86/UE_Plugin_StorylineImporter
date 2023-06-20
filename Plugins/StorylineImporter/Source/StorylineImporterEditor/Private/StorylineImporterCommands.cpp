// Copyright 2023 Pentangle Studio under EULA

#include "StorylineImporterCommands.h"
#include "StorylineImporterStyle.h"

#define LOCTEXT_NAMESPACE "FStorylineImporterEditorModule"

FStorylineImporterCommands::FStorylineImporterCommands() : TCommands<FStorylineImporterCommands>(TEXT("StorylineImporter"), NSLOCTEXT("Contexts", "StorylineImporter", "StorylineImporter Plugin"), NAME_None, FStorylineImporterStyle::GetStyleSetName())
{}

void FStorylineImporterCommands::RegisterCommands()
{
	UI_COMMAND(ImportStoryline, "Import Storyline", "Imports information from Storyline XML file to Data Tables.", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE