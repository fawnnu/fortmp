#pragma once

class lightswitch
{
public:
    inline static nlohmann::json response = R"(
		[{
			"serviceInstanceId": "fortnite",
			"status": "UP",
			"message": "UP",
			"maintenanceUri": null,
			"allowedActions": [],
			"banned": false
		}]
	)"_json;

	lightswitch() = default;
	
    lightswitch(httplib::Server& in_app)
    {
        in_app.Get("/lightswitch/api/service/bulk/status", [](const httplib::Request& req, httplib::Response& res)
        {
        	res.set_content(response.dump(), "application/json");
        });
    }
};
