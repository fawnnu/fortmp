#pragma once

class fortnitegame
{
public:
    inline static nlohmann::json content_pages = R"(
	{
		"_title": "FortMP News",
		"_locale": "en-US",
		"lastModified": "2022-02-28T06:38:50.565Z",
		"loginmessage": {
            "_title": "LoginMessage",
            "loginmessage": {
                "_type": "CommonUI Simple Message",
                "message": {
                    "_type": "CommonUI Simple Message Base",
                    "title": "",
                    "body": ""
                }
            },
            "_locale": "en-US",
			"lastModified": "2022-02-28T06:38:50.566Z"
        },
		"battleroyalenews": {
			"_title": "battleroyalenews",
			"news": {
				"messages": [{
                        "image": null,
                        "hidden": false,
                        "_type": "CommonUI Simple Message Base",
                        "adspace": "",
                        "title": "",
                        "body": "",
                        "spotlight": false
                    }
				],
				"motds": [
				]
			},
			"_locale": "en-US",
			"lastModified": "2022-02-28T06:38:50.566Z"
		},
		"battleroyalenewsv2": {
			"_title": "battleroyalenews",
			"news": {
				"messages": [{
                        "image": null,
                        "hidden": false,
                        "_type": "CommonUI Simple Message Base",
                        "adspace": "",
                        "title": "",
                        "body": "",
                        "spotlight": false
                    }
				],
				"motds": [
				]
			},
			"_locale": "en-US",
			"lastModified": "2022-02-28T06:38:50.566Z"
		},
		"emergencynotice": {
			"_title": "emergencynotice",
			"news": {
				"messages": [{
						"_type": "CommonUI Simple Message Base",
						"image": null,
						"hidden": false,
						"messagetype": "normal",
						"title": "",
						"body": "",
						"spotlight": false
					}
				],
				"motds": [
				]
			},
			"_locale": "en-US",
			"lastModified": "2022-02-28T06:38:50.568Z"
		},
		"emergencynoticev2": {
			"_title": "emergencynotice",
			"news": {
				"messages": [{
						"_type": "CommonUI Simple Message Base",
						"image": null,
						"hidden": false,
						"messagetype": "normal",
						"title": "",
						"body": "",
						"spotlight": false
					}
				],
				"motds": [
				]
			},
			"_locale": "en-US",
			"lastModified": "2022-02-28T06:38:50.568Z"
		},
		"subgameinfo": {
			"_title": "SubgameInfo",
			"_locale": "en-US",
			"lastModified": "2022-02-28T06:38:50.569Z",
			"battleroyale": {
				"_type": "Subgame Info",
				"image": "",
				"color": "000000",
				"specialMessage": null,
				"description": "100 Player PvP",
				"subgame": "battleroyale",
				"standardMessageLine2": null,
				"title": "Battle Royale",
				"standardMessageLine1": null
			},
			"savetheworld": {
				"_type": "Subgame Info",
				"image": "",
				"color": "7615E9FF",
				"specialMessage": null,
				"description": "Cooperative PvE Adventure",
				"subgame": "savetheworld",
				"standardMessageLine2": null,
				"title": "savetheworld",
				"standardMessageLine1": null
			},
			"creative": {
				"_type": "Subgame Info",
				"image": "",
				"color": "13BDA1FF",
				"specialMessage": null,
				"description": "Your Islands. Your Friends. Your Rules.",
				"subgame": "creative",
				"standardMessageLine2": null,
				"title": "",
				"standardMessageLine1": null
			}
		},
		"subgameselectdata": {
			"_title": "subgameselectdata",
			"saveTheWorldUnowned": {
				"_type": "CommonUI Simple Message",
				"message": {
					"_type": "CommonUI Simple Message Base",
					"image": "",
					"hidden": false,
					"messagetype": "normal",
					"title": "Co-op PvE",
					"body": "Cooperative PvE storm-fighting adventure!",
					"spotlight": false
				}
			},
			"battleRoyale": {
				"_type": "CommonUI Simple Message",
				"message": {
					"_type": "CommonUI Simple Message Base",
					"image": "",
					"hidden": false,
					"messagetype": "normal",
					"title": "100 Player PvP",
					"body": "100 Player PvP Battle Royale.\n\nPvE Progress does not affect Battle Royale.",
					"spotlight": false
				}
			},
			"creative": {
				"_type": "CommonUI Simple Message",
				"message": {
					"_type": "CommonUI Simple Message Base",
					"image": "",
					"hidden": false,
					"messagetype": "normal",
					"title": "NEW - Build Your Own Island!",
					"body": "Create your own unique games and play with your friends!",
					"spotlight": false
				}
			},
			"saveTheWorld": {
				"_type": "CommonUI Simple Message",
				"message": {
					"_type": "CommonUI Simple Message Base",
					"image": "",
					"hidden": false,
					"messagetype": "normal",
					"title": "Co-op PvE",
					"body": "Cooperative PvE storm-fighting adventure!",
					"spotlight": false
				}
			},
			"lastModified": "2022-02-28T06:38:50.569Z",
			"_locale": "en-US"
		},
		"dynamicbackgrounds": {
			"_title": "dynamicbackgrounds",
			"backgrounds": {
				"backgrounds": [{
						"stage": "season1",
						"key": "lobby",
						"_type": "DynamicBackground"
					},{
						"stage": "season1",
						"key": "vault",
						"_type": "DynamicBackground"
					}
				],
				"_type": "DynamicBackgroundList"
			},
			"_locale": "en-US",
			"lastModified": "2022-02-28T06:38:50.570Z"
		}
	}
	)"_json;
    
    fortnitegame() = default;

    fortnitegame(httplib::Server& in_app)
    {
        in_app.Post(R"(/fortnite/api/game/v2/tryPlayOnPlatform/account/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            res.set_content("true", "text/plain");
        });

        in_app.Get("/fortnite/api/game/v2/enabled_features", [](const httplib::Request& req, httplib::Response& res)
        {
            res.set_content("[]", "application/json"); 
        });

        in_app.Get("/fortnite/api/game/v2/world/info", [](const httplib::Request& req, httplib::Response& res)
        {
            res.set_content("{}", "application/json");
        });

        in_app.Get(R"(/fortnite/api/game/v2/twitch/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            res.set_content("{}", "application/json"); 
        });

        in_app.Get(R"(/fortnite/api/matchmaking/session/findPlayer/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            res.status = 204; 
        });

        in_app.Get(R"(/fortnite/api/v2/versioncheck/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            const nlohmann::json response = {
                {"type", "NO_UPDATE"}
            };

            res.set_content(response.dump(), "application/json");
        });

        in_app.Get(R"(/fortnite/api/v2/versioncheck(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            const nlohmann::json response = {
                {"type", "NO_UPDATE"}
            };
            
            res.set_content(response.dump(), "application/json");
        });

        in_app.Get(R"(/fortnite/api/versioncheck(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            const nlohmann::json response = {
                {"type", "NO_UPDATE"}
            };
            
            res.set_content(response.dump(), "application/json");
        });

    	in_app.Get("/content/api/pages/fortnite-game", [](const httplib::Request& req, httplib::Response& res)
    	{
    		// Set login message
    		content_pages["loginmessage"]["loginmessage"]["message"]["title"] = "FortMP";
    		content_pages["loginmessage"]["loginmessage"]["message"]["body"] = XOR("Welcome to FortMP, a Fortnite Battle Royale private multiplayer experience\n\nCredits to Jake and Lawin for some of the responses used by FMP API");

    		// Set news
    		content_pages["battleroyalenews"]["news"]["messages"][0]["title"] = "FortMP";
    		content_pages["battleroyalenews"]["news"]["messages"][0]["body"] = XOR("Make sure to join the Era discord server: discord.gg/erafn");
			content_pages["battleroyalenewsv2"]["news"]["messages"][0]["title"] = "FortMP";
    		content_pages["battleroyalenewsv2"]["news"]["messages"][0]["body"] = XOR("Make sure to join the Era discord server: discord.gg/erafn");

    		// Set emergency notice
    		content_pages["emergencynotice"]["news"]["messages"][0]["title"] = XOR("FortMP");
			content_pages["emergencynotice"]["news"]["messages"][0]["body"] = XOR("Press play to join a game.");
			content_pages["emergencynoticev2"]["news"]["messages"][0]["title"] = XOR("FortMP");
			content_pages["emergencynoticev2"]["news"]["messages"][0]["body"] = XOR("Press play to join a game.");

    		// Set dynamic background
			std::string target_season = "season" + std::to_string(api_utils::get_season_number(req.get_header_value("User-Agent")));

			if (api_utils::get_season_number(req.get_header_value("User-Agent")) == 10)
			{
				target_season = "seasonx";
			}
    		
			content_pages["dynamicbackgrounds"]["backgrounds"]["backgrounds"][0]["stage"] = target_season;
			content_pages["dynamicbackgrounds"]["backgrounds"]["backgrounds"][1]["stage"] = target_season;
    		
    		res.set_content(content_pages.dump(), "application/json");
    	});
    }
};