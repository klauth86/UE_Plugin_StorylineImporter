// Copyright 2023 Pentangle Studio under EULA

#pragma once

#include "Modules/ModuleManager.h"

class FStorylineImporterModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};