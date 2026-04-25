#pragma once

#include "Modules/ModuleInterface.h"

class FChronicleEngineModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

