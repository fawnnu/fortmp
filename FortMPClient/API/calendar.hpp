#pragma once

class calendar
{
public:
    inline static nlohmann::json calendar_response = R"(
	{
		"channels":{
			"standalone-store":{
				"states":[
					{
						"validFrom":"0001-01-01T00:00:00.000Z",
						"activeEvents":[
						],
						"state":{
							"activePurchaseLimitingEventIds":[
							
							],
							"storefront":{
							
							},
							"rmtPromotionConfig":[
							
							],
							"storeEnd":"0001-01-01T00:00:00.000Z"
						}
					}
				],
				"cacheExpire":"0001-01-01T00:00:00.000Z"
			},
			"client-events":{
				"states":[
					{
						"validFrom":"0001-01-01T00:00:00.000Z",
						"activeEvents":[
							{
								"eventType":"EventFlag.Season0",
								"activeUntil":"9999-12-31T23:59:59.999Z",
								"activeSince":"0001-01-01T00:00:00.000Z"
							},
							{
								"eventType":"EventFlag.LobbySeason0",
								"activeUntil":"9999-12-31T23:59:59.999Z",
								"activeSince":"0001-01-01T00:00:00.000Z"
							}
						],
						"state":{
							"activeStorefronts":[
							],
							"eventNamedWeights":{
							},
							"seasonNumber":0,
							"seasonTemplateId":"AthenaSeason:athenaseason0",
							"matchXpBonusPoints":0.0,
							"eventPunchCardTemplateId":"",
							"seasonBegin":"0001-01-01T00:00:00.000Z",
							"seasonEnd":"9999-12-31T23:59:59.999Z",
							"seasonDisplayedEnd":"9999-12-31T23:59:59.999Z",
							"weeklyStoreEnd":"9999-12-31T23:59:59.999Z",
							"dailyStoreEnd":"9999-12-31T23:59:59.999Z",
							"stwDailyStoreEnd":"9999-12-31T23:59:59.999Z",
							"stwEventStoreEnd":"9999-12-31T23:59:59.999Z",
							"stwWeeklyStoreEnd":"9999-12-31T23:59:59.999Z",
							"sectionStoreEnds":{
							}
						}
					}
				],
				"cacheExpire":"9999-12-31T23:59:59.999Z"
			}
		},
		"currentTime":"0001-01-01T00:00:00.000Z",
		"cacheIntervalMins":15.0
	}
	)"_json;

	calendar() = default;

	calendar(httplib::Server& in_app)
	{
		in_app.Get("/fortnite/api/calendar/v1/timeline", [](const httplib::Request& req, httplib::Response& res)
		{
			std::string user_agent = req.get_header_value("User-Agent");

			int season_number = api_utils::get_season_number(user_agent);

			calendar_response["channels"]["client-events"]["states"][0]["activeEvents"][0]["eventType"] = "EventFlag.Season" + std::to_string(season_number);
			calendar_response["channels"]["client-events"]["states"][0]["activeEvents"][1]["eventType"] = season_number == 2 ? "EventFlag.LobbyWinterDecor" : "EventFlag.LobbySeason" + std::to_string(season_number);
			calendar_response["channels"]["client-events"]["states"][0]["state"]["seasonTemplateId"] = "AthenaSeason:athenaseason" + std::to_string(season_number);
			calendar_response["channels"]["client-events"]["states"][0]["state"]["seasonNumber"] = std::to_string(season_number);

			res.set_content(calendar_response.dump(), "application/json");
		});
	}
};