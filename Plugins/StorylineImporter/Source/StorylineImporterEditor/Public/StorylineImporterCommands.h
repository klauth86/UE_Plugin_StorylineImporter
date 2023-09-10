// Copyright 2023 Pentangle Studio under EULA

#pragma once

#include "Framework/Commands/Commands.h"

class FStorylineImporterCommands : public TCommands<FStorylineImporterCommands>
{
public:
	FStorylineImporterCommands();

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> ImportStoryline;
};