#pragma once

class cloudstorage
{
public:
    cloudstorage() = default;

    cloudstorage(httplib::Server& in_app)
    {
        in_app.Get(R"(/fortnite/api/cloudstorage/user/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            res.set_content("[]", "application/json");
        });

        in_app.Get(R"(/fortnite/api/cloudstorage/user/(.*)/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            res.status = 200;
        });

        in_app.Put(R"(/fortnite/api/cloudstorage/user/(.*)/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            std::string filename = req.matches[2];

            if (filename != "ClientSettings.Sav")
            {
                res.status = 404;

                const nlohmann::json file_not_found = R"({
                    "error": "file not found"
                })"_json;
                
                res.set_content(file_not_found.dump(), "application/json");

                return;
            }

            std::string cloudstorage_dir = std::string(dll_utils::get_current_path()).append(XOR("/Settings/Cloudstorage"));

            // Create the directory if it doesn't exist
            std::filesystem::create_directories(cloudstorage_dir);

            // Create the file
            std::string clientsettings_file_name = filename.substr(0, filename.find(XOR("."))) +
                XOR("-") +
                std::to_string(api_utils::get_season_number(req.get_header_value(XOR("User-Agent")))) +
                filename.substr(filename.find(XOR(".")), filename.length());
            
            std::ofstream clientsettings;

            clientsettings.open(cloudstorage_dir + XOR("/") + clientsettings_file_name);
            clientsettings << req.body;
            clientsettings.close();

            res.status = 204;
        });
        
        in_app.Get("/fortnite/api/cloudstorage/system", [](const httplib::Request& req, httplib::Response& res)
        {
            res.set_content("[]", "application/json");
        });
    }
};