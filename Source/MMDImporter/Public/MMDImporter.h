#pragma once

#include "Modules/ModuleManager.h"

class FMMDImporterModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
