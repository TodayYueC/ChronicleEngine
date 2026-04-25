#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FChronicleEngineTestsModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FChronicleEngineTestsModule, ChronicleEngineTests)

