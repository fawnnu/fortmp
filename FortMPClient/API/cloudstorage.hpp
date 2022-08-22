#pragma once

class cloudstorage
{
public:
    cloudstorage() = default;

    cloudstorage(httplib::Server& in_app)
    {   
        in_app.Get(R"(/fortnite/api/cloudstorage/user/(.*)/(.*))", [](const httplib::Request& req, httplib::Response& res)
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

            // Get correct file name
            std::string clientsettings_file_name = filename.substr(0, filename.find(XOR("."))) +
                XOR("-") +
                std::to_string(api_utils::get_season_number(req.get_header_value(XOR("User-Agent")))) +
                filename.substr(filename.find(XOR(".")), filename.length());

            if (std::filesystem::exists(cloudstorage_dir + XOR("/") + clientsettings_file_name))
            {
                res.status = 200;

                // Parse file
                std::ifstream file_to_parse;
                std::stringstream parsed_file;
                
                file_to_parse.open(cloudstorage_dir + XOR("/") + clientsettings_file_name);
                parsed_file << file_to_parse.rdbuf();
                file_to_parse.close();
                
                res.set_content(parsed_file.str(), "application/octet-stream");
            }
            else
            {
                res.status = 200;
            }
        });
        
        in_app.Get(R"(/fortnite/api/cloudstorage/user/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            std::string account_id = req.matches[1];
            
            nlohmann::json response = R"([])"_json;

            std::string cloudstorage_dir = std::string(dll_utils::get_current_path()).append(XOR("/Settings/Cloudstorage"));

            // Create the directory if it doesn't exist
            std::filesystem::create_directories(cloudstorage_dir);

            // Get correct file name
            std::string clientsettings_file_name = std::string(XOR("ClientSettings")) +
                XOR("-") +
                std::to_string(api_utils::get_season_number(req.get_header_value(XOR("User-Agent")))) +
                std::string(XOR(".Sav"));

            if (std::filesystem::exists(cloudstorage_dir + XOR("/") + clientsettings_file_name))
            {
                // Parse file
                std::ifstream file_to_parse;
                std::stringstream parsed_file;
                
                file_to_parse.open(cloudstorage_dir + XOR("/") + clientsettings_file_name);
                parsed_file << file_to_parse.rdbuf();
                file_to_parse.close();

                SHA1 sha1;
                SHA256 sha256;

                nlohmann::json file_info = {
                    {"uniqueFilename", "ClientSettings.Sav"},
                    {"filename", "ClientSettings.Sav"},
                    {"hash", sha1(parsed_file.str())},
                    {"hash256", sha256(parsed_file.str())},
                    {"length", parsed_file.str().length()},
                    {"contentType", "application/octet-stream"},
                    {"uploaded", ""},
                    {"storageType", "S3"},
                    {"storageIds", nlohmann::json({})},
                    {"accountId", account_id},
                    {"doNotCache", true}
                };

                response.push_back(file_info);

                res.set_content(response.dump(), "application/json");
            }
            else
            {
                res.set_content("[]", "application/json");   
            }
        });

        in_app.Put(R"(/fortnite/api/cloudstorage/user/(.*)/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            std::string filename = req.matches[2];

            if (filename != XOR("ClientSettings.Sav"))
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