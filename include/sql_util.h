//
// Created by pc on 2024/12/14.
//

#ifndef BACKEND_NEWS_SERVER_SQL_UTIL_H
#define BACKEND_NEWS_SERVER_SQL_UTIL_H

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <mutex>

extern std::string project_database_name;

extern std::string news_table_name;

extern std::mutex sql_mutex;

std::unique_ptr<sql::Connection> sql_auto_connect();

bool execute_sql_file(const std::string &file_path, std::unique_ptr<sql::Connection> &conn);

void preprocess_sql_command(std::vector<std::string> &sqls);

void init_news_data(std::unique_ptr<sql::Connection> &conn);

std::unique_ptr<sql::ResultSet> sql_query(std::unique_ptr<sql::Connection> &conn, const std::string &query_sql);

void build_query_latest_news_sql(std::unique_ptr<sql::Connection> &conn, size_t count, size_t offset, std::string &query_sql);

#endif //BACKEND_NEWS_SERVER_SQL_UTIL_H

