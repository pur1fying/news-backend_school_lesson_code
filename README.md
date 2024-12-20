# news-backend

## Introduction
1.微信小程序深圳大学校友录 https://github.com/DBWGLX/SZU_Alumni
的后端获取新闻信息服务器

## Build
1.请使用MSVC工具链

## Config
在`/config/global_setting`中配置一些必要参数
<br>
1."/host" , "/port" : 新闻服务器的地址和端口
<br>
2."/news_file_path" : 新闻文件的路径,格式为使用news_downloader.py的输出
<br>
3."/return_json_dump_size" : 返回的json的indent大小
<br>
4."/sql/database_name" : 数据库名
<br>
5."/sql/username" : 数据库用户名
<br>
6."/sql/password" : 数据库密码
<br>
7."/sql/url" : 数据库地址

## API
API 接口: /get_latest_news
请求方法: POST
描述:
此接口根据请求中的 count 和 offset 参数，获取最新的新闻数据。它会执行 SQL 查询，从数据库中获取新闻信息，并将结果以 JSON 格式返回。响应包含新闻的 ID、标题、更新时间、所属部门、作者和图片 URL 等信息。

count（可选）：请求获取的新闻数量，默认值为 10。
<br>
offset（可选）：新闻的起始偏移量，默认值为 0。

示例请求体：
`{
"count": 10,
"offset": 0 }`



响应体:
响应将是一个 JSON 数组，包含最新新闻的详细信息。每个数组项是一个新闻对象，包含以下字段：

id: 新闻文章的 ID。

url: 新闻文章的链接地址。

title: 新闻文章的标题。

date: 新闻文章的更新时间。

views: 新闻文章的浏览量（目前固定为 0）。

image: 新闻文章相关的图片 URL。

author: 新闻文章的作者（目前固定为 "深圳大学"）。

department: 新闻文章所属的部门。

示例响应体：

日志:
记录发起请求的客户端 IP 地址。
记录请求中 count 和 offset 参数。