create database if not exists Alumni;
USE Alumni;
# drop table if exists news;
CREATE TABLE IF NOT EXISTS news (
        idx VARCHAR(255) PRIMARY KEY,
        title TEXT,
        content TEXT,
        directory VARCHAR(255),
        category VARCHAR(255),
        create_time DATETIME,
        update_time DATETIME
) DEFAULT CHARSET=utf8mb4;
