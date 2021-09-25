//
// Created by yuwenyong.vincent on 2021/8/22.
//

#include "EasyEvent/HttpUtil/UrlParse.h"
#include "EasyEvent/Common/StrUtil.h"


const char * EasyEvent::UrlErrorCategory::name() const noexcept {
    return "url error";
}

std::string EasyEvent::UrlErrorCategory::message(int ev) const {
    switch (static_cast<UrlErrors>(ev)) {
        case UrlErrors::InvalidIPv6Url:
            return "Invalid IPv6 url";
        case UrlErrors::InvalidSeparator:
            return "Invalid separator";
        case UrlErrors::BadQueryField:
            return "Bad query field";
        default:
            return "unknown url error";
    }
}

const std::error_category& EasyEvent::getUrlErrorCategory() {
    static const UrlErrorCategory errCategory{};
    return errCategory;
}


std::string EasyEvent::NetlocResult::getUserName() const {
    const std::string& netloc = getNetloc();
    std::string_view userInfo, haveInfo, hostInfo;
    std::tie(userInfo, haveInfo, hostInfo) = StrUtil::rpartition(std::string_view(netloc), "@");
    if (!haveInfo.empty()) {
        std::string_view userName, havePassword, password;
        std::tie(userName, havePassword, password) = StrUtil::partition(userInfo, ":");
        return std::string(userName.data(), userName.size());
    } else {
        return std::string {};
    }
}

std::string EasyEvent::NetlocResult::getPassword() const {
    const std::string& netloc = getNetloc();
    std::string_view userInfo, haveInfo, hostInfo;
    std::tie(userInfo, haveInfo, hostInfo) = StrUtil::rpartition(std::string_view(netloc), "@");
    if (!haveInfo.empty()) {
        std::string_view userName, havePassword, password;
        std::tie(userName, havePassword, password) = StrUtil::partition(userInfo, ":");
        if (havePassword.empty()) {
            return std::string {};
        } else {
            return std::string(password.data(), password.size());
        }
    }
    return std::string {};
}

std::string EasyEvent::NetlocResult::getHostName() const {
    const std::string& netloc = getNetloc();
    std::string_view hostInfo;
    std::tie(std::ignore, std::ignore, hostInfo) = StrUtil::rpartition(std::string_view(netloc), "@");
    std::string_view haveOpenBr, bracketed;
    std::tie(std::ignore, haveOpenBr, bracketed) = StrUtil::partition(hostInfo, "[");
    std::string_view hostName, port;
    if (!haveOpenBr.empty()) {
        std::tie(hostName, std::ignore, port) = StrUtil::partition(bracketed, "]");
    } else {
        std::tie(hostName, std::ignore, port) = StrUtil::partition(hostInfo, ":");
    }
    return std::string(hostName.data(), hostName.size());
}

int EasyEvent::NetlocResult::getPort() const {
    const std::string& netloc = getNetloc();
    std::string_view hostInfo;
    std::tie(std::ignore, std::ignore, hostInfo) = StrUtil::rpartition(std::string_view(netloc), "@");
    std::string_view haveOpenBr, bracketed;
    std::tie(std::ignore, haveOpenBr, bracketed) = StrUtil::partition(hostInfo, "[");
    std::string_view hostName, port;
    if (!haveOpenBr.empty()) {
        std::tie(hostName, std::ignore, port) = StrUtil::partition(bracketed, "]");
        std::tie(std::ignore, std::ignore, port) = StrUtil::partition(port, ":");
    } else {
        std::tie(hostName, std::ignore, port) = StrUtil::partition(hostInfo, ":");
    }
    int retVal;
    try {
        retVal = std::stoi(std::string(port.data(), port.size()));
        if (retVal < 0 || retVal > 65535) {
            retVal = -1;
        }
    } catch (...) {
        retVal = -1;
    }
    return retVal;
}

std::string EasyEvent::SplitResult::getUrl() const {
    return UrlParse::urlUnsplit(*this);
}

std::string EasyEvent::ParseResult::getUrl() const {
    return UrlParse::urlUnparse(*this);
}

const StringViewSet EasyEvent::UrlParse::UsesRelative = {
    "", "ftp", "http", "gopher", "nntp", "imap",
    "wais", "file", "https", "shttp", "mms",
    "prospero", "rtsp", "rtspu",  "sftp",
    "svn", "svn+ssh", "ws", "wss"
};

const StringViewSet EasyEvent::UrlParse::UsesNetloc = {
    "", "ftp", "http", "gopher", "nntp", "telnet",
    "imap", "wais", "file", "mms", "https", "shttp",
    "snews", "prospero", "rtsp", "rtspu", "rsync",
    "svn", "svn+ssh", "sftp", "nfs", "git", "git+ssh",
    "ws", "wss"
};

const StringViewSet EasyEvent::UrlParse::UsesParams = {
    "", "ftp", "hdl", "prospero", "http", "imap",
    "https", "shttp", "rtsp", "rtspu", "sip", "sips",
    "mms", "sftp", "tel", //"ws", "wss"
};

const std::string_view EasyEvent::UrlParse::SchemeChars  = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+-.";

const std::map<std::string_view, char> EasyEvent::UrlParse::HexToChar = {
        {"00", '\x00'},  {"01", '\x01'},  {"02", '\x02'},  {"03", '\x03'},  {"04", '\x04'},  {"05", '\x05'},
        {"06", '\x06'},  {"07", '\x07'},  {"08", '\x08'},  {"09", '\x09'},  {"0A", '\x0a'},  {"0B", '\x0b'},
        {"0C", '\x0c'},  {"0D", '\x0d'},  {"0E", '\x0e'},  {"0F", '\x0f'},  {"0a", '\x0a'},  {"0b", '\x0b'},
        {"0c", '\x0c'},  {"0d", '\x0d'},  {"0e", '\x0e'},  {"0f", '\x0f'},  {"10", '\x10'},  {"11", '\x11'},
        {"12", '\x12'},  {"13", '\x13'},  {"14", '\x14'},  {"15", '\x15'},  {"16", '\x16'},  {"17", '\x17'},
        {"18", '\x18'},  {"19", '\x19'},  {"1A", '\x1a'},  {"1B", '\x1b'},  {"1C", '\x1c'},  {"1D", '\x1d'},
        {"1E", '\x1e'},  {"1F", '\x1f'},  {"1a", '\x1a'},  {"1b", '\x1b'},  {"1c", '\x1c'},  {"1d", '\x1d'},
        {"1e", '\x1e'},  {"1f", '\x1f'},  {"20", '\x20'},  {"21", '\x21'},  {"22", '\x22'},  {"23", '\x23'},
        {"24", '\x24'},  {"25", '\x25'},  {"26", '\x26'},  {"27", '\x27'},  {"28", '\x28'},  {"29", '\x29'},
        {"2A", '\x2a'},  {"2B", '\x2b'},  {"2C", '\x2c'},  {"2D", '\x2d'},  {"2E", '\x2e'},  {"2F", '\x2f'},
        {"2a", '\x2a'},  {"2b", '\x2b'},  {"2c", '\x2c'},  {"2d", '\x2d'},  {"2e", '\x2e'},  {"2f", '\x2f'},
        {"30", '\x30'},  {"31", '\x31'},  {"32", '\x32'},  {"33", '\x33'},  {"34", '\x34'},  {"35", '\x35'},
        {"36", '\x36'},  {"37", '\x37'},  {"38", '\x38'},  {"39", '\x39'},  {"3A", '\x3a'},  {"3B", '\x3b'},
        {"3C", '\x3c'},  {"3D", '\x3d'},  {"3E", '\x3e'},  {"3F", '\x3f'},  {"3a", '\x3a'},  {"3b", '\x3b'},
        {"3c", '\x3c'},  {"3d", '\x3d'},  {"3e", '\x3e'},  {"3f", '\x3f'},  {"40", '\x40'},  {"41", '\x41'},
        {"42", '\x42'},  {"43", '\x43'},  {"44", '\x44'},  {"45", '\x45'},  {"46", '\x46'},  {"47", '\x47'},
        {"48", '\x48'},  {"49", '\x49'},  {"4A", '\x4a'},  {"4B", '\x4b'},  {"4C", '\x4c'},  {"4D", '\x4d'},
        {"4E", '\x4e'},  {"4F", '\x4f'},  {"4a", '\x4a'},  {"4b", '\x4b'},  {"4c", '\x4c'},  {"4d", '\x4d'},
        {"4e", '\x4e'},  {"4f", '\x4f'},  {"50", '\x50'},  {"51", '\x51'},  {"52", '\x52'},  {"53", '\x53'},
        {"54", '\x54'},  {"55", '\x55'},  {"56", '\x56'},  {"57", '\x57'},  {"58", '\x58'},  {"59", '\x59'},
        {"5A", '\x5a'},  {"5B", '\x5b'},  {"5C", '\x5c'},  {"5D", '\x5d'},  {"5E", '\x5e'},  {"5F", '\x5f'},
        {"5a", '\x5a'},  {"5b", '\x5b'},  {"5c", '\x5c'},  {"5d", '\x5d'},  {"5e", '\x5e'},  {"5f", '\x5f'},
        {"60", '\x60'},  {"61", '\x61'},  {"62", '\x62'},  {"63", '\x63'},  {"64", '\x64'},  {"65", '\x65'},
        {"66", '\x66'},  {"67", '\x67'},  {"68", '\x68'},  {"69", '\x69'},  {"6A", '\x6a'},  {"6B", '\x6b'},
        {"6C", '\x6c'},  {"6D", '\x6d'},  {"6E", '\x6e'},  {"6F", '\x6f'},  {"6a", '\x6a'},  {"6b", '\x6b'},
        {"6c", '\x6c'},  {"6d", '\x6d'},  {"6e", '\x6e'},  {"6f", '\x6f'},  {"70", '\x70'},  {"71", '\x71'},
        {"72", '\x72'},  {"73", '\x73'},  {"74", '\x74'},  {"75", '\x75'},  {"76", '\x76'},  {"77", '\x77'},
        {"78", '\x78'},  {"79", '\x79'},  {"7A", '\x7a'},  {"7B", '\x7b'},  {"7C", '\x7c'},  {"7D", '\x7d'},
        {"7E", '\x7e'},  {"7F", '\x7f'},  {"7a", '\x7a'},  {"7b", '\x7b'},  {"7c", '\x7c'},  {"7d", '\x7d'},
        {"7e", '\x7e'},  {"7f", '\x7f'},  {"80", '\x80'},  {"81", '\x81'},  {"82", '\x82'},  {"83", '\x83'},
        {"84", '\x84'},  {"85", '\x85'},  {"86", '\x86'},  {"87", '\x87'},  {"88", '\x88'},  {"89", '\x89'},
        {"8A", '\x8a'},  {"8B", '\x8b'},  {"8C", '\x8c'},  {"8D", '\x8d'},  {"8E", '\x8e'},  {"8F", '\x8f'},
        {"8a", '\x8a'},  {"8b", '\x8b'},  {"8c", '\x8c'},  {"8d", '\x8d'},  {"8e", '\x8e'},  {"8f", '\x8f'},
        {"90", '\x90'},  {"91", '\x91'},  {"92", '\x92'},  {"93", '\x93'},  {"94", '\x94'},  {"95", '\x95'},
        {"96", '\x96'},  {"97", '\x97'},  {"98", '\x98'},  {"99", '\x99'},  {"9A", '\x9a'},  {"9B", '\x9b'},
        {"9C", '\x9c'},  {"9D", '\x9d'},  {"9E", '\x9e'},  {"9F", '\x9f'},  {"9a", '\x9a'},  {"9b", '\x9b'},
        {"9c", '\x9c'},  {"9d", '\x9d'},  {"9e", '\x9e'},  {"9f", '\x9f'},  {"A0", '\xa0'},  {"A1", '\xa1'},
        {"A2", '\xa2'},  {"A3", '\xa3'},  {"A4", '\xa4'},  {"A5", '\xa5'},  {"A6", '\xa6'},  {"A7", '\xa7'},
        {"A8", '\xa8'},  {"A9", '\xa9'},  {"AA", '\xaa'},  {"AB", '\xab'},  {"AC", '\xac'},  {"AD", '\xad'},
        {"AE", '\xae'},  {"AF", '\xaf'},  {"Aa", '\xaa'},  {"Ab", '\xab'},  {"Ac", '\xac'},  {"Ad", '\xad'},
        {"Ae", '\xae'},  {"Af", '\xaf'},  {"B0", '\xb0'},  {"B1", '\xb1'},  {"B2", '\xb2'},  {"B3", '\xb3'},
        {"B4", '\xb4'},  {"B5", '\xb5'},  {"B6", '\xb6'},  {"B7", '\xb7'},  {"B8", '\xb8'},  {"B9", '\xb9'},
        {"BA", '\xba'},  {"BB", '\xbb'},  {"BC", '\xbc'},  {"BD", '\xbd'},  {"BE", '\xbe'},  {"BF", '\xbf'},
        {"Ba", '\xba'},  {"Bb", '\xbb'},  {"Bc", '\xbc'},  {"Bd", '\xbd'},  {"Be", '\xbe'},  {"Bf", '\xbf'},
        {"C0", '\xc0'},  {"C1", '\xc1'},  {"C2", '\xc2'},  {"C3", '\xc3'},  {"C4", '\xc4'},  {"C5", '\xc5'},
        {"C6", '\xc6'},  {"C7", '\xc7'},  {"C8", '\xc8'},  {"C9", '\xc9'},  {"CA", '\xca'},  {"CB", '\xcb'},
        {"CC", '\xcc'},  {"CD", '\xcd'},  {"CE", '\xce'},  {"CF", '\xcf'},  {"Ca", '\xca'},  {"Cb", '\xcb'},
        {"Cc", '\xcc'},  {"Cd", '\xcd'},  {"Ce", '\xce'},  {"Cf", '\xcf'},  {"D0", '\xd0'},  {"D1", '\xd1'},
        {"D2", '\xd2'},  {"D3", '\xd3'},  {"D4", '\xd4'},  {"D5", '\xd5'},  {"D6", '\xd6'},  {"D7", '\xd7'},
        {"D8", '\xd8'},  {"D9", '\xd9'},  {"DA", '\xda'},  {"DB", '\xdb'},  {"DC", '\xdc'},  {"DD", '\xdd'},
        {"DE", '\xde'},  {"DF", '\xdf'},  {"Da", '\xda'},  {"Db", '\xdb'},  {"Dc", '\xdc'},  {"Dd", '\xdd'},
        {"De", '\xde'},  {"Df", '\xdf'},  {"E0", '\xe0'},  {"E1", '\xe1'},  {"E2", '\xe2'},  {"E3", '\xe3'},
        {"E4", '\xe4'},  {"E5", '\xe5'},  {"E6", '\xe6'},  {"E7", '\xe7'},  {"E8", '\xe8'},  {"E9", '\xe9'},
        {"EA", '\xea'},  {"EB", '\xeb'},  {"EC", '\xec'},  {"ED", '\xed'},  {"EE", '\xee'},  {"EF", '\xef'},
        {"Ea", '\xea'},  {"Eb", '\xeb'},  {"Ec", '\xec'},  {"Ed", '\xed'},  {"Ee", '\xee'},  {"Ef", '\xef'},
        {"F0", '\xf0'},  {"F1", '\xf1'},  {"F2", '\xf2'},  {"F3", '\xf3'},  {"F4", '\xf4'},  {"F5", '\xf5'},
        {"F6", '\xf6'},  {"F7", '\xf7'},  {"F8", '\xf8'},  {"F9", '\xf9'},  {"FA", '\xfa'},  {"FB", '\xfb'},
        {"FC", '\xfc'},  {"FD", '\xfd'},  {"FE", '\xfe'},  {"FF", '\xff'},  {"Fa", '\xfa'},  {"Fb", '\xfb'},
        {"Fc", '\xfc'},  {"Fd", '\xfd'},  {"Fe", '\xfe'},  {"Ff", '\xff'},  {"a0", '\xa0'},  {"a1", '\xa1'},
        {"a2", '\xa2'},  {"a3", '\xa3'},  {"a4", '\xa4'},  {"a5", '\xa5'},  {"a6", '\xa6'},  {"a7", '\xa7'},
        {"a8", '\xa8'},  {"a9", '\xa9'},  {"aA", '\xaa'},  {"aB", '\xab'},  {"aC", '\xac'},  {"aD", '\xad'},
        {"aE", '\xae'},  {"aF", '\xaf'},  {"aa", '\xaa'},  {"ab", '\xab'},  {"ac", '\xac'},  {"ad", '\xad'},
        {"ae", '\xae'},  {"af", '\xaf'},  {"b0", '\xb0'},  {"b1", '\xb1'},  {"b2", '\xb2'},  {"b3", '\xb3'},
        {"b4", '\xb4'},  {"b5", '\xb5'},  {"b6", '\xb6'},  {"b7", '\xb7'},  {"b8", '\xb8'},  {"b9", '\xb9'},
        {"bA", '\xba'},  {"bB", '\xbb'},  {"bC", '\xbc'},  {"bD", '\xbd'},  {"bE", '\xbe'},  {"bF", '\xbf'},
        {"ba", '\xba'},  {"bb", '\xbb'},  {"bc", '\xbc'},  {"bd", '\xbd'},  {"be", '\xbe'},  {"bf", '\xbf'},
        {"c0", '\xc0'},  {"c1", '\xc1'},  {"c2", '\xc2'},  {"c3", '\xc3'},  {"c4", '\xc4'},  {"c5", '\xc5'},
        {"c6", '\xc6'},  {"c7", '\xc7'},  {"c8", '\xc8'},  {"c9", '\xc9'},  {"cA", '\xca'},  {"cB", '\xcb'},
        {"cC", '\xcc'},  {"cD", '\xcd'},  {"cE", '\xce'},  {"cF", '\xcf'},  {"ca", '\xca'},  {"cb", '\xcb'},
        {"cc", '\xcc'},  {"cd", '\xcd'},  {"ce", '\xce'},  {"cf", '\xcf'},  {"d0", '\xd0'},  {"d1", '\xd1'},
        {"d2", '\xd2'},  {"d3", '\xd3'},  {"d4", '\xd4'},  {"d5", '\xd5'},  {"d6", '\xd6'},  {"d7", '\xd7'},
        {"d8", '\xd8'},  {"d9", '\xd9'},  {"dA", '\xda'},  {"dB", '\xdb'},  {"dC", '\xdc'},  {"dD", '\xdd'},
        {"dE", '\xde'},  {"dF", '\xdf'},  {"da", '\xda'},  {"db", '\xdb'},  {"dc", '\xdc'},  {"dd", '\xdd'},
        {"de", '\xde'},  {"df", '\xdf'},  {"e0", '\xe0'},  {"e1", '\xe1'},  {"e2", '\xe2'},  {"e3", '\xe3'},
        {"e4", '\xe4'},  {"e5", '\xe5'},  {"e6", '\xe6'},  {"e7", '\xe7'},  {"e8", '\xe8'},  {"e9", '\xe9'},
        {"eA", '\xea'},  {"eB", '\xeb'},  {"eC", '\xec'},  {"eD", '\xed'},  {"eE", '\xee'},  {"eF", '\xef'},
        {"ea", '\xea'},  {"eb", '\xeb'},  {"ec", '\xec'},  {"ed", '\xed'},  {"ee", '\xee'},  {"ef", '\xef'},
        {"f0", '\xf0'},  {"f1", '\xf1'},  {"f2", '\xf2'},  {"f3", '\xf3'},  {"f4", '\xf4'},  {"f5", '\xf5'},
        {"f6", '\xf6'},  {"f7", '\xf7'},  {"f8", '\xf8'},  {"f9", '\xf9'},  {"fA", '\xfa'},  {"fB", '\xfb'},
        {"fC", '\xfc'},  {"fD", '\xfd'},  {"fE", '\xfe'},  {"fF", '\xff'},  {"fa", '\xfa'},  {"fb", '\xfb'},
        {"fc", '\xfc'},  {"fd", '\xfd'},  {"fe", '\xfe'},  {"ff", '\xff'},
};

const std::string_view EasyEvent::UrlParse::AlwaysSafe = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.-~";

const std::map<char, std::string_view> EasyEvent::UrlParse::SafeMap = {
        {'\x00', "%00"},  {'\x83', "%83"},  {'\x04', "%04"},  {'\x87', "%87"},  {'\x08', "%08"},  {'\x8b', "%8B"},
        {'\x0c', "%0C"},  {'\x8f', "%8F"},  {'\x10', "%10"},  {'\x93', "%93"},  {'\x14', "%14"},  {'\x97', "%97"},
        {'\x18', "%18"},  {'\x9b', "%9B"},  {'\x1c', "%1C"},  {'\x9f', "%9F"},  {'\x20', "%20"},  {'\xa3', "%A3"},
        {'\x24', "%24"},  {'\xa7', "%A7"},  {'\x28', "%28"},  {'\xab', "%AB"},  {'\x2c', "%2C"},  {'\xaf', "%AF"},
        {'\x30', "0"},  {'\xb3', "%B3"},  {'\x34', "4"},  {'\xb7', "%B7"},  {'\x38', "8"},  {'\xbb', "%BB"},
        {'\x3c', "%3C"},  {'\xbf', "%BF"},  {'\x40', "%40"},  {'\xc3', "%C3"},  {'\x44', "D"},  {'\xc7', "%C7"},
        {'\x48', "H"},  {'\xcb', "%CB"},  {'\x4c', "L"},  {'\xcf', "%CF"},  {'\x50', "P"},  {'\xd3', "%D3"},
        {'\x54', "T"},  {'\xd7', "%D7"},  {'\x58', "X"},  {'\xdb', "%DB"},  {'\x5c', "%5C"},  {'\xdf', "%DF"},
        {'\x60', "%60"},  {'\xe3', "%E3"},  {'\x64', "d"},  {'\xe7', "%E7"},  {'\x68', "h"},  {'\xeb', "%EB"},
        {'\x6c', "l"},  {'\xef', "%EF"},  {'\x70', "p"},  {'\xf3', "%F3"},  {'\x74', "t"},  {'\xf7', "%F7"},
        {'\x78', "x"},  {'\xfb', "%FB"},  {'\x7c', "%7C"},  {'\xff', "%FF"},  {'\x80', "%80"},  {'\x03', "%03"},
        {'\x84', "%84"},  {'\x07', "%07"},  {'\x88', "%88"},  {'\x0b', "%0B"},  {'\x8c', "%8C"},  {'\x0f', "%0F"},
        {'\x90', "%90"},  {'\x13', "%13"},  {'\x94', "%94"},  {'\x17', "%17"},  {'\x98', "%98"},  {'\x1b', "%1B"},
        {'\x9c', "%9C"},  {'\x1f', "%1F"},  {'\xa0', "%A0"},  {'\x23', "%23"},  {'\xa4', "%A4"},  {'\x27', "%27"},
        {'\xa8', "%A8"},  {'\x2b', "%2B"},  {'\xac', "%AC"},  {'\x2f', "%2F"},  {'\xb0', "%B0"},  {'\x33', "3"},
        {'\xb4', "%B4"},  {'\x37', "7"},  {'\xb8', "%B8"},  {'\x3b', "%3B"},  {'\xbc', "%BC"},  {'\x3f', "%3F"},
        {'\xc0', "%C0"},  {'\x43', "C"},  {'\xc4', "%C4"},  {'\x47', "G"},  {'\xc8', "%C8"},  {'\x4b', "K"},
        {'\xcc', "%CC"},  {'\x4f', "O"},  {'\xd0', "%D0"},  {'\x53', "S"},  {'\xd4', "%D4"},  {'\x57', "W"},
        {'\xd8', "%D8"},  {'\x5b', "%5B"},  {'\xdc', "%DC"},  {'\x5f', "_"},  {'\xe0', "%E0"},  {'\x63', "c"},
        {'\xe4', "%E4"},  {'\x67', "g"},  {'\xe8', "%E8"},  {'\x6b', "k"},  {'\xec', "%EC"},  {'\x6f', "o"},
        {'\xf0', "%F0"},  {'\x73', "s"},  {'\xf4', "%F4"},  {'\x77', "w"},  {'\xf8', "%F8"},  {'\x7b', "%7B"},
        {'\xfc', "%FC"},  {'\x7f', "%7F"},  {'\x81', "%81"},  {'\x02', "%02"},  {'\x85', "%85"},  {'\x06', "%06"},
        {'\x89', "%89"},  {'\x0a', "%0A"},  {'\x8d', "%8D"},  {'\x0e', "%0E"},  {'\x91', "%91"},  {'\x12', "%12"},
        {'\x95', "%95"},  {'\x16', "%16"},  {'\x99', "%99"},  {'\x1a', "%1A"},  {'\x9d', "%9D"},  {'\x1e', "%1E"},
        {'\xa1', "%A1"},  {'\x22', "%22"},  {'\xa5', "%A5"},  {'\x26', "%26"},  {'\xa9', "%A9"},  {'\x2a', "%2A"},
        {'\xad', "%AD"},  {'\x2e', "."},  {'\xb1', "%B1"},  {'\x32', "2"},  {'\xb5', "%B5"},  {'\x36', "6"},
        {'\xb9', "%B9"},  {'\x3a', "%3A"},  {'\xbd', "%BD"},  {'\x3e', "%3E"},  {'\xc1', "%C1"},  {'\x42', "B"},
        {'\xc5', "%C5"},  {'\x46', "F"},  {'\xc9', "%C9"},  {'\x4a', "J"},  {'\xcd', "%CD"},  {'\x4e', "N"},
        {'\xd1', "%D1"},  {'\x52', "R"},  {'\xd5', "%D5"},  {'\x56', "V"},  {'\xd9', "%D9"},  {'\x5a', "Z"},
        {'\xdd', "%DD"},  {'\x5e', "%5E"},  {'\xe1', "%E1"},  {'\x62', "b"},  {'\xe5', "%E5"},  {'\x66', "f"},
        {'\xe9', "%E9"},  {'\x6a', "j"},  {'\xed', "%ED"},  {'\x6e', "n"},  {'\xf1', "%F1"},  {'\x72', "r"},
        {'\xf5', "%F5"},  {'\x76', "v"},  {'\xf9', "%F9"},  {'\x7a', "z"},  {'\xfd', "%FD"},  {'\x7e', "%7E"},
        {'\x01', "%01"},  {'\x82', "%82"},  {'\x05', "%05"},  {'\x86', "%86"},  {'\x09', "%09"},  {'\x8a', "%8A"},
        {'\x0d', "%0D"},  {'\x8e', "%8E"},  {'\x11', "%11"},  {'\x92', "%92"},  {'\x15', "%15"},  {'\x96', "%96"},
        {'\x19', "%19"},  {'\x9a', "%9A"},  {'\x1d', "%1D"},  {'\x9e', "%9E"},  {'\x21', "%21"},  {'\xa2', "%A2"},
        {'\x25', "%25"},  {'\xa6', "%A6"},  {'\x29', "%29"},  {'\xaa', "%AA"},  {'\x2d', "-"},  {'\xae', "%AE"},
        {'\x31', "1"},  {'\xb2', "%B2"},  {'\x35', "5"},  {'\xb6', "%B6"},  {'\x39', "9"},  {'\xba', "%BA"},
        {'\x3d', "%3D"},  {'\xbe', "%BE"},  {'\x41', "A"},  {'\xc2', "%C2"},  {'\x45', "E"},  {'\xc6', "%C6"},
        {'\x49', "I"},  {'\xca', "%CA"},  {'\x4d', "M"},  {'\xce', "%CE"},  {'\x51', "Q"},  {'\xd2', "%D2"},
        {'\x55', "U"},  {'\xd6', "%D6"},  {'\x59', "Y"},  {'\xda', "%DA"},  {'\x5d', "%5D"},  {'\xde', "%DE"},
        {'\x61', "a"},  {'\xe2', "%E2"},  {'\x65', "e"},  {'\xe6', "%E6"},  {'\x69', "i"},  {'\xea', "%EA"},
        {'\x6d', "m"},  {'\xee', "%EE"},  {'\x71', "q"},  {'\xf2', "%F2"},  {'\x75', "u"},  {'\xf6', "%F6"},
        {'\x79', "y"},  {'\xfa', "%FA"},  {'\x7d', "%7D"},  {'\xfe', "%FE"}
};

EasyEvent::ParseResult EasyEvent::UrlParse::urlParse(std::string url, std::string schema, bool allowFragments) {
    std::string netloc, query, fragment, params;
    auto splitResult = urlSplit(std::move(url), std::move(schema), allowFragments);
    schema = splitResult.getScheme();
    netloc = splitResult.getNetloc();
    url = splitResult.getPath();
    query = splitResult.getQuery();
    fragment = splitResult.getFragment();
    if (UsesParams.find(schema) != UsesParams.end() && url.find(';') != std::string::npos) {
        std::string_view svUrl, svParams;
        std::tie(svUrl, svParams) = splitParams(url);
        url.assign(svUrl.data(), svUrl.size());
        params.assign(svParams.data(), svParams.size());
    }
    return ParseResult(std::move(schema), std::move(netloc), std::move(url), std::move(params), std::move(query),
                       std::move(fragment));
}

EasyEvent::SplitResult EasyEvent::UrlParse::urlSplit(std::string url, std::string schema, bool allowFragments) {
    std::string netloc, query, fragment;
    std::string::size_type pos;
    std::string::size_type i = url.find(':');
    if (i != std::string::npos) {
        std::string_view tempUrl(url.data(), i);
        pos = tempUrl.find_first_not_of(SchemeChars);
        if (pos == std::string_view::npos) {
            schema = StrUtil::toLowerCopy(tempUrl);
            url = url.substr(i + 1);
        }
    }
    if (StrUtil::startsWith(url, "//")) {
        std::string_view svNetloc, svUrl;
        std::tie(svNetloc, svUrl) = splitNetloc(url, 2);
        bool leftBracketFound = svNetloc.find('[') != std::string_view::npos;
        bool rightBracketFound = svNetloc.find(']') != std::string_view::npos;
        if (leftBracketFound != rightBracketFound) {
            throwError(UrlErrors::InvalidIPv6Url, "urlSplit");
        }
        netloc.assign(svNetloc.data(), svNetloc.size());
        url.assign(svUrl.data(), svUrl.size());
    }
    if (allowFragments) {
        pos = url.find('#');
        if (pos != std::string::npos) {
            fragment = url.substr(pos + 1);
            url = url.substr(0, pos);
        }
    }
    pos = url.find('?');
    if (pos != std::string::npos) {
        query = url.substr(pos + 1);
        url = url.substr(0, pos);
    }
    return SplitResult(std::move(schema), std::move(netloc), std::move(url), std::move(query), std::move(fragment));
}

std::string EasyEvent::UrlParse::urlUnparse(const ParseResult &components) {
    const std::string &scheme = components.getScheme();
    const std::string &netloc = components.getNetloc();
    std::string url = components.getPath();
    const std::string &params = components.getParams();
    const std::string &query = components.getQuery();
    const std::string &fragment = components.getFragment();
    if (!params.empty()) {
        url += ';' + params;
    }
    return urlUnsplit(SplitResult(scheme, netloc, std::move(url), query, fragment));
}

std::string EasyEvent::UrlParse::urlUnsplit(const SplitResult &components) {
    std::string url = components.getPath();
    const std::string& scheme = components.getScheme();
    const std::string& netloc = components.getNetloc();
    const std::string& query = components.getQuery();
    const std::string& fragment = components.getFragment();
    if (!netloc.empty() ||
        (!scheme.empty() && UsesNetloc.find(scheme) != UsesNetloc.end() && !StrUtil::startsWith(url, "//"))) {
        if (!url.empty() && url[0] != '/') {
            url = '/' + url;
        }
        url = "//" + netloc + url;
    }
    if (!scheme.empty()) {
        url = scheme + ':' + url;
    }
    if (!query.empty()) {
        url += '?' + query;
    }
    if (!fragment.empty()) {
        url += '#' + fragment;
    }
    return url;
}

std::string EasyEvent::UrlParse::urlJoin(const std::string &base, const std::string &url, bool allowFragments) {
    if (base.empty()) {
        return url;
    }
    if (url.empty()) {
        return base;
    }
    std::string bscheme, bnetloc, bpath, bparams, bquery, bfragment;
    ParseResult bresult = urlParse(base, "", allowFragments);
    bscheme = bresult.getScheme();
    bnetloc = bresult.getNetloc();
    bpath = bresult.getPath();
    bparams = bresult.getParams();
    bquery = bresult.getQuery();
    //bfragment = bresult.getFragment();

    std::string scheme, netloc, path, params, query, fragment;
    ParseResult result = urlParse(url, bscheme, allowFragments);
    scheme = result.getScheme();
    netloc = result.getNetloc();
    path = result.getPath();
    params = result.getParams();
    fragment = result.getFragment();
    if (scheme != bscheme || UsesRelative.find(scheme) == UsesRelative.end()) {
        return url;
    }
    if (UsesNetloc.find(scheme) != UsesNetloc.end()) {
        if (!netloc.empty()) {
            return urlUnparse(ParseResult(std::move(scheme), std::move(netloc), std::move(path), std::move(params),
                                          std::move(query), std::move(fragment)));
        }
        netloc = std::move(bnetloc);
    }

    if (path.empty() && params.empty()) {
        path = std::move(bpath);
        params = std::move(bparams);
        if (query.empty()) {
            query = std::move(bquery);
        }
        return urlUnparse(ParseResult(std::move(scheme), std::move(netloc), std::move(path), std::move(params),
                                      std::move(query), std::move(fragment)));
    }

    StringViewVec segments;
    if (!path.empty() && path.front() == '/') {
        segments = StrUtil::split(std::string_view(path), '/');
    } else {
        StringViewVec baseParts = StrUtil::split(std::string_view(bpath), '/');
        if (!baseParts.back().empty()) {
            baseParts.pop_back();
        }

        auto parts = StrUtil::split(std::string_view(path), '/');
        for (auto iter = baseParts.begin(); iter != baseParts.end(); ++iter) {
            if (iter == baseParts.begin() || !iter->empty()) {
                segments.emplace_back(*iter);
            }
        }
        for (auto iter = parts.begin(); iter != parts.end(); ++iter) {
            if (std::next(iter) == parts.end() || !iter->empty()) {
                segments.emplace_back(*iter);
            }
        }
    }

    StringViewVec resolvedPath;

    for (auto& seg: segments) {
        if (seg == "..") {
            if (!resolvedPath.empty()) {
                resolvedPath.pop_back();
            }
        } else if (seg == ".") {
            continue;
        } else {
            resolvedPath.emplace_back(seg);
        }
    }

    if (segments.back() == "." || segments.back() == "..") {
        resolvedPath.emplace_back("");
    }

    if (!resolvedPath.empty()) {
        path = StrUtil::join(resolvedPath, "/");
    } else {
        path = "/";
    }

    return urlUnparse(ParseResult(std::move(scheme), std::move(netloc), std::move(path),
                                  std::move(params), std::move(query), std::move(fragment)));
}

EasyEvent::DefragResult EasyEvent::UrlParse::urlDefrag(const std::string &url) {
    std::string defrag, frag;
    if (url.find('#') != std::string::npos) {
        std::string s, n, p, a, q;
        ParseResult parseResult = urlParse(url);
        s = parseResult.getScheme();
        n = parseResult.getNetloc();
        p = parseResult.getPath();
        a = parseResult.getParams();
        q = parseResult.getQuery();
        frag = parseResult.getFragment();
        defrag = urlUnparse(ParseResult(std::move(s), std::move(n), std::move(p), std::move(a),
                                                       std::move(q), ""));
    } else {
        defrag = url;
    }
    return DefragResult(std::move(defrag), std::move(frag));
}

std::string EasyEvent::UrlParse::unquote(const std::string &s) {
    if (s.empty()) {
        return "";
    }
    std::vector<std::string_view> bits;
    bits = StrUtil::split(std::string_view(s), '%');
    if (bits.size() == 1) {
        return s;
    }

    std::vector<std::string_view> res;

    for (auto& item: bits) {
        if (res.empty()) {
            res.emplace_back(item);
            continue;
        }
        if (item.size() < 2) {
            res.emplace_back("%");
            res.emplace_back(item);
            continue;
        }

        auto trans = HexToChar.find(item.substr(0, 2));
        if (trans == HexToChar.end()) {
            res.emplace_back("%");
            res.emplace_back(item);
        } else {
            res.emplace_back(&trans->second, 1);
            res.emplace_back(item.substr(2));
        }
    }
    return StrUtil::join(res, "");
}

std::string EasyEvent::UrlParse::unquotePlus(const std::string &s) {
    return unquote(StrUtil::replaceAllCopy(s, "+", " "));
}

std::string EasyEvent::UrlParse::quote(const std::string &s, const std::string &safe) {
    std::set<char> alwaysSafe;
    for (auto c: AlwaysSafe) {
        alwaysSafe.insert(c);
    }
    for (auto c: safe) {
        if (c < (char)128) {
            alwaysSafe.insert(c);
        }
    }
    bool found = false;
    for (auto c: s) {
        if (alwaysSafe.find(c) == alwaysSafe.end()) {
            found = true;
            break;
        }
    }
    if (!found) {
        return s;
    }
    std::string builder;
    builder.reserve(s.size() * 2);
    for (auto c: s) {
        if (alwaysSafe.find(c) != alwaysSafe.end()) {
            builder.append(1, c);
        } else {
            const auto& safeChar = SafeMap.at(c);
            builder.append(safeChar.data(), safeChar.size());
        }
    }
    return builder;
}

std::string EasyEvent::UrlParse::quotePlus(const std::string &s, const std::string &safe) {
    if (s.find(' ') == std::string::npos) {
        return quote(s, safe);
    }
    std::string retValue = quote(s, safe +  ' ');
    return StrUtil::replaceAllCopy(retValue, " ", "+");
}

std::string EasyEvent::UrlParse::urlEncode(const OrderedDict<std::string, std::string> &query,
                                           const std::string &safe) {
    StringVec l;
    for (auto &kv: query) {
        l.emplace_back(quotePlus(kv.first, safe) + '=' + quotePlus(kv.second, safe));
    }
    return StrUtil::join(l, "&");
}

std::string EasyEvent::UrlParse::urlEncode(const std::vector<StringPair> &query, const std::string &safe) {
    StringVec l;
    for (auto &kv: query) {
        l.emplace_back(quotePlus(kv.first, safe) + '=' + quotePlus(kv.second, safe));
    }
    return StrUtil::join(l, "&");
}

std::string EasyEvent::UrlParse::urlEncode(const std::map<std::string, std::string> &query, const std::string &safe) {
    StringVec l;
    for (auto &kv: query) {
        l.emplace_back(quotePlus(kv.first, safe) + '=' + quotePlus(kv.second, safe));
    }
    return StrUtil::join(l, "&");
}

std::string EasyEvent::UrlParse::urlEncode(const std::unordered_map<std::string, std::string> &query,
                                           const std::string &safe) {
    StringVec l;
    for (auto &kv: query) {
        l.emplace_back(quotePlus(kv.first, safe) + '=' + quotePlus(kv.second, safe));
    }
    return StrUtil::join(l, "&");
}

std::string EasyEvent::UrlParse::urlEncode(const std::multimap<std::string, std::string> &query,
                                           const std::string &safe) {
    StringVec l;
    for (auto &kv: query) {
        l.emplace_back(quotePlus(kv.first, safe) + '=' + quotePlus(kv.second, safe));
    }
    return StrUtil::join(l, "&");
}

std::string EasyEvent::UrlParse::urlEncode(const std::unordered_multimap<std::string, std::string> &query,
                                           const std::string &safe) {
    StringVec l;
    for (auto &kv: query) {
        l.emplace_back(quotePlus(kv.first, safe) + '=' + quotePlus(kv.second, safe));
    }
    return StrUtil::join(l, "&");
}

std::string EasyEvent::UrlParse::urlEncode(const std::map<std::string, StringVec> &query, const std::string &safe) {
    StringVec l;
    std::string k;
    for (auto &kvs: query) {
        k = quotePlus(kvs.first, safe);
        for (auto &v: kvs.second) {
            l.emplace_back(k + '=' + quotePlus(v, safe));
        }
    }
    return StrUtil::join(l, "&");
}

std::string EasyEvent::UrlParse::urlEncode(const std::unordered_map<std::string, StringVec> &query,
                                           const std::string &safe) {
    StringVec l;
    std::string k;
    for (auto &kvs: query) {
        k = quotePlus(kvs.first, safe);
        for (auto &v: kvs.second) {
            l.emplace_back(k + '=' + quotePlus(v, safe));
        }
    }
    return StrUtil::join(l, "&");
}

std::map<std::string, StringVec> EasyEvent::UrlParse::parseQS(const std::string &queryString, bool keepBlankValues,
                                                              bool strictParsing, const std::string &separator) {
    std::map<std::string, StringVec> parsedResult;
    decltype(parsedResult.begin()) iter;
    auto pairs = parseQSL(queryString, keepBlankValues, strictParsing, separator);
    for (auto &nv: pairs) {
        iter = parsedResult.find(nv.first);
        if (iter != parsedResult.end()) {
            iter->second.emplace_back(std::move(nv.second));
        } else {
            parsedResult.emplace(std::move(nv.first), std::vector<std::string>{std::move(nv.second)});
        }
    }
    return parsedResult;
}

std::vector<StringPair> EasyEvent::UrlParse::parseQSL(const std::string &queryString, bool keepBlankValues,
                                                      bool strictParsing, const std::string &separator) {

    if (separator.empty()) {
        throwError(UrlErrors::InvalidSeparator, "parseQSL");
    }

    std::vector<StringPair> r;
    StringVec pairs;
    pairs = StrUtil::split(queryString, separator);

    size_t pos;
    std::string name, value;
    for (auto &nameValue: pairs) {
        if (nameValue.empty() && !strictParsing) {
            continue;
        }
        pos = nameValue.find('=');
        if (pos == std::string::npos) {
            if (strictParsing) {
                throwError(UrlErrors::BadQueryField, "parseQSL");
            }
            if (keepBlankValues) {
                name = nameValue;
                value = "";
            } else {
                continue;
            }
        } else {
            name = nameValue.substr(0, pos);
            value = nameValue.substr(pos + 1);
        }
        if (keepBlankValues || !value.empty()) {
            StrUtil::replaceAll(name, "+", " ");
            name = unquote(name);
            StrUtil::replaceAll(value, "+", " ");
            value = unquote(value);
            r.emplace_back(std::make_pair(std::move(name), std::move(value)));
        }
    }
    return r;
}

std::tuple<std::string_view, std::string_view> EasyEvent::UrlParse::splitParams(const std::string_view &url) {
    size_t i;
    i = url.rfind('/');
    if (i != std::string_view::npos) {
        i = url.find(';', i);
        if (i == std::string_view::npos) {
            return std::make_tuple(url, "");
        }
    } else {
        i = url.find(';');
    }
    return std::make_tuple(url.substr(0, i), url.substr(i + 1));
}

std::tuple<std::string_view, std::string_view> EasyEvent::UrlParse::splitNetloc(
        const std::string_view &url, size_t start) {
    size_t delim = url.size(), wdelim;
    char delimiters[3] = {'/', '?', '#'};
    for (char delimiter: delimiters) {
        wdelim = url.find(delimiter, start);
        if (wdelim != std::string_view::npos) {
            delim = std::min(delim, wdelim);
        }
    }
    return std::make_tuple(url.substr(start, delim - start), url.substr(delim));
}