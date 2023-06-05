#include "include.hpp"
#include "battlepass_component.hpp"

battlepass_component::battlepass_component()
{
    // Issue request to backend
    httplib::Client client(XOR("https://api.v1.external.erafn.org"));
    httplib::Headers headers = {
        { XOR("x-era-client-token"), XOR("HVO4Dkf54RvWhWAWyCthiRQW0w5E1IJ4hXzlyzVgtvjXmYLEVXOO6GSsWL8ldSpa") }
    };
    
    auto Result = client.Get(XOR("/api/external/game/seasons/rewards?build=") + std::to_string(GetCL()), headers);

    try
    {
        nlohmann::json JsonResult = nlohmann::json::parse(Result->body);

        if (JsonResult.contains(XOR("data")) && JsonResult[XOR("data")].is_array())
        {
            nlohmann::json Data = JsonResult[XOR("data")];
            
            for (int i = 0; i < Data.size(); i++)
            {
                nlohmann::json CurrentDataElement = Data.at(i);

                if (CurrentDataElement.is_object())
                {
                    if (CurrentDataElement.contains(XOR("rewards")) && CurrentDataElement[XOR("rewards")].is_array())
                    {
                        nlohmann::json rewards = CurrentDataElement[XOR("rewards")];

                        if (rewards.size() == 0)
                        {
                            // Empty the reward at this level
                            UAthenaSeasonItemDefinition* ActiveSeasonDefinition = FindObject<UAthenaSeasonItemDefinition>(XOR(L"/Game/Athena/Items/Seasons/AthenaSeason4.AthenaSeason4"));

                            if (ActiveSeasonDefinition)
                            {
                                auto& FreeSchedule = ActiveSeasonDefinition->BookXpScheduleFree();

                                TArrayManager Levels = TArrayManager<FAthenaRewardScheduleLevel>(&FreeSchedule.Levels(), FAthenaRewardScheduleLevel::GetStructSize());
                                TArrayManager Rewards = TArrayManager<FAthenaRewardItemReference>(&Levels[i].Rewards(), FAthenaRewardItemReference::GetStructSize());

                                for (FAthenaRewardItemReference& Reward : Rewards)
                                {
                                    // Empty
                                    ZeroMemory(&Reward, FAthenaRewardItemReference::GetStructSize());
                                }
                            }
                            
                            continue;
                        }

                        for (int j = 0; j < rewards.size(); j++)
                        {
                            nlohmann::json CurrentRewardElement = rewards.at(j);

                            if (CurrentRewardElement.is_object())
                            {
                                if (CurrentRewardElement.contains(XOR("asset")) &&
                                    CurrentRewardElement.contains(XOR("quantity")) &&
                                    CurrentRewardElement.contains(XOR("template")))
                                {
                                    std::string Asset = CurrentRewardElement[XOR("asset")];
                                    int Quantity = CurrentRewardElement[XOR("quantity")];
                                    std::string Template = CurrentRewardElement[XOR("template")];

                                    // Now apply these rewards to the free battle pass
                                    //TODO: Get the currently active one
                                    UAthenaSeasonItemDefinition* ActiveSeasonDefinition = FindObject<UAthenaSeasonItemDefinition>(XOR(L"/Game/Athena/Items/Seasons/AthenaSeason4.AthenaSeason4"));

                                    if (ActiveSeasonDefinition)
                                    {
                                        auto& FreeSchedule = ActiveSeasonDefinition->BookXpScheduleFree();

                                        // Valid level index
                                        if (i < FreeSchedule.Levels().Num())
                                        {
                                            TArrayManager Levels = TArrayManager<FAthenaRewardScheduleLevel>(&FreeSchedule.Levels(), FAthenaRewardScheduleLevel::GetStructSize());
                                            TArrayManager Rewards = TArrayManager<FAthenaRewardItemReference>(&Levels[i].Rewards(), FAthenaRewardItemReference::GetStructSize());

                                            // Convert strings
                                            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
                                            std::wstring wAsset = converter.from_bytes(Asset);
                                            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter2;
                                            std::wstring wTemplate = converter2.from_bytes(Template);
                                        
                                            if (j < Rewards.Num())
                                            {
                                                // Clear the Reward entry
                                                ZeroMemory(&Rewards[j], FAthenaRewardItemReference::GetStructSize());
                                            
                                                Rewards[j].ItemDefinition().WeakPtr = FWeakObjectPtr(FindObject(wAsset.c_str()));
                                                Rewards[j].ItemDefinition().ObjectID.AssetPathName = wAsset.c_str();
                                                Rewards[j].TemplateId() = wTemplate;
                                                
                                                Rewards[j].Quantity() = Quantity;
                                            }
                                            else
                                            {
                                                // Create an empty reward entry and initialize it, then add it to the Rewards array
                                                FAthenaRewardItemReference* NewReward = static_cast<FAthenaRewardItemReference*>(alloca(FAthenaRewardItemReference::GetStructSize()));
                                                ZeroMemory(NewReward, FAthenaRewardItemReference::GetStructSize());

                                                NewReward->ItemDefinition().WeakPtr = FWeakObjectPtr(FindObject(wAsset.c_str()));
                                                NewReward->ItemDefinition().ObjectID.AssetPathName = wAsset.c_str();
                                                NewReward->TemplateId() = wTemplate;
                                                NewReward->Quantity() = Quantity;

                                                Rewards.Add(NewReward);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    catch (nlohmann::json::parse_error& ex)
    {
        
    }
}
