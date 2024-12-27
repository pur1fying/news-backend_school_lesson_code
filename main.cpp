#include <BAASUtil.h>
#include <httplib.h>
#include <config/BAASGlobalSetting.h>
#include <config/BAASConfig.h>
#include <sql_util.h>
using namespace nlohmann;
using namespace std;

int main() {
    try{
        system("chcp 65001");
        BAASUtil::init_globals();
        auto conn = sql_auto_connect();

        // create table
        execute_sql_file("resource/sql_util/init.sql", conn);
        init_news_data(conn);

        IMAGE_URL = global_setting->get<std::string>("image_url");

        int json_dump_size = global_setting->get<int>("return_json_dump_size", 0);
        httplib::Server svr;

        svr.Options(".*", [](const auto& req, auto& res) {
            res.set_header("Access-Control-Allow-Origin", req.get_header_value("Origin"));
            res.set_header("Allow", "GET, POST, HEAD, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "X-Requested-With, Content-Type, Accept, Origin, Authorization");
            res.set_header("Access-Control-Allow-Methods", "OPTIONS, GET, POST, HEAD");
        });

        svr.Post("/get_latest_news", [&conn, &json_dump_size](const httplib::Request& req, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", req.get_header_value("Origin"));
            res.set_header("Allow", "GET, POST, HEAD, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "X-Requested-With, Content-Type, Accept, Origin, Authorization");
            res.set_header("Access-Control-Allow-Methods", "OPTIONS, GET, POST, HEAD");
            auto ip = req.remote_addr;
            BAASConfig payload = BAASConfig(json::parse(req.body), (BAASLogger*)BAASGlobalLogger);
            auto count = payload.getSizeT("count", 10);
            auto offset = payload.getSizeT("offset", 0);
            BAASGlobalLogger->BAASInfo("Get latest news " + ip + " count: " + std::to_string(count) + " offset: " + std::to_string(offset));

            nlohmann::json ret = nlohmann::json::array();
            json temp = json::object();

            std::string query_sql;
            build_query_latest_news_sql(conn, count, offset, query_sql);
            long long t1 = BAASUtil::getCurrentTimeMS();
            std::unique_ptr<sql::ResultSet> sql_ret = sql_query(conn, query_sql);
            long long t2 = BAASUtil::getCurrentTimeMS();
            BAASGlobalLogger->BAASInfo("Query Time: " + std::to_string(t2 - t1) + "ms");
            if (sql_ret == nullptr) {
                res.set_content(ret.dump(json_dump_size), "application/json");
                return;
            }
            int id;
            while(sql_ret->next()) {
                id = sql_ret->getInt("idx");
                temp["id"] = id;
                temp["url"] = "https://www1.szu.edu.cn/board/view.asp?id=" + std::to_string(id);
                temp["title"] = sql_ret->getString("title");
                temp["date"] = sql_ret->getString("update_time");
                temp["views"] = 0;
                temp["image"] = IMAGE_URL;
                temp["author"] = "深圳大学";
                temp["department"] = sql_ret->getString("directory");
                ret.push_back(temp);
            }
            res.set_content(ret.dump(json_dump_size), "application/json");
            BAASGlobalLogger->BAASInfo("Get latest news " + ip + " count: " + std::to_string(count) + " offset: " + std::to_string(offset) + " done");
        });

        auto host = global_setting->get<std::string>("host");
        int port = global_setting->get<int>("port");

        BAASGlobalLogger->hr("Server running at " + host + ":" + std::to_string(port));
        svr.listen(host, port);

    }catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        system("pause");
    }
    return 0;
}
