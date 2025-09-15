// TemperatureSettings.h
#pragma once
#include "Engine/DeveloperSettings.h"
#include "TemperatureSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Temperature"))
class FUNPROJECT_API UTemperatureSettings : public UDeveloperSettings
{
    GENERATED_BODY()
public:
    // Wähle hier im Editor dein Default-Rules-Asset
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Rules", meta = (AllowedClasses = "TemperatureRules"))
    FSoftObjectPath DefaultRules;
};
