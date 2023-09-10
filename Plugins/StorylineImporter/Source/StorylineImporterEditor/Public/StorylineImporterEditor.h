// Copyright 2023 Pentangle Studio under EULA

#pragma once

#include "Modules/ModuleManager.h"

class FStorylineImporterEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void OnImportStoryline();

private:
	void RegisterMenus();

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};