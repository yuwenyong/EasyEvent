//
// Created by yuwenyong.vincent on 2021/9/5.
//

#include "EasyEvent/HttpUtil/HttpParse.h"
#include "EasyEvent/HttpUtil/HttpHeaders.h"
#include "EasyEvent/Common/StrUtil.h"

using namespace std::string_literals;

const char * EasyEvent::HttpInputErrorCategory::name() const noexcept {
    return "http input error";
}

std::string EasyEvent::HttpInputErrorCategory::message(int ev) const {
    switch (static_cast<HttpInputErrors>(ev)) {
        case HttpInputErrors::MalformedHttpRequestLine:
            return "malformed HTTP request line";
        case HttpInputErrors::MalformedHttpVersionInHttpRequestLine:
            return "malformed HTTP version in HTTP request line";
        case HttpInputErrors::EmptyHeaderLine:
            return "empty header line";
        case HttpInputErrors::NoColonInHeaderLine:
            return "no colon in header line";
        case HttpInputErrors::FirstHeaderLineStartWithWhitespace:
            return "first header line start with whitespace";
        case HttpInputErrors::ErrorParsingResponseStartLine:
            return "error parsing response start line";
        case HttpInputErrors::TransferEncodingAndContentLengthConflict:
            return "response with both Transfer-Encoding and Content-Length";
        case HttpInputErrors::MultipleUnequalContentLength:
            return "multiple unequal Content-Lengths";
        case HttpInputErrors::ContentLengthInvalidType:
            return "only integer Content-Length is allowed";
        case HttpInputErrors::ContentLengthTooLong:
            return "Content-Length too long";
        case HttpInputErrors::ChunkedBodyTooLarge:
            return "chunked body too large";
        case HttpInputErrors::UnexpectedChunkEnds:
            return "unexpected chunk ends";
        case HttpInputErrors::ImproperlyTerminateChunkedRequest:
            return "improperly terminated chunked request";
        default:
            return "unknown http input error";
    }
}

const std::error_category& EasyEvent::getHttpInputErrorCategory() {
    static const HttpInputErrorCategory errCategory{};
    return errCategory;
}

const char * EasyEvent::HttpOutputErrorCategory::name() const noexcept {
    return "http output error";
}

std::string EasyEvent::HttpOutputErrorCategory::message(int ev) const {
    switch (static_cast<HttpOutputErrors>(ev)) {
        case HttpOutputErrors::WriteMoreDataThanContentLength:
            return "tried to write more data than Content-Length";
        case HttpOutputErrors::WriteLessDataThanContentLength:
            return "tried to write less data than Content-Length";
        case HttpOutputErrors::NewlineInHeaders:
            return "newline in headers";
        default:
            return "unknown http output error";
    }
}

const std::error_category& EasyEvent::getHttpOutputErrorCategory() {
    static const HttpOutputErrorCategory errCategory{};
    return errCategory;
}

std::string EasyEvent::HttpParse::urlConcat(std::string url, const OrderedDict<std::string, std::string> &args) {
    if (args.empty()) {
        return url;
    }
    auto parsedUrl = UrlParse::urlParse(std::move(url));
    auto parsedQuery = UrlParse::parseQSL(parsedUrl.getQuery(), true);
    for (auto &kv: args) {
        parsedQuery.emplace_back(kv.first, kv.second);
    }
    auto finalQuery = UrlParse::urlEncode(parsedQuery);
    url = UrlParse::urlUnparse({
                                       parsedUrl.getScheme(),
                                       parsedUrl.getNetloc(),
                                       parsedUrl.getPath(),
                                       parsedUrl.getParams(),
                                       finalQuery,
                                       parsedUrl.getFragment()
                               });
    return url;
}

std::string EasyEvent::HttpParse::urlConcat(std::string url, const std::vector<StringPair> &args) {
    if (args.empty()) {
        return url;
    }
    auto parsedUrl = UrlParse::urlParse(std::move(url));
    auto parsedQuery = UrlParse::parseQSL(parsedUrl.getQuery(), true);
    parsedQuery.insert(parsedQuery.end(), args.begin(), args.end());
    auto finalQuery = UrlParse::urlEncode(parsedQuery);
    url = UrlParse::urlUnparse({
                                       parsedUrl.getScheme(),
                                       parsedUrl.getNetloc(),
                                       parsedUrl.getPath(),
                                       parsedUrl.getParams(),
                                       finalQuery,
                                       parsedUrl.getFragment()
                               });
    return url;
}

std::string EasyEvent::HttpParse::urlConcat(std::string url, const std::map<std::string, std::string> &args) {
    if (args.empty()) {
        return url;
    }
    auto parsedUrl = UrlParse::urlParse(std::move(url));
    auto parsedQuery = UrlParse::parseQSL(parsedUrl.getQuery(), true);
    for (auto &kv: args) {
        parsedQuery.emplace_back(kv.first, kv.second);
    }
    auto finalQuery = UrlParse::urlEncode(parsedQuery);
    url = UrlParse::urlUnparse({
                                       parsedUrl.getScheme(),
                                       parsedUrl.getNetloc(),
                                       parsedUrl.getPath(),
                                       parsedUrl.getParams(),
                                       finalQuery,
                                       parsedUrl.getFragment()
                               });
    return url;
}

std::string EasyEvent::HttpParse::urlConcat(std::string url, const std::unordered_map<std::string, std::string> &args) {
    if (args.empty()) {
        return url;
    }
    auto parsedUrl = UrlParse::urlParse(std::move(url));
    auto parsedQuery = UrlParse::parseQSL(parsedUrl.getQuery(), true);
    for (auto &kv: args) {
        parsedQuery.emplace_back(kv.first, kv.second);
    }
    auto finalQuery = UrlParse::urlEncode(parsedQuery);
    url = UrlParse::urlUnparse({
                                       parsedUrl.getScheme(),
                                       parsedUrl.getNetloc(),
                                       parsedUrl.getPath(),
                                       parsedUrl.getParams(),
                                       finalQuery,
                                       parsedUrl.getFragment()
                               });
    return url;
}

std::string EasyEvent::HttpParse::urlConcat(std::string url, const std::multimap<std::string, std::string> &args) {
    if (args.empty()) {
        return url;
    }
    auto parsedUrl = UrlParse::urlParse(std::move(url));
    auto parsedQuery = UrlParse::parseQSL(parsedUrl.getQuery(), true);
    for (auto &kv: args) {
        parsedQuery.emplace_back(kv.first, kv.second);
    }
    auto finalQuery = UrlParse::urlEncode(parsedQuery);
    url = UrlParse::urlUnparse({
                                       parsedUrl.getScheme(),
                                       parsedUrl.getNetloc(),
                                       parsedUrl.getPath(),
                                       parsedUrl.getParams(),
                                       finalQuery,
                                       parsedUrl.getFragment()
                               });
    return url;
}

std::string EasyEvent::HttpParse::urlConcat(std::string url, const std::unordered_multimap<std::string, std::string> &args) {
    if (args.empty()) {
        return url;
    }
    auto parsedUrl = UrlParse::urlParse(std::move(url));
    auto parsedQuery = UrlParse::parseQSL(parsedUrl.getQuery(), true);
    for (auto &kv: args) {
        parsedQuery.emplace_back(kv.first, kv.second);
    }
    auto finalQuery = UrlParse::urlEncode(parsedQuery);
    url = UrlParse::urlUnparse({
                                       parsedUrl.getScheme(),
                                       parsedUrl.getNetloc(),
                                       parsedUrl.getPath(),
                                       parsedUrl.getParams(),
                                       finalQuery,
                                       parsedUrl.getFragment()
                               });
    return url;
}

std::string EasyEvent::HttpParse::urlConcat(std::string url, const std::map<std::string, StringVec> &args) {
    if (args.empty()) {
        return url;
    }
    auto parsedUrl = UrlParse::urlParse(std::move(url));
    auto parsedQuery = UrlParse::parseQSL(parsedUrl.getQuery(), true);
    for (auto &kv: args) {
        for (auto& val: kv.second) {
            parsedQuery.emplace_back(kv.first, val);
        }
    }
    auto finalQuery = UrlParse::urlEncode(parsedQuery);
    url = UrlParse::urlUnparse({
                                       parsedUrl.getScheme(),
                                       parsedUrl.getNetloc(),
                                       parsedUrl.getPath(),
                                       parsedUrl.getParams(),
                                       finalQuery,
                                       parsedUrl.getFragment()
                               });
    return url;
}

std::string EasyEvent::HttpParse::urlConcat(std::string url, const std::unordered_map<std::string, StringVec> &args) {
    if (args.empty()) {
        return url;
    }
    auto parsedUrl = UrlParse::urlParse(std::move(url));
    auto parsedQuery = UrlParse::parseQSL(parsedUrl.getQuery(), true);
    for (auto &kv: args) {
        for (auto& val: kv.second) {
            parsedQuery.emplace_back(kv.first, val);
        }
    }
    auto finalQuery = UrlParse::urlEncode(parsedQuery);
    url = UrlParse::urlUnparse({
                                       parsedUrl.getScheme(),
                                       parsedUrl.getNetloc(),
                                       parsedUrl.getPath(),
                                       parsedUrl.getParams(),
                                       finalQuery,
                                       parsedUrl.getFragment()
                               });
    return url;
}

void EasyEvent::HttpParse::parseBodyArguments(const std::string_view &contentType, const std::string &body,
                                              HttpArguments &arguments, HttpFiles &files, StringVec &errors,
                                              const HttpHeaders *headers) {
    if (headers && headers->has(HttpHeaderFields::ContentEncoding)) {
        errors.emplace_back("Unsupported Content-Encoding: " + headers->at(HttpHeaderFields::ContentEncoding));
        return;
    }
    if (StrUtil::startsWith(contentType, "application/x-www-form-urlencoded")) {
        std::map<std::string, StringVec> uriArguments;
        try {
            uriArguments = UrlParse::parseQS(body, true);
        } catch (std::exception &e) {
            errors.emplace_back("Invalid x-www-form-urlencoded body: "s + e.what());
        }
        for (auto &nv: uriArguments) {
            if (!nv.second.empty()) {
                for (auto &value: nv.second) {
                    arguments[nv.first].push_back(std::move(value));
                }
            }
        }
    } else if (StrUtil::startsWith(contentType, "multipart/form-data")) {
        StringViewVec fields = StrUtil::split(contentType, ';');
        bool found = false;
        std::string_view k, sep, v;
        for (auto &field: fields) {
            StrUtil::trim(field);
            std::tie(k, sep, v) = StrUtil::partition(field, "=");
            if (k == "boundary" && !v.empty()) {
                HttpParse::parseMultipartFormData(std::string(v.data(), v.size()), body, arguments, files, errors);
                found = true;
                break;
            }
        }
        if (!found) {
            errors.emplace_back("Invalid multipart/form-data");
        }
    }
}

void EasyEvent::HttpParse::parseMultipartFormData(std::string boundary, const std::string &data,
                                                  HttpArguments &arguments, HttpFiles &files, StringVec &errors) {
    if (StrUtil::startsWith(boundary, "\"") && StrUtil::endsWith(boundary, "\"")) {
        if (boundary.length() > 2) {
            boundary = boundary.substr(1, boundary.length() - 2);
        } else {
            boundary.clear();
        }
    }

    size_t finalBoundaryIndex = data.rfind("--" + boundary + "--");
    if (finalBoundaryIndex == std::string::npos) {
        errors.emplace_back("Invalid multipart/form-data: no final boundary");
        return;
    }
    StringViewVec parts = StrUtil::split(std::string_view(data.data(), finalBoundaryIndex), "--" + boundary + "\r\n");
    size_t eoh;
    HttpHeaders headers;
    std::string dispHeader, disposition, name, value, ctype;
    StringMap dispParams;
    for (auto &part: parts) {
        if (part.empty()) {
            continue;
        }
        eoh = part.find("\r\n\r\n");
        if (eoh == std::string_view::npos) {
            errors.emplace_back("multipart/form-data missing headers");
            continue;
        }
        headers.clear();
        headers.parseLines(part.substr(0, eoh));
        dispHeader = headers.get("Content-Disposition");
        std::tie(disposition, dispParams) = parseHeader(dispHeader);
        if (disposition != "form-data" || !StrUtil::endsWith(part, "\r\n")) {
            errors.emplace_back("Invalid multipart/form-data");
            continue;
        }
        if (part.length() <= eoh + 6) {
            value.clear();
        } else {
            value = part.substr(eoh + 4, part.length() - eoh - 6);
        }
        auto nameIter = dispParams.find("name");
        if (nameIter == dispParams.end()) {
            errors.emplace_back( "multipart/form-data value missing name");
            continue;
        }
        name = nameIter->second;
        auto fileNameIter = dispParams.find("filename");
        if (fileNameIter != dispParams.end()) {
            ctype = headers.get("Content-Type", "application/unknown");
            files[name].emplace_back(std::move(fileNameIter->second), std::move(ctype), std::move(value));
        } else {
            arguments[name].emplace_back(std::move(value));
        }
    }
}

std::string EasyEvent::HttpParse::formatTimestamp(Time ts) {
    struct tm dt = ts.utcTime();
    const char *weekdayNames[] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    const char *monthNames[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s, %02d %s %04d %02d:%02d:%02d GMT", weekdayNames[dt.tm_wday],
             dt.tm_mday, monthNames[dt.tm_mon], dt.tm_year + 1900, dt.tm_hour, dt.tm_min, dt.tm_sec);
    return buffer;
}

std::tuple<std::string, std::optional<unsigned short>> EasyEvent::HttpParse::splitHostAndPort(const std::string &netloc) {
    const std::regex pat(R"(^(.+):(\d+)$)");
    std::smatch match;
    std::string host;
    std::optional<unsigned short> port;
    if (std::regex_match(netloc, match, pat)) {
        host = match[1];
        port = (unsigned short) std::stoul(match[2]);
    } else {
        host = netloc;
    }
    return std::make_tuple(std::move(host), port);
}

StringVec EasyEvent::HttpParse::parseParam(std::string_view s) {
    StringVec parts;
    size_t end;
    std::string_view temp;
    while (!s.empty() && s[0] == ';') {
        s = s.substr(1);
        end = s.find(';');
        while (end != std::string_view::npos
               && ((StrUtil::count(s, '"', 0, end) - StrUtil::count(s, "\\\"", 0, end)) % 2 != 0)) {
            end = s.find('"', end + 1);
        }
        if (end == std::string_view::npos) {
            end = s.size();
        }
        temp = StrUtil::trimCopy(s.substr(0, end));
        parts.emplace_back(temp.data(), temp.size());
        s = s.substr(end);
    }
    return parts;
}

std::tuple<std::string, StringMap> EasyEvent::HttpParse::parseHeader(const std::string &line) {
    StringVec parts = parseParam(";" + line);
    std::string key = std::move(parts[0]);
    parts.erase(parts.begin());
    StringMap pdict;
    size_t i;
    std::string name, value;
    for (auto &p: parts) {
        i = p.find('=');
        if (i != std::string::npos) {
            name = StrUtil::toLowerCopy(StrUtil::trimCopy(p.substr(0, i)));
            value = StrUtil::trimCopy(p.substr(i + 1));
            if (value.size() >= 2 && value.front() == '\"' && value.back() == '\"') {
                value = value.substr(1, value.size() - 2);
                StrUtil::replaceAll(value, "\\\\", "\\");
                StrUtil::replaceAll(value, "\\\"", "\"");
            }
            pdict[std::move(name)] = std::move(value);
        } else {
            pdict[p] = "";
        }
    }
    return std::make_tuple(std::move(key), std::move(pdict));
}

std::string EasyEvent::HttpParse::encodeHeader(const std::string &key, const StringMap &pdict) {
    if (pdict.empty()) {
        return key;
    }
    StringVec out{key,};
    for (auto &kv: pdict) {
        if (kv.second.empty()) {
            out.emplace_back(kv.first);
        } else {
            out.emplace_back(kv.first + "=" + kv.second);
        }
    }
    return StrUtil::join(out, "; ");
}

EasyEvent::RequestStartLine EasyEvent::RequestStartLine::parse(const std::string_view& line) {
    auto fields = StrUtil::split(line, ' ');
    if (fields.size() < 3) {
        throwError(HttpInputErrors::MalformedHttpRequestLine, "RequestStartLine");
    }
    std::string method(fields[0].data(), fields[0].size());
    std::string path(fields[1].data(), fields[1].size());
    std::string version(fields[2].data(), fields[2].size());
    const std::regex versionPat(R"(HTTP/1\.[0-9])");
    if (!std::regex_match(version, versionPat)) {
        throwError(HttpInputErrors::MalformedHttpVersionInHttpRequestLine);
    }
    return RequestStartLine(std::move(method), std::move(path), std::move(version));
}

EasyEvent::ResponseStartLine EasyEvent::ResponseStartLine::parse(const std::string& line) {
    const std::regex firstLinePattern("(HTTP/1.[0-9]) ([0-9]+) ([^\r]*).*");
    std::smatch match;
    if (!std::regex_match(line, match, firstLinePattern)) {
        throwError(HttpInputErrors::ErrorParsingResponseStartLine, "ResponseStartLine");
    }
    return ResponseStartLine(match[1], std::stoi(match[2]), match[3]);
}