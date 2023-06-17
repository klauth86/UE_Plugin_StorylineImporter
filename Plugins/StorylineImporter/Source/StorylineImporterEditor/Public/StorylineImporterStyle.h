// Copyright 2023 Pentangle Studio

#pragma once

#include "Styling/SlateStyle.h"

class FSlateStyleSet;

class FStorylineImporterStyle
{
public:
	static void Initialize();
	static void Shutdown();

	static void ReloadTextures();

	static const ISlateStyle& Get();

	static FName GetStyleSetName();

private:
	static TSharedRef<FSlateStyleSet> Create();

private:
	static TSharedPtr<FSlateStyleSet> StyleInstance;
};