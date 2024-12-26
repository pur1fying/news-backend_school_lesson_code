//
// Created by pc on 2024/12/14.
//

#include <sql_util.h>
#include <BAASUtil.h>
#include <config/BAASGlobalSetting.h>
#include <sstream>
#include <nlohmann/json.hpp>

std::string project_database_name;

std::string news_table_name;

std::mutex sql_lock;

std::unique_ptr<sql::Connection> sql_auto_connect() {
    BAASGlobalLogger->hr("Sql Connect");
    const auto sql_url = global_setting->get<std::string>("/sql/url");
    const auto sql_user = global_setting->get<std::string>("/sql/username");
    const auto sql_password = global_setting->get<std::string>("/sql/password");
    project_database_name = global_setting->get<std::string>("/sql/database_name");
    news_table_name = global_setting->get<std::string>("/sql/news_table_name");
    BAASGlobalLogger->BAASInfo("URL     : " + sql_url);
    BAASGlobalLogger->BAASInfo("User    : " + sql_user);
    BAASGlobalLogger->BAASInfo("Password: " + sql_password);
    BAASGlobalLogger->BAASInfo("Database: " + project_database_name);
    try{
        sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> conn(driver->connect(sql_url, sql_user, sql_password));
        BAASGlobalLogger->BAASInfo("Connected");
        return conn;
    }
    catch (sql::SQLException &e) {
        BAASGlobalLogger->BAASError("SQL Error: " + std::string(e.what()));
        throw SQLError("SQL Error: " + std::string(e.what()));
    }
}

bool execute_sql_file(const std::string &file_path, std::unique_ptr<sql::Connection> &conn) {
    BAASGlobalLogger->BAASInfo("Execute SQL File : [ " + file_path + " ]");
    if (!std::filesystem::exists(file_path)) {
        BAASGlobalLogger->BAASError("File not exist: " + file_path);
        return false;
    }
    std::ifstream file(file_path);
    if (!file.is_open()) {
        BAASGlobalLogger->BAASError("File open failed: " + file_path);
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string sql = buffer.str();

    std::vector<std::string> sqls;
    BAASUtil::stringSplit(sql, ";", sqls);
    preprocess_sql_command(sqls);
    BAASGlobalLogger->BAASInfo("SQL Count: " + std::to_string(sqls.size()));
    std::unique_ptr<sql::Statement> stmt(conn->createStatement());
    for (auto &one_entry : sqls) {
        BAASGlobalLogger->BAASInfo("Execute : " + one_entry);
        try {
            stmt->execute(one_entry);
        }
        catch (sql::SQLException &e) {
            BAASGlobalLogger->BAASError("SQL Error: " + std::string(e.what()));
            return false;
        }
    }
    return true;
}

void preprocess_sql_command(std::vector<std::string> &sqls) {
    std::vector <std::string> new_sqls;
    for(auto &one_entry : sqls) {
        if (one_entry.empty()) continue;
        size_t pos = one_entry.find_first_not_of('\n');
        if (pos == std::string::npos)continue; // all '\n'
        one_entry.erase(0, pos);
        if (one_entry[0] == '#') continue;
        if (one_entry.empty()) continue;       // empty
        new_sqls.push_back(one_entry);
    }
    sqls = new_sqls;
}

void init_news_data(std::unique_ptr<sql::Connection> &conn) {
    auto start = BAASUtil::getCurrentTimeMS();
    BAASGlobalLogger->hr("Init News Data");
    std::unique_ptr<sql::Statement> stmt(conn->createStatement());
    stmt->execute("USE " + global_setting->get<std::string>("/sql/database_name"));
    try{
        long long t1 = BAASUtil::getCurrentTimeMS();
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT COUNT(*) FROM news"));
        res->next();
        int count = res->getInt(1);
        long long t2 = BAASUtil::getCurrentTimeMS();
        BAASGlobalLogger->BAASInfo("Total News Entries: " + std::to_string(count) + " Query Time: " + std::to_string(t2 - t1) + "ms");
    }
    catch(sql::SQLException &e) {
        BAASGlobalLogger->BAASError("SQL Error: " + std::string(e.what()));
        throw SQLError("SQL Error: " + std::string(e.what()));
    }

    BAASConfig news_json= BAASConfig(global_setting->get<std::string>("news_file_path"), (BAASLogger*)BAASGlobalLogger);
    auto generate_time = news_json.get<std::string>("/metadata/processed_time");
    BAASGlobalLogger->BAASInfo("Latest News Generate Time: " + generate_time);
    BAASConfig news_resource= BAASConfig(news_json.get<nlohmann::json>("/documents"), (BAASLogger*)BAASGlobalLogger);

    int last_time_max_idx = global_setting->get<int>("/sql/last_insert_id", 0);
    BAASGlobalLogger->BAASInfo("Last Insert Id: " + std::to_string(last_time_max_idx));
    int this_time_max_idx = last_time_max_idx;

    for(int i = 0; i < news_resource.get_config().size(); i++) {
        BAASConfig one_entry = BAASConfig(news_resource.get<nlohmann::json>("/" + std::to_string(i)), (BAASLogger*)BAASGlobalLogger);
        auto index = one_entry.get<std::string>("index");
        auto idx = std::stoi(index);
        if (idx <= last_time_max_idx) continue;
        if (idx > this_time_max_idx) this_time_max_idx = idx;

        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT * FROM news WHERE idx = " + index));
        if(res->next()) {
            BAASGlobalLogger->BAASInfo("Entry [ " + index + " ] Exist");
            continue;
        }

        auto title = one_entry.get<std::string>("title");
        auto content = one_entry.get<std::string>("content");
        auto directory = one_entry.get<std::string>("directory");
        auto category = one_entry.get<std::string>("category");
        auto create_time = one_entry.get<std::string>("create_time");
        auto update_time = one_entry.get<std::string>("update_time");

        std::string insert_sql = "INSERT INTO news (idx, title, content, directory, category, create_time, update_time) VALUES (";

        insert_sql += index + ", ";
        insert_sql += "\"" + title + "\", ";
        insert_sql += "\"" + content + "\", ";
        insert_sql += "\"" + directory + "\", ";
        insert_sql += "\"" + category + "\", ";
        insert_sql += "\"" + create_time + "\", ";
        insert_sql += "\"" + update_time + "\")";

        try {
            stmt->execute(insert_sql);
        }
        catch(sql::SQLException &e) {
            BAASGlobalLogger->BAASError("Insert Error at index : [ " + index + " ]");
            BAASGlobalLogger->BAASError("SQL Error: " + std::string(e.what()));
            continue;
        }
    }
    if (this_time_max_idx == last_time_max_idx) {
        BAASGlobalLogger->BAASInfo("No New Entry Inserted");
        return;
    }
    else {
        global_setting->update_and_save("/sql/last_insert_id", this_time_max_idx);
        BAASGlobalLogger->BAASInfo("Max Insert Id: " + std::to_string(this_time_max_idx));
        try{
            long long t1 = BAASUtil::getCurrentTimeMS();
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT COUNT(*) FROM news"));
            res->next();
            int count = res->getInt(1);
            long long t2 = BAASUtil::getCurrentTimeMS();
            BAASGlobalLogger->BAASInfo("Total News Entries: " + std::to_string(count) + " Query Time: " + std::to_string(t2 - t1) + "ms");
        }
        catch(sql::SQLException &e) {
            BAASGlobalLogger->BAASError("SQL Error: " + std::string(e.what()));
            throw SQLError("SQL Error: " + std::string(e.what()));
        }
    }
    auto end = BAASUtil::getCurrentTimeMS();
    BAASGlobalLogger->hr("Init News Data Time " + std::to_string(end - start) + "ms");
}

std::unique_ptr<sql::ResultSet> sql_query(std::unique_ptr<sql::Connection> &conn, const std::string &query_sql) {
    try{
        sql_lock.lock();
        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(query_sql));
        sql_lock.unlock();
        return res;
    }catch (sql::SQLException &e) {
        BAASGlobalLogger->BAASError("SQL Error: " + std::string(e.what()));
        sql_lock.unlock();
        return nullptr;
    }
}

void build_query_latest_news_sql(std::unique_ptr<sql::Connection> &conn, size_t count, size_t offset, std::string &query_sql) {
    query_sql = "SELECT * FROM " + news_table_name + " ORDER BY create_time DESC LIMIT " + std::to_string(count) + " OFFSET " + std::to_string(offset);
}