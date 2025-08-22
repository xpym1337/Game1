// Fill out your copyright notice in the Description page of Project Settings.

#include "EROEOREOREOR.h"
#include "Modules/ModuleManager.h"
#include "GameplayTagsManager.h"

// Custom game module for native tag registration - Epic Games standard
class FEROEOREOREORGameModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FDefaultGameModuleImpl::StartupModule();
		
		// Register critical gameplay tags natively - Epic Games standard
		UGameplayTagsManager& GameplayTagsManager = UGameplayTagsManager::Get();
		
		// Add native tags that must be available at runtime
		GameplayTagsManager.AddNativeGameplayTag(FName("Input.Dash.Left"), TEXT("Left dash input direction"));
		GameplayTagsManager.AddNativeGameplayTag(FName("Input.Dash.Right"), TEXT("Right dash input direction"));
		
		UE_LOG(LogTemp, Log, TEXT("EROEOREOREOR: Native gameplay tags registered"));
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE( FEROEOREOREORGameModule, EROEOREOREOR, "EROEOREOREOR" );
