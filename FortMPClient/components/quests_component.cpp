#include "include.hpp"
#include "quests_component.hpp"

quests_component::quests_component()
{
    // Issue request to backend
    httplib::Client client(XOR("https://api.v1.external.erafn.org"));
    httplib::Headers headers = {
        { XOR("x-era-client-token"), XOR("HVO4Dkf54RvWhWAWyCthiRQW0w5E1IJ4hXzlyzVgtvjXmYLEVXOO6GSsWL8ldSpa") }
    };
    
    auto Result = client.Get(XOR("/api/external/game/seasons/quests?build=") + std::to_string(GetCL()), headers);
    
    if (Result->status == 200)
    {
        std::string Body = Result->body;

        try
        {
            nlohmann::json JsonBody = nlohmann::json::parse(Body);

            if (JsonBody.contains(XOR("data")))
            {
                nlohmann::json Data = JsonBody[XOR("data")];

                if (Data.is_array())
                {
                    for (auto& elem : Data.items())
                    {
                        if (elem.value().is_object())
                        {
                            if (elem.value().contains(XOR("rewards")) && elem.value().contains(XOR("template")))
                            {
                                // Split the template and find the target object
                                std::string template_id = elem.value()[XOR("template")];
                                if (template_id.find(XOR(":")) != std::string::npos)
                                {
                                    template_id = template_id.substr(template_id.find(XOR(":")) + 1);
                                    TO_WSTRING(template_id, wTemplate_id);

                                    if (UFortQuestItemDefinition* Quest = FindObject<UFortQuestItemDefinition>(wTemplate_id.c_str(), false, true))
                                    {
                                        nlohmann::json Rewards = elem.value()[XOR("rewards")];

                                        if (Rewards.is_array())
                                        {
                                            if (Rewards.size() > 0)
                                            {
                                                static void* Property = FindPropertyByName(Quest->Class, XOR(L"Rewards"));
                                                
                                                // Clear the Quest rewards
                                                TArrayManager<FFortItemQuantityPair> RewardsArray(&Quest->Rewards(), FFortItemQuantityPair::GetStructSize());
                                                RewardsArray.Clear(Property);

                                                // Add each Reward
                                                for (auto& Reward : Rewards.items())
                                                {
                                                    if (Reward.value().is_object())
                                                    {
                                                        if (Reward.value().contains(XOR("asset")) &&
                                                            Reward.value().contains(XOR("quantity")))
                                                        {
                                                            std::string asset = Reward.value()[XOR("asset")];
                                                            int quantity = Reward.value()[XOR("quantity")];
                                                            TO_WSTRING(asset, wAsset);

                                                            // Build new FFortItemQuantityPair
                                                            FFortItemQuantityPair* NewItemQuantityPair = static_cast<FFortItemQuantityPair*>(alloca(FFortItemQuantityPair::GetStructSize()));
                                                            ZeroMemory(NewItemQuantityPair, FFortItemQuantityPair::GetStructSize());

                                                            // Set ItemDefinition
                                                            NewItemQuantityPair->ItemDefinition().WeakPtr = FWeakObjectPtr(FindObject(wAsset.c_str()));
                                                            NewItemQuantityPair->ItemDefinition().ObjectID.AssetPathName = wAsset.c_str();
                                                            NewItemQuantityPair->Quantity() = quantity;

                                                            // Grant Reward
                                                            RewardsArray.Add(NewItemQuantityPair);
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
            }
        }
        catch (nlohmann::json::parse_error& err)
        {
            
        }
    }
}

