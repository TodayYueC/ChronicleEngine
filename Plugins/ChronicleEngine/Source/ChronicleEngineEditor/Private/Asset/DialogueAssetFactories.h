#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "DialogueAssetFactories.generated.h"

UCLASS()
class UDialogueTreeFactory : public UFactory
{
    GENERATED_BODY()

public:
    UDialogueTreeFactory();
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

UCLASS()
class UDialogueDatabaseFactory : public UFactory
{
    GENERATED_BODY()

public:
    UDialogueDatabaseFactory();
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

UCLASS()
class USpeakerProfileFactory : public UFactory
{
    GENERATED_BODY()

public:
    USpeakerProfileFactory();
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

