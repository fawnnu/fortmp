#pragma once

class blueprint_component
{
public:
    blueprint_component();

    // Wrapper UFunction to call spdlog on blueprints
    static void LogFMP(UObject* Self, FFrame& StackFrame);
};
