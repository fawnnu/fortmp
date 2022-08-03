#pragma once

class UAthenaGameData : public UObject
{
public:
    DEFINE_PROPERTY(TArray<UAthenaCharacterItemDefinition*>, RandomCharacters);
};