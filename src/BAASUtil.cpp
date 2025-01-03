//
// Created by pc on 2024/12/10
//

#include <sstream>
#include <filesystem>
#include <version.h>

#include "BAASUtil.h"
#include "config/BAASStaticConfig.h"
#include "config/BAASGlobalSetting.h"

using namespace std::chrono;
using namespace std;
using namespace std::filesystem;

string BAAS_CONFIG_DIR;

string CURRENT_TIME_STRING;

string BAAS_PROJECT_DIR;

string BAAS_OUTPUT_DIR;

string IMAGE_URL;

void BAASUtil::init_globals() {

    path curr = current_path();

    BAAS_PROJECT_DIR = curr.string();

    BAAS_OUTPUT_DIR = BAAS_PROJECT_DIR + R"(\output)";

    BAAS_CONFIG_DIR = BAAS_PROJECT_DIR + R"(\config)";

    BAASGlobalLogger = GlobalLogger::getGlobalLogger();

    BAASGlobalLogger->BAASInfo("VERSION : " + string(NEWS_SERVER_VERSION));

    BAASConfig::check_config_folder_exist();

    static_config = BAASStaticConfig::getStaticConfig();

    default_global_setting = new BAASConfig(CONFIG_TYPE_DEFAULT_GLOBAL_SETTING);

    global_setting = BAASGlobalSetting::getGlobalSetting();

    BAASGlobalLogger->BAASInfo(BAAS_PROJECT_DIR);

}

bool BAASUtil::initWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        BAASGlobalLogger->BAASError("WSAStartup failed");
        return false;
    }
    return true;
}

string BAASUtil::int2hex(int a) {
    string st;
    int length = 4;
    for (int i = 0; i < length; ++i) {
        st.push_back(static_cast<char>((a >> (length - i - 1) * 4) & 0xF));
        if (st[i] < 10) st[i] += '0';
        else st[i] += 'a' - 10;
    }
    return st;
}

string BAASUtil::int2binary(int a) {
    string st;
    for (int i = 0; i < 4; ++i)st += static_cast<char>((a >> (24 - i * 8) & 0xFF));
    return st;
}

int BAASUtil::hex2int(const string& input, int length) {
    int output = 0;
    for (int i = 0; i < length; ++i) {
        if (input[i] >= '0' && input[i] <= '9') output = output * 16 + input[i] - '0';
        else if ((input[i] >= 'A' && input[i] <= 'F') || (input[i] >= 'a' && input[i] <= 'f'))
            output = output * 16 + (input[i] & 0xF) + 9;
    }
    return output;
}

int BAASUtil::unsignedBinary2int(const string& input, int length) {
    int output = 0;
    int temp = 1;
    unsigned int t;
    for (int i = length - 1; i >= 0; --i) {
        t = input[i] & 0xFF;
        output += t * temp;
        temp *= 256;
    }
    return output;
}

int BAASUtil::binary2int(const string& input, int length) {
    int res = 0;
    int mask = 0xFF;
    for (int i = 0; i < length; ++i) {
        res |= (mask << (24 - i * 8) & (input[i] << (24 - i * 8)));
    }
    return res;
}

string BAASUtil::current_time_string() {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::tm localTime = *std::localtime(&timeT);
    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d_%H-%M-%S");
    oss << '.' << std::setw(4) << std::setfill('0') << ms.count();
    std::string formattedTime = oss.str();
    CURRENT_TIME_STRING = oss.str();
    return CURRENT_TIME_STRING;
}


FILE* BAASUtil::executeCommand(const string& command) {
    return _popen(command.c_str(), "rb");
}

string BAASUtil::getStreamOutput(FILE* stream) {
    char buffer[128];
    string output;
    while(fgets(buffer, 128, stream) != nullptr) {
        output += buffer;
    }
    return output;
}


string BAASUtil::changeEndian(int a) {
    string st;
    for (int i = 0; i < 4; ++i)st.push_back(a >> (i * 8) & 0xFF);
    return st;
}

int BAASUtil::getCurrentTimeStamp() {
    return static_cast<int>(chrono::system_clock::to_time_t(chrono::system_clock::now()));
}

string BAASUtil::int2String(int a) {
    string st = "";
    while (a) {
        st += a % 10 + '0';
        a /= 10;
    }
    return st;
}

wstring BAASUtil::stringToWString(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

string BAASUtil::wstringToString(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

bool BAASUtil::serialHost(const std::string &serial, string &host) {
    int pos = static_cast<int>(serial.find(':'));
    if(pos == string::npos) {
        host = "";
        return false;
    }
    host = serial.substr(0, pos);
    return true;
}

bool BAASUtil::serialPort(const std::string &serial, string &port) {
    int pos = static_cast<int>(serial.find(':'));
    if(pos == string::npos) {
        port = "";
        return false;
    }
    port = serial.substr(pos + 1);
    return true;
}



void BAASUtil::stringReplace(const std::string& OLD, const std::string& NEW, string &src, string &dst) {
    if(&src != &dst) {
        dst = src;
    }
    int start = 0;
    while((start = int(dst.find(OLD, start))) != string::npos) {
        dst.replace(start, OLD.length(), NEW);
        start += int(NEW.length());
    }
}

void BAASUtil::stringReplace(const string &OLD, const string &NEW, string &tar) {
    int start = 0;
    while((start = int(tar.find(OLD, start))) != string::npos) {
        tar.replace(start, OLD.length(), NEW);
        start += int(NEW.length());
    }
}

void BAASUtil::stringSplit(const string &src, const string &separator, vector<std::string> &dst) {
    dst.clear();
    int currentStart,currentEnd;
    currentStart = 0;
    currentEnd = int(src.find(separator, currentStart));
    while(currentEnd != string::npos) {
        dst.push_back(src.substr(currentStart, currentEnd - currentStart));
        currentStart = currentEnd + int(separator.length());
        currentEnd = int(src.find(separator, currentStart));
    }
    if(currentStart != currentEnd) {
        dst.push_back(src.substr(currentStart, currentEnd - currentStart));
    }
}

void BAASUtil::stringJoin(const vector<std::string> &src, const string &joiner, string &dst) {
    dst.clear();
    for(int i = 0; i < src.size(); i++) {
        dst += src[i];
        if(i != src.size() - 1) {
            dst += joiner;
        }
    }
}

std::pair<std::string, std::string> BAASUtil::serialToHostPort(const std::string &serial) {
    int pos = int(serial.find(':'));
    if(pos == string::npos) {
        return make_pair("", "");
    }
    return make_pair(serial.substr(0, pos), serial.substr(pos + 1));
}


long long BAASUtil::getCurrentTimeMS() {
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return duration.count();
}

double BAASUtil::genRandDouble(const double &min,const double &max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(min, max);
    return dis(gen);
}

int BAASUtil::genRandInt(const int &min, const int &max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(min, max);
    return dis(gen);
}

bool BAASUtil::endsWith(const string &src, const string &suffix) {
    if(src.length() < suffix.length()) return false;
    return src.substr(src.length() - suffix.length()) == suffix;
}

bool BAASUtil::allNumberChar(const string &src) {
    auto it = src.begin();
    if(*it == '-') it++;
    for(; it != src.end(); it++) {
        if((*it < '0' || *it > '9') && *it !='.') return false;
    }
    return true;
}


void BAASUtil::stringSplit(const string &src, const char separator, vector<std::string> &dst) {
    dst.clear();
    int currentStart,currentEnd;
    currentStart = 0;
    currentEnd = int(src.find(separator, currentStart));
    while(currentEnd != string::npos) {
        if(currentStart != currentEnd)dst.push_back(src.substr(currentStart, currentEnd - currentStart));
        currentStart = currentEnd + 1;
        currentEnd = int(src.find(separator, currentStart));
    }
    if(currentStart != currentEnd) {
        dst.push_back(src.substr(currentStart, currentEnd - currentStart));
    }
}

void BAASUtil::re_find_all(const string &src, const string &pattern, vector<std::string> &dst) {
    dst.clear();

    std::regex regex_pattern(pattern);
    std::smatch match;
    std::string::const_iterator search_start(src.cbegin());

    while (std::regex_search(search_start, src.cend(), match, regex_pattern)) {
        dst.push_back(match[1].str());
        search_start = match.suffix().first;
    }
}

bool BAASUtil::re_match(const string &src, const string &pattern) {
    std::regex regex_pattern(pattern);
    return std::regex_match(src, regex_pattern);
}

uint32_t BAASUtil::st2u32(const string &src) {
    uint32_t ret;
    memcpy(&ret, src.data(), sizeof(uint32_t));
    return ret;
}

void BAASUtil::insert_swipe(vector<std::pair<int, int>> &output, int start_x, int start_y, int end_x, int end_y, int step_len) {
    step_len = max(1, step_len);

    output.clear();
    output.emplace_back(start_x, start_y);
    int dis = squared_distance(start_x, start_y, end_x, end_y);
    if(dis < step_len * step_len) {
        output.emplace_back(end_x, end_y);
        return;
    }

    double total_len = sqrt(dis);
    int step_num = ceil(total_len / step_len);
    double dx = double((end_x - start_x)*1.0) / step_num;
    double dy = double((end_y - start_y)*1.0) / step_num;
    for(int i = 1; i < step_num; i++) {
        output.emplace_back(start_x + int(round(i * dx)), start_y + int(round(i * dy)));
    }

    output.emplace_back(end_x, end_y);
}

std::istream &BAASUtil::safeGetLine(istream &is, string &t) {
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf *sb = is.rdbuf();

    if (se) {
        for (;;) {
            int c = sb->sbumpc();
            switch (c) {
                case '\n':
                    return is;
                case '\r':
                    if (sb->sgetc() == '\n') sb->sbumpc();
                    return is;
                case EOF:
                    // Also handle the case when the last line has no line ending
                    if (t.empty()) is.setstate(std::ios::eofbit);
                    return is;
                default:
                    t += static_cast<char>(c);
            }
        }
    }

    return is;
}

void BAASUtil::re_find_all(const string &src, const string &pattern, vector<std::smatch> &dst) {
    std::regex regex_pattern(pattern);
    std::string::const_iterator search_start(src.cbegin());

    std::smatch match;
    while (std::regex_search(search_start, src.cend(), match, regex_pattern)) {
        dst.push_back(match);
        search_start = match.suffix().first;
    }
}

void BAASUtil::re_find(const string &src, const string &pattern, string &dst) {
    std::regex regex_pattern(pattern);
    std::smatch match;
    std::regex_search(src, match, regex_pattern);
    if(match.size() > 1) dst = match[1].str();
    else dst = "";
}

void BAASUtil::calc_swipe_params(int x1, int y1, int x2, int y2, double duration, int &step_len, double &sleep_delay) {
    int dis_squared = BAASUtil::squared_distance(x1, y1, x2, y2);
    sleep_delay = 0.005;
    if(dis_squared <= 25) step_len = 5;
    else {
        int total_steps = int(duration * 1000) / 5;
        int dis = int(sqrt(dis_squared));
        step_len = int(dis / total_steps) + 1;
        if(step_len < 5) {
            step_len = 5;
            total_steps = dis / 5;
            sleep_delay = duration / total_steps;
        }
    }
}






