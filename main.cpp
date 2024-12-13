#pragma comment(lib, "ws2_32.lib")
#include <BAASUtil.h>
#include <httplib.h>
#include <config/BAASGlobalSetting.h>
#include <config/BAASConfig.h>

using namespace nlohmann;
using namespace std;

int main() {
    try{
        BAASUtil::init_globals();

        BAASConfig news_resource = BAASConfig(global_setting->get<std::string>("news_file_path"), (BAASLogger*)BAASGlobalLogger);

        int json_dump_size = global_setting->get<int>("return_json_dump_size", 0);
        httplib::Server svr;

        svr.Options(".*", [](const auto& req, auto& res) {
            res.set_header("Access-Control-Allow-Origin", req.get_header_value("Origin"));
            res.set_header("Allow", "GET, POST, HEAD, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "X-Requested-With, Content-Type, Accept, Origin, Authorization");
            res.set_header("Access-Control-Allow-Methods", "OPTIONS, GET, POST, HEAD");
        });

        svr.Post("/get_latest_news", [&news_resource, &json_dump_size](const httplib::Request& req, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", req.get_header_value("Origin"));
            res.set_header("Allow", "GET, POST, HEAD, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "X-Requested-With, Content-Type, Accept, Origin, Authorization");
            res.set_header("Access-Control-Allow-Methods", "OPTIONS, GET, POST, HEAD");
            auto ip = req.remote_addr;
            BAASConfig payload = BAASConfig(json(req.body), (BAASLogger*)BAASGlobalLogger);
            int count = payload.getInt("count", 10);
            BAASGlobalLogger->BAASInfo("Get latest news " + ip + " count = " + std::to_string(count));
            nlohmann::json ret = nlohmann::json::array();
            json temp = json::object();
            for (int i = 0; i < count; i++) {
                BAASConfig one_entry = BAASConfig(news_resource.get<json>("/" + std::to_string(i)), (BAASLogger*)BAASGlobalLogger);
                temp["id"] = stoi(one_entry.get<std::string>("index"));
                temp["url"] = nullptr;
                temp["title"] = one_entry.get<std::string>("title");
                temp["date"] = one_entry.get<std::string>("update_time");
                temp["views"] = 0;
                temp["image"] = nullptr;
                temp["author"] = "深圳大学";
                temp["department"] = one_entry.get<std::string>("directory");
                ret.push_back(temp);
            }
            res.set_content(ret.dump(json_dump_size), "application/json");
            //
        });

        auto host = global_setting->get<std::string>("host");
        int port = global_setting->get<int>("port");
        
        BAASGlobalLogger->hr("Server running at " + host + ":" + std::to_string(port));
        svr.listen(host, port);

    }catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
