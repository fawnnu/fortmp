#pragma once

class profile
{
public:
    inline static nlohmann::json athena_profile = R"(
	{
		"created": "0001-01-01T00:00:00.000Z",
		"updated": "0001-01-01T00:00:00.000Z",
		"rvn":1,
		"wipeNumber":1,
		"accountId":"",
		"profileId":"athena",
		"version":"FMP",
		"items":{
			"FMPitems":{
				"templateId":"CosmeticLocker:cosmeticlocker_athena",
				"attributes":{
					"locker_slots_data":{
						"slots":{
							"Pickaxe":{
								"items":[
									""
								],
								"activeVariants":[
									null
								]
							},
							"Dance":{
								"items":[
									"eid:dancemoves",
									null,
									null,
									null,
									null,
									null
								]
							},
							"Glider":{
								"items":[
									null
								]
							},
							"Character":{
								"items":[
									""
								],
								"activeVariants":[
									{
										"variants":[
											null
										]
									}
								]
							},
							"Backpack":{
								"items":[
									""
								],
								"activeVariants":[
									null
								]
							},
							"ItemWrap":{
								"items":[
									null,
									null,
									null,
									null,
									null,
									null,
									null
								],
								"activeVariants":[
									null,
									null,
									null,
									null,
									null,
									null,
									null
								]
							},
							"LoadingScreen":{
								"items":[
									null
								],
								"activeVariants":[
									null
								]
							},
							"MusicPack":{
								"items":[
									null
								],
								"activeVariants":[
									null
								]
							},
							"SkyDiveContrail":{
								"items":[
									null
								],
								"activeVariants":[
									null
								]
							}
						}
					},
					"use_count":0,
					"banner_icon_template":"DefaultBanner1",
					"locker_name":"",
					"banner_color_template":"DefaultColor1",
					"item_seen":false,
					"favorite":false
				},
				"quantity":1
			},
			"AthenaCharacter:CID_001_Athena_Commando_F_Default": {
				"templateId": "AthenaCharacter:CID_001_Athena_Commando_F_Default",
				"attributes": {
					"max_level_bonus": 0,
					"level": 1,
					"item_seen": true,
					"xp": 0,
					"variants": [],
					"favorite": false
				},
				"quantity": 1
			}
		},
		"stats":{
			"attributes":{
				"favorite_character": "AthenaCharacter:CID_001_Athena_Commando_F_Default",
				"season_match_boost":0,
				"loadouts":[
					null
				],
				"rested_xp_overflow":0,
				"mfa_reward_claimed":true,
				"quest_manager":{
				
				},
				"book_level":0,
				"season_num":0,
				"season_update":0,
				"book_xp":0,
				"permissions":[
				
				],
				"book_purchased":true,
				"lifetime_wins":0,
				"party_assist_quest":"",
				"purchased_battle_pass_tier_offers":[
				
				],
				"rested_xp_exchange":0,
				"level":100,
				"xp_overflow":0,
				"rested_xp":0,
				"rested_xp_mult":0,
				"accountLevel":100,
				"competitive_identity":{
				
				},
				"inventory_limit_bonus":0,
				"last_applied_loadout":null,
				"daily_rewards":{
				
				},
				"xp":0,
				"season_friend_match_boost":0,
				"active_loadout_index":0
			}
		},
		"commandRevision":0
	}
	)"_json;
    
    profile() = default;

    profile(httplib::Server& in_app)
    {
        in_app.Post(R"(/fortnite/api/game/v2/profile/(.*)/client/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
        	nlohmann::json profile_changes = {};
        	std::string default_profile_id = "common_core";
        	int default_revision = -1;
        	
        	if (req.has_param("profileId"))
        	{
        		default_profile_id = req.get_param_value("profileId");
        	}
        	if (req.has_param("rvn"))
        	{
        		default_revision = std::stoi(req.get_param_value("rvn"));
        	}

        	if (default_revision != 0)
        	{
        		profile_changes = {{
        			{"changeType", "fullProfileUpdate"},
        			{"profile", athena_profile}
        		}};
        	}

        	const nlohmann::json response = {
        		{"profileRevision", 1},
				{"profileId", default_profile_id},
				{"profileChangesBaseRevision", 0},
        		{"profileChanges", profile_changes},
				{"profileCommandRevision", 1},
				{"responseVersion", 1}
        	};

        	res.set_content(response.dump(), "application/json");
        });
    }
};