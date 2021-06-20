//
// Created by yuwenyong on 2021/1/3.
//

#include "EasyEvent/Configuration/Options.h"


void EasyEvent::Options::addOpt(const std::string &shortOpt, const std::string &longOpt, bool needArg,
                                const std::string &dflt, bool repeat) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_parsed) {
        throwError(UserErrors::BadState, "Options", "cannot add options after parse() was called");
    }
    checkArgs(shortOpt, longOpt, needArg, dflt);
    addValidOpt(shortOpt, longOpt, needArg, dflt, repeat);
}

std::vector<std::string> EasyEvent::Options::parse(const std::vector<std::string> &args) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_parsed) {
        throwError(UserErrors::BadState, "Options", "cannot call parse() more than once on the same Option instance");
    }

    std::set<std::string> seenNonRepeatOpts;
    std::vector<std::string> result;
    std::string::size_type i;

    for(i = 1; i < args.size(); ++i) {
        if(args[i] == "-" || args[i] == "--") {
            ++i;
            break;
        }

        std::string opt;
        OptMapping::iterator pos;
        bool argDone = false;

        if(args[i].compare(0, 2, "--") == 0) {
            std::string::size_type p = args[i].find('=', 2);
            if(p != std::string::npos) {
                opt = args[i].substr(2, p - 2);
            } else {
                opt = args[i].substr(2);
            }

            pos = checkOpt(opt, OptType::LongOpt);

            if(!pos->second->repeat) {
                auto seenPos = seenNonRepeatOpts.find(opt);
                if(seenPos != seenNonRepeatOpts.end()) {
                    std::string err = "`--";
                    err += opt + ":' option cannot be repeated";
                    throwError(UserErrors::BadValue, "Options", err);
                }
                seenNonRepeatOpts.insert(seenPos, opt);
                std::string synonym = getSynonym(opt);
                if(!synonym.empty()) {
                    seenNonRepeatOpts.insert(synonym);
                }
            }

            if(p != std::string::npos) {
                if(!pos->second->needArg) {
                    std::string err = "`";
                    err += opt;
                    err += "': option does not take an argument";
                    throwError(UserErrors::BadValue, "Options", err);
                } else if(p == args[i].size() - 1) {
                    std::string err = "`";
                    err += opt;
                    err += "': option requires an argument";
                    throwError(UserErrors::BadValue, "Options", err);
                }
                setOpt(opt, "", args[i].substr(p + 1), pos->second->repeat);
                argDone = true;
            }
        } else if(!args[i].empty() && args[i][0] == '-') {
            for(std::string::size_type p = 1; p < args[i].size(); ++p) {
                opt.clear();
                opt.push_back(args[i][p]);
                pos = checkOpt(opt, OptType::ShortOpt);
                if(!pos->second->repeat) {
                    auto seenPos = seenNonRepeatOpts.find(opt);
                    if(seenPos != seenNonRepeatOpts.end()) {
                        std::string err = "`--";
                        err += opt + ":' option cannot be repeated";
                        throwError(UserErrors::BadValue, "Options", err);
                    }
                    seenNonRepeatOpts.insert(seenPos, opt);
                    std::string synonym = getSynonym(opt);
                    if(!synonym.empty()) {
                        seenNonRepeatOpts.insert(synonym);
                    }
                }

                if(pos->second->needArg) {
                    if(p != args[i].size() - 1) {
                        std::string optArg = args[i].substr(p + 1);
                        setOpt(opt, "", optArg, pos->second->repeat);
                        argDone = true;
                        break;
                    }
                } else {
                    setOpt(opt, "", "1", pos->second->repeat);
                    argDone = true;
                }
            }
        } else {
            result.push_back(args[i]);
            argDone = true;
        }

        if(!argDone) {
            if(pos->second->needArg) {
                if(i == args.size() - 1) {
                    std::string err = "`-";
                    if(opt.size() != 1) {
                        err += "-";
                    }
                    err += opt;
                    err += "' option requires an argument";
                    throwError(UserErrors::BadValue, "Options", err);
                }
                setOpt(opt, "", args[++i], pos->second->repeat);
            } else {
                setOpt(opt, "", "1", pos->second->repeat);
            }
        }
    }

    _synonyms.clear();

    while(i < args.size()) {
        result.push_back(args[i++]);
    }
    return result;
}

std::vector<std::string> EasyEvent::Options::parse(int argc, const char *const *argv) {
    std::vector<std::string> vec;
    for(int i = 0; i < argc; ++i) {
        vec.emplace_back(argv[i]);
    }
    return parse(vec);
}

bool EasyEvent::Options::isSet(const std::string &opt) const {
    std::lock_guard<std::mutex> lock(_mutex);
    if (!_parsed) {
        throwError(UserErrors::BadState, "Options", "cannot lookup options before calling parse()");
    }

    auto pos = checkOptIsValid(opt);
    return pos->second->repeat ? _optVal.find(opt)!= _optVal.end() : _optVals.find(opt) != _optVals.end();
}

std::string EasyEvent::Options::optVal(const std::string &opt) const {
    std::lock_guard<std::mutex> lock(_mutex);
    if (!_parsed) {
        throwError(UserErrors::BadState, "Options", "cannot lookup options before calling parse()");
    }
    auto pos = checkOptHasArg(opt);
    if(pos->second->repeat) {
        std::string err = "`-";
        if(pos->second->type == OptType::LongOpt)
        {
            err.push_back('-');
        }
        err += opt;
        err += "': is a repeating option -- use optVals() to get its arguments";
        throwError(UserErrors::OperationForbidden, "Options", err);
    }

    auto p = _optVal.find(opt);
    if(p == _optVal.end()) {
        return "";
    }
    return p->second->val;
}

std::vector<std::string> EasyEvent::Options::optVals(const std::string &opt) const {
    std::lock_guard<std::mutex> lock(_mutex);
    if (!_parsed) {
        throwError(UserErrors::BadState, "Options", "cannot lookup options before calling parse()");
    }

    auto pos = checkOptHasArg(opt);

    if(!pos->second->repeat) {
        std::string err = "`-";
        if(pos->second->type == OptType::LongOpt)
        {
            err.push_back('-');
        }
        err += opt + "': is a non-repeating option -- use optArg() to get its argument";
        throwError(UserErrors::OperationForbidden, "Options", err);
    }

    auto p = _optVals.find(opt);
    return p == _optVals.end() ? std::vector<std::string>{} : p->second->vals;
}

std::vector<std::string> EasyEvent::Options::split(const std::string &line) {
    const std::string IFS = " \t\n";
    std::string::size_type start = line.find_first_not_of(IFS);
    if(start == std::string::npos) {
        return std::vector<std::string>{};
    }
    std::string::size_type end = line.find_last_not_of(IFS);
    Assert(end != std::string::npos);

    std::string l(line, start, end - start + 1);
    std::vector<std::string> vec;

    enum ParseState { Normal, DoubleQuote, SingleQuote, ANSIQuote };
    ParseState state = Normal;

    std::string arg;

    for(std::string::size_type i = 0; i < l.size(); ++i) {
        char c = l[i];
        switch(state) {
            case Normal: {
                switch(c) {
                    case '\\': {
                        if(i < l.size() - 1 && l[++i] != '\n') {
                            switch(l[i]) {
                                case ' ':
                                case '$':
                                case '\'':
                                case '"': {
                                    arg.push_back(l[i]);
                                    break;
                                }
                                default: {
                                    arg.push_back('\\');
                                    arg.push_back(l[i]);
                                    break;
                                }
                            }
                        }
                        break;
                    }
                    case '\'': {
                        state = SingleQuote;
                        break;
                    }
                    case '"': {
                        state = DoubleQuote;
                        break;
                    }
                    case '$': {
                        if(i < l.size() - 1 && l[i + 1] == '\'') {
                            state = ANSIQuote; // Bash uses $'<text>' to allow ANSI escape sequences within <text>.
                            ++i;
                        } else {
                            arg.push_back('$');
                        }
                        break;
                    }
                    default: {
                        if(IFS.find(l[i]) != std::string::npos) {
                            vec.push_back(arg);
                            arg.clear();

                            while(++i < l.size() && IFS.find(l[i]) != std::string::npos) {}
                            --i;
                        } else {
                            arg.push_back(l[i]);
                        }
                        break;
                    }
                }
                break;
            }
            case DoubleQuote: {
                if(c == '\\' && i < l.size() - 1) {
                    switch(c = l[++i]) {
                        case '"':
                        case '\\':
                        case '\n': {
                            arg.push_back(c);
                            break;
                        }
                        default: {
                            arg.push_back('\\');
                            arg.push_back(c);
                            break;
                        }
                    }
                } else if(c == '"') {
                    state = Normal;
                } else {
                    arg.push_back(c);
                }
                break;
            }
            case SingleQuote: {
                if(c == '\'') {
                    state = Normal;
                } else {
                    arg.push_back(c);
                }
                break;
            }
            case ANSIQuote: {
                switch(c) {
                    case '\\': {
                        if(i == l.size() - 1) {
                            break;
                        }
                        switch(c = l[++i]) {
                            case 'a': {
                                arg.push_back('\a');
                                break;
                            }
                            case 'b': {
                                arg.push_back('\b');
                                break;
                            }
                            case 'f': {
                                arg.push_back('\f');
                                break;
                            }
                            case 'n': {
                                arg.push_back('\n');
                                break;
                            }
                            case 'r': {
                                arg.push_back('\r');
                                break;
                            }
                            case 't': {
                                arg.push_back('\t');
                                break;
                            }
                            case 'v': {
                                arg.push_back('\v');
                                break;
                            }
                            case '\\': {
                                arg.push_back('\\');
                                break;
                            }
                            case '\'': {
                                arg.push_back('\'');
                                break;
                            }
                            case 'e': { // Not ANSI-C, but used by bash.
                                arg.push_back('\033');
                                break;
                            }
                            case '0':
                            case '1':
                            case '2':
                            case '3':
                            case '4':
                            case '5':
                            case '6':
                            case '7': {
                                static const std::string octalDigits = "01234567";
                                unsigned short us = 0;
                                std::string::size_type j;
                                for(j = i; j < i + 3 && j < l.size() &&
                                           octalDigits.find_first_of(c = l[j]) != std::string::npos; ++j) {
                                    us = static_cast<unsigned short>(us * 8u + static_cast<unsigned short>(c - '0'));
                                }
                                i = j - 1;
                                arg.push_back(static_cast<char>(us));
                                break;
                            }
                            case 'x': {
                                if(i < l.size() - 1 && !isxdigit(static_cast<unsigned char>(l[i + 1]))) {
                                    arg.push_back('\\');
                                    arg.push_back('x');
                                    break;
                                }

                                Int64 ull = 0;
                                std::string::size_type j;
                                for(j = i + 1; j < i + 3 && j < l.size() &&
                                               isxdigit(static_cast<unsigned char>(c = l[j])); ++j) {
                                    ull *= 16;
                                    if(isdigit(static_cast<unsigned char>(c))) {
                                        ull += c - '0';
                                    } else if(islower(static_cast<unsigned char>(c))) {
                                        ull += c - 'a' + 10;
                                    } else {
                                        ull += c - 'A' + 10;
                                    }
                                }
                                i = j - 1;
                                arg.push_back(static_cast<char>(ull));
                                break;
                            }
                            case 'c': {
                                c = l[++i];
                                if(std::isalpha(c) || c == '@' || (c >= '[' && c <= '_')) {
                                    arg.push_back(static_cast<char>(toupper(static_cast<unsigned char>(c)) - '@'));
                                } else {
                                    arg.push_back('\\');
                                    arg.push_back('c');
                                    arg.push_back(c);
                                }
                                break;
                            }
                            default: {
                                arg.push_back('\\');
                                arg.push_back(c);
                                break;
                            }
                        }
                        break;
                    }
                    case '\'': {
                        state = Normal;
                        break;
                    }
                    default: {
                        arg.push_back(c);
                        break;
                    }
                }
                break;
            }
            default: {
                Assert(false);
                break;
            }
        }
    }

    switch(state) {
        case Normal: {
            vec.push_back(arg);
            break;
        }
        case SingleQuote: {
            throwError(UserErrors::ParsingFailed, "Options", "missing closing single quote");
            break;
        }
        case DoubleQuote: {
            throwError(UserErrors::ParsingFailed, "Options", "missing closing double quote");
            break;
        }
        case ANSIQuote: {
            throwError(UserErrors::ParsingFailed, "Options", "unterminated $' quote");
            break;
        }
        default: {
            Assert(false);
            break;
        }
    }

    return vec;
}

void EasyEvent::Options::addValidOpt(const std::string &shortOpt, const std::string &longOpt, bool needArg,
                                     const std::string &dflt, bool repeat) {
    if(!shortOpt.empty() && _opts.find(shortOpt) != _opts.end()) {
        std::string err = "`";
        err += shortOpt;
        err += "': duplicate option";
        throwError(UserErrors::DuplicateValues, "Options", err);
    }
    if(!longOpt.empty() && _opts.find(longOpt) != _opts.end()) {
        std::string err = "`";
        err += longOpt;
        err += "': duplicate option";
        throwError(UserErrors::DuplicateValues, "Options", err);
    }

    auto opt = std::make_shared<Opt>();
    opt->needArg = needArg;
    opt->repeat = repeat;
    opt->hasDefault = !dflt.empty();

    if(!shortOpt.empty()) {
        opt->type = OptType::ShortOpt;
        _opts[shortOpt] = opt;
    }
    if(!longOpt.empty())
    {
        opt->type = OptType::LongOpt;
        _opts[longOpt] = opt;
    }

    updateSynonyms(shortOpt, longOpt);

    if(needArg && !dflt.empty()) {
        setOpt(shortOpt, longOpt, dflt, repeat);
    }
}

EasyEvent::Options::OptMapping::iterator EasyEvent::Options::checkOpt(const std::string &opt, OptType optType) {
    auto pos = _opts.find(opt);
    if(pos == _opts.end()) {
        std::string err = "invalid option: `-";
        if(optType == OptType::LongOpt) {
            err.push_back('-');
        }
        err += opt;
        err.push_back('\'');
        throwError(UserErrors::NotFound, "Options", err);
    }
    return pos;
}

void EasyEvent::Options::setOpt(const std::string &opt1, const std::string &opt2, const std::string &val, bool repeat) {
    Assert(!(opt1.empty() && opt2.empty()));

    if(!repeat) {
        setNonRepeatingOpt(opt1, val);
        setNonRepeatingOpt(opt2, val);
    } else {
        setRepeatingOpt(opt1, val);
        setRepeatingOpt(opt2, val);
    }
}

void EasyEvent::Options::setNonRepeatingOpt(const std::string &opt, const std::string &val) {
    if(opt.empty()) {
        return;
    }
    Assert(_optVal.find(opt) == _optVal.end() || _opts.find(opt)->second->hasDefault);

    auto opv = std::make_shared<OptVal>();
    opv->val = val;
    _optVal[opt] = opv;

    const std::string synonym = getSynonym(opt);
    if(!synonym.empty()) {
        _optVal[synonym] = opv;
    }
}

void EasyEvent::Options::setRepeatingOpt(const std::string &opt, const std::string &val) {
    if(opt.empty()) {
        return;
    }

    auto vpos = _opts.find(opt);
    Assert(vpos != _opts.end());

    auto pos = _optVals.find(opt);
    const std::string synonym = getSynonym(opt);
    auto spos = _optVals.find(synonym);

    if(pos != _optVals.end()) {
        Assert(vpos->second->repeat);
        _optVals[opt] = pos->second;
        if(vpos->second->hasDefault && pos->second->vals.size() == 1) {
            pos->second->vals[0] = val;
            vpos->second->hasDefault = false;
        } else {
            pos->second->vals.push_back(val);
        }
    } else if(spos != _optVals.end()) {
        Assert(_opts.find(synonym) != _opts.end());
        Assert(_opts.find(synonym)->second->repeat);

        _optVals[synonym] = spos->second;
        if(vpos->second->hasDefault && spos->second->vals.size() == 1) {
            spos->second->vals[0] = val;
            vpos->second->hasDefault = false;
        } else {
            spos->second->vals.push_back(val);
        }
    } else {
        auto opv = std::make_shared<OptVals>();
        opv->vals.push_back(val);
        _optVals[opt] = opv;
        if(!synonym.empty()) {
            _optVals[synonym] = opv;
        }
    }
}

EasyEvent::Options::OptMapping::const_iterator EasyEvent::Options::checkOptIsValid(const std::string &opt) const {
    auto pos = _opts.find(opt);
    if(pos == _opts.end()) {
        std::string err = "`";
        err += opt;
        err += "': invalid option";
        throwError(UserErrors::NotFound, "Options", err);
    }
    return pos;
}

EasyEvent::Options::OptMapping::const_iterator EasyEvent::Options::checkOptHasArg(const std::string &opt) const {
    auto pos = checkOptIsValid(opt);
    if(!pos->second->needArg) {
        std::string err = "`-";
        if(pos->second->type == OptType::LongOpt) {
            err.push_back('-');
        }
        err += opt;
        err += "': option does not take arguments";
        throwError(UserErrors::InvalidArgument, "Options", err);
    }
    return pos;
}

void EasyEvent::Options::updateSynonyms(const std::string &shortOpt, const std::string &longOpt) {
    if(!shortOpt.empty() && !longOpt.empty()) {
        _synonyms[shortOpt] = longOpt;
        _synonyms[longOpt] = shortOpt;
    }
}

std::string EasyEvent::Options::getSynonym(const std::string &optName) const {
    auto pos = _synonyms.find(optName);
    return pos != _synonyms.end() ? pos->second : std::string{};
}

void EasyEvent::Options::checkArgs(const std::string &shortOpt, const std::string &longOpt, bool needArg,
                                   const std::string &dflt) {
    if(shortOpt.empty() && longOpt.empty()) {
        throwError(UserErrors::InvalidArgument, "Options", "short and long option cannot both be empty");
    }

    if(!shortOpt.empty()) {
        if(shortOpt.size() != 1) {
            std::string err = "`";
            err += shortOpt;
            err += "': a short option cannot specify more than one option";
            throwError(UserErrors::InvalidArgument, "Options", err);
        }
        if(shortOpt.find_first_of(" \t\n\r\f\v") != std::string::npos) {
            std::string err = "`";
            err += shortOpt;
            err += "': a short option cannot be whitespace";
            throwError(UserErrors::InvalidArgument, "Options", err);
        }
        if(shortOpt[0] == '-') {
            std::string err = "`";
            err += shortOpt;
            err += "': a short option cannot be `-'";
            throwError(UserErrors::InvalidArgument, "Options", err);
        }
    }

    if(!longOpt.empty()) {
        if(longOpt.find_first_of(" \t\n\r\f\v") != std::string::npos) {
            std::string err = "`";
            err += longOpt;
            err += "': a long option cannot contain whitespace";
            throwError(UserErrors::InvalidArgument, "Options", err);
        }
        if(longOpt[0] == '-') {
            std::string err = "`";
            err += longOpt;
            err += "': a long option must not contain a leading `-'";
            throwError(UserErrors::InvalidArgument, "Options", err);
        }
    }

    if(!needArg && !dflt.empty()) {
        throwError(UserErrors::InvalidArgument, "Options",
                   "a default value can be specified only for options requiring an argument");
    }
}
