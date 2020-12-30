//
// Created by yuwenyong on 2020/12/28.
//

#include "EasyEvent/Configuration/Json.h"


static inline char getDecimalPoint() {
    struct lconv* lc = localeconv();
    return lc ? *(lc->decimal_point) : '\0';
}

//static inline void fixNumericLocale(char* begin, char* end) {
//    while (begin < end) {
//        if (*begin == ',') {
//            *begin = '.';
//        }
//        ++begin;
//    }
//}

static void fixNumericLocaleInput(char* begin, char* end) {
    char decimalPoint = getDecimalPoint();
    if (decimalPoint != '\0' && decimalPoint != '.') {
        while (begin < end) {
            if (*begin == '.') {
                *begin = decimalPoint;
            }
            ++begin;
        }
    }
}

static const double maxUInt64AsDouble = 18446744073709551615.0;


template <typename T, typename U>
static inline bool InRange(double d, T min, U max) {
    return d >= (double)min && d <= (double)max;
}

static inline bool IsIntegral(double d) {
    double integralPart;
    return modf(d, &integralPart) == 0.0;
}

static std::string valueToString(double value, bool useSpecialFloats, unsigned int precision) {
    if (std::isfinite(value)) {
        std::ostringstream os;
        os << std::setprecision((int)precision) << value;
        return os.str();
    } else {
        if (value != value) {
            return useSpecialFloats ? "NaN" : "null";
        } else if (value < 0.0) {
            return useSpecialFloats ? "-Infinity" : "-1e+9999";
        } else {
            return useSpecialFloats ? "Infinity" : "1e+9999";
        }
    }
}

static std::string valueToString(double value) {
    return valueToString(value, false, 17);
}


static bool isAnyCharRequiredQuoting(const std::string &s) {
    for (auto c: s) {
        if (c == '\\' || c == '\"' || c < ' ' || static_cast<unsigned char>(c) < 0x80) {
            return true;
        }
    }
    return false;
}

static unsigned int utf8ToCodePoint(const char *&s, const char *e) {
    const unsigned int REPLACEMENT_CHARACTER = 0xFFFD;
    unsigned int firstByte = static_cast<unsigned char>(*s);
    if (firstByte < 0x80) {
        return firstByte;
    }
    if (firstByte < 0xE0) {
        if (e - s < 2) {
            return REPLACEMENT_CHARACTER;
        }
        unsigned int calculated = ((firstByte & 0x1F) << 6) | (static_cast<unsigned int>(s[1]) & 0x3F);
        s += 1;
        return calculated < 0x80 ? REPLACEMENT_CHARACTER : calculated;
    }

    if (firstByte < 0xF0) {
        if (e - s < 3) {
            return REPLACEMENT_CHARACTER;
        }
        unsigned int calculated = ((firstByte & 0x0F) << 12)
                                  | ((static_cast<unsigned int>(s[1]) & 0x3F) << 6)
                                  |  (static_cast<unsigned int>(s[2]) & 0x3F);
        s += 2;
        if (calculated >= 0xD800 && calculated <= 0xDFFF) {
            return REPLACEMENT_CHARACTER;
        }
        return calculated < 0x800 ? REPLACEMENT_CHARACTER : calculated;
    }

    if (firstByte < 0xF8) {
        if (e - s < 4) {
            return REPLACEMENT_CHARACTER;
        }
        unsigned int calculated = ((firstByte & 0x07) << 24)
                                  | ((static_cast<unsigned int>(s[1]) & 0x3F) << 12)
                                  | ((static_cast<unsigned int>(s[2]) & 0x3F) << 6)
                                  |  (static_cast<unsigned int>(s[3]) & 0x3F);
        s += 3;
        return calculated < 0x10000 ? REPLACEMENT_CHARACTER : calculated;
    }
    return REPLACEMENT_CHARACTER;
}


static std::string codePointToUTF8(unsigned int cp) {
    std::string result;
    // based on description from http://en.wikipedia.org/wiki/UTF-8
    if (cp <= 0x7f) {
        result.resize(1);
        result[0] = static_cast<char>(cp);
    } else if (cp <= 0x7FF) {
        result.resize(2);
        result[1] = static_cast<char>(0x80 | (0x3f & cp));
        result[0] = static_cast<char>(0xC0 | (0x1f & (cp >> 6)));
    } else if (cp <= 0xFFFF) {
        result.resize(3);
        result[2] = static_cast<char>(0x80 | (0x3f & cp));
        result[1] = static_cast<char>(0x80 | (0x3f & (cp >> 6)));
        result[0] = static_cast<char>(0xE0 | (0xf & (cp >> 12)));
    } else if (cp <= 0x10FFFF) {
        result.resize(4);
        result[3] = static_cast<char>(0x80 | (0x3f & cp));
        result[2] = static_cast<char>(0x80 | (0x3f & (cp >> 6)));
        result[1] = static_cast<char>(0x80 | (0x3f & (cp >> 12)));
        result[0] = static_cast<char>(0xF0 | (0x7 & (cp >> 18)));
    }
    return result;
}


static const char hex2[] =
    "000102030405060708090a0b0c0d0e0f"
    "101112131415161718191a1b1c1d1e1f"
    "202122232425262728292a2b2c2d2e2f"
    "303132333435363738393a3b3c3d3e3f"
    "404142434445464748494a4b4c4d4e4f"
    "505152535455565758595a5b5c5d5e5f"
    "606162636465666768696a6b6c6d6e6f"
    "707172737475767778797a7b7c7d7e7f"
    "808182838485868788898a8b8c8d8e8f"
    "909192939495969798999a9b9c9d9e9f"
    "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
    "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
    "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
    "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
    "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
    "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";

static std::string toHex16Bit(unsigned int x) {
    const unsigned int hi = (x >> 8) & 0xff;
    const unsigned int lo = x & 0xff;
    std::string result(4, ' ');
    result[0] = hex2[2 * hi];
    result[1] = hex2[2 * hi + 1];
    result[2] = hex2[2 * lo];
    result[3] = hex2[2 * lo + 1];
    return result;
}

static std::string valueToQuotedString(const std::string &value) {
    if (!isAnyCharRequiredQuoting(value)) {
        return "\"" + value + "\"";
    }
    std::string::size_type maxsize = value.size() * 2 + 3;
    std::string result;
    result.reserve(maxsize);
    result += "\"";
    const char *end = value.c_str() + value.size();
    for (const char *c = value.c_str(); c != end; ++c) {
        switch (*c) {
            case '\"': {
                result += "\\\"";
                break;
            }
            case '\\': {
                result += "\\\\";
                break;
            }
            case '\b': {
                result += "\\b";
                break;
            }
            case '\f': {
                result += "\\f";
                break;
            }
            case '\n': {
                result += "\\n";
                break;
            }
            case '\r': {
                result += "\\r";
                break;
            }
            case '\t': {
                result += "\\t";
                break;
            }
            default: {
                unsigned int cp = utf8ToCodePoint(c, end);
                if (cp < 0x80 && cp >= 0x20) {
                    result += static_cast<char>(cp);
                } else if (cp < 0x10000) {
                    result += "\\u";
                    result += toHex16Bit(cp);
                } else {
                    cp -= 0x10000;
                    result += "\\u";
                    result += toHex16Bit((cp >> 10) + 0xD800);
                    result += "\\u";
                    result += toHex16Bit((cp & 0x3FF) + 0xDC00);
                }
                break;
            }
        }
    }
    result += "\"";
    return result;
}


EasyEvent::JsonValue::JsonValue(JsonType type) {
    switch (type) {
        case JsonType::NullValue: {
            break;
        }
        case JsonType::IntValue: {
            _value = (int64_t)0;
            break;
        }
        case JsonType::UintValue: {
            _value = (uint64_t)0;
            break;
        }
        case JsonType::RealValue: {
            _value = 0.0;
            break;
        }
        case JsonType::StringValue: {
            _value = "";
            break;
        }
        case JsonType::BoolValue: {
            _value = false;
            break;
        }
        case JsonType::ArrayValue: {
            _value = ArrayType{};
            break;
        }
        case JsonType::ObjectValue: {
            _value = ObjectType{};
            break;
        }
    }
}

EasyEvent::JsonType EasyEvent::JsonValue::type() const {
    return std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, NullValue>) {
            return JsonType::NullValue;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return JsonType::IntValue;
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return JsonType::UintValue;
        } else if constexpr (std::is_same_v<T, double>) {
            return JsonType::RealValue;
        } else if constexpr (std::is_same_v<T, std::string>) {
            return JsonType::StringValue;
        } else if constexpr (std::is_same_v<T, bool>) {
            return JsonType::BoolValue;
        } else if constexpr (std::is_same_v<T, ArrayType>) {
            return JsonType::ArrayValue;
        } else if constexpr (std::is_same_v<T, ObjectType>) {
            return JsonType::ObjectValue;
        } else {
            static_assert(FailType<T>{});
        }
    }, _value);
}

int EasyEvent::JsonValue::compare(const JsonValue &other) const {
    if (*this < other) {
        return -1;
    }
    if (*this > other) {
        return 1;
    }
    return 0;
}

std::string EasyEvent::JsonValue::asString(std::error_code& ec) const {
    return std::visit([&ec](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, NullValue>) {
            ec.assign(0, ec.category());
            return std::string{};
        } else if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t>) {
            ec.assign(0, ec.category());
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, double>) {
            ec.assign(0, ec.category());
            return valueToString(arg);
        } else if constexpr (std::is_same_v<T, std::string>) {
            ec.assign(0, ec.category());
            return arg;
        } else if constexpr (std::is_same_v<T, bool>) {
            ec.assign(0, ec.category());
            return arg ? std::string("true") : std::string("false");
        } else if constexpr (std::is_same_v<T, ArrayType> || std::is_same_v<T, ObjectType>) {
            ec = UserErrors::NotConvertible;
            return std::string{};
        } else {
            static_assert(FailType<T>{});
        }
    }, _value);
}

int EasyEvent::JsonValue::asInt(std::error_code& ec) const {
    return std::visit([&ec](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, NullValue>) {
            ec.assign(0, ec.category());
            return 0;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            if (arg < std::numeric_limits<int>::min() || arg > std::numeric_limits<int>::max()) {
                ec = UserErrors::OutOfRange;
                return 0;
            } else {
                ec.assign(0, ec.category());
                return static_cast<int>(arg);
            }
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            if (arg > std::numeric_limits<int>::max()) {
                ec = UserErrors::OutOfRange;
                return 0;
            } else {
                ec.assign(0, ec.category());
                return static_cast<int>(arg);
            }
        } else if constexpr (std::is_same_v<T, double>) {
            if (!InRange(arg, std::numeric_limits<int>::min(), std::numeric_limits<int>::max())) {
                ec = UserErrors::OutOfRange;
                return 0;
            } else {
                ec.assign(0, ec.category());
                return static_cast<int>(arg);
            }
        } else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, ArrayType> || std::is_same_v<T, ObjectType>) {
            ec = UserErrors::NotConvertible;
            return 0;
        } else if constexpr (std::is_same_v<T, bool>) {
            ec.assign(0, ec.category());
            return arg ? 1 : 0;
        } else {
            static_assert(FailType<T>{});
        }
    }, _value);
}

unsigned int EasyEvent::JsonValue::asUInt(std::error_code& ec) const {
    return std::visit([&ec](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, NullValue>) {
            ec.assign(0, ec.category());
            return 0u;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            if (arg < 0 || arg > std::numeric_limits<unsigned int>::max()) {
                ec = UserErrors::OutOfRange;
                return 0u;
            } else {
                ec.assign(0, ec.category());
                return static_cast<unsigned int>(arg);
            }
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            if (arg > std::numeric_limits<unsigned int>::max()) {
                ec = UserErrors::OutOfRange;
                return 0u;
            } else {
                ec.assign(0, ec.category());
                return static_cast<unsigned int>(arg);
            }
        } else if constexpr (std::is_same_v<T, double>) {
            if (!InRange(arg, 0, std::numeric_limits<unsigned int>::max())) {
                ec = UserErrors::OutOfRange;
                return 0u;
            } else {
                ec.assign(0, ec.category());
                return static_cast<unsigned int>(arg);
            }
        } else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, ArrayType> || std::is_same_v<T, ObjectType>) {
            ec = UserErrors::NotConvertible;
            return 0u;
        } else if constexpr (std::is_same_v<T, bool>) {
            ec.assign(0, ec.category());
            return arg ? 1u : 0u;
        } else {
            static_assert(FailType<T>{});
        }
    }, _value);
}

int64_t EasyEvent::JsonValue::asInt64(std::error_code& ec) const {
    return std::visit([&ec](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, NullValue>) {
            ec.assign(0, ec.category());
            return INT64(0);
        } else if constexpr (std::is_same_v<T, int64_t>) {
            ec.assign(0, ec.category());
            return arg;
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            if (arg > std::numeric_limits<int64_t>::max()) {
                ec = UserErrors::OutOfRange;
                return INT64(0);
            } else {
                ec.assign(0, ec.category());
                return static_cast<int64_t>(arg);
            }
        } else if constexpr (std::is_same_v<T, double>) {
            if (!InRange(arg, std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max())) {
                ec = UserErrors::OutOfRange;
                return INT64(0);
            } else {
                ec.assign(0, ec.category());
                return static_cast<int64_t>(arg);
            }
        } else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, ArrayType> || std::is_same_v<T, ObjectType>) {
            ec = UserErrors::NotConvertible;
            return INT64(0);
        } else if constexpr (std::is_same_v<T, bool>) {
            ec.assign(0, ec.category());
            return arg ? INT64(1) : INT64(0);
        } else {
            static_assert(FailType<T>{});
        }
    }, _value);
}

uint64_t EasyEvent::JsonValue::asUInt64(std::error_code& ec) const {
    return std::visit([&ec](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, NullValue>) {
            ec.assign(0, ec.category());
            return UINT64(0);
        } else if constexpr (std::is_same_v<T, int64_t>) {
            if (arg < 0) {
                ec = UserErrors::OutOfRange;
                return UINT64(0);
            } else {
                ec.assign(0, ec.category());
                return static_cast<uint64_t>(arg);
            }
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return arg;
        } else if constexpr (std::is_same_v<T, double>) {
            if (!InRange(arg, 0, std::numeric_limits<uint64_t>::max())) {
                ec = UserErrors::OutOfRange;
                return UINT64(0);
            } else {
                ec.assign(0, ec.category());
                return static_cast<uint64_t>(arg);
            }
        } else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, ArrayType> || std::is_same_v<T, ObjectType>) {
            ec = UserErrors::NotConvertible;
            return UINT64(0);
        } else if constexpr (std::is_same_v<T, bool>) {
            ec.assign(0, ec.category());
            return arg ? UINT64(1) : UINT64(0);
        } else {
            static_assert(FailType<T>{});
        }
    }, _value);
}

float EasyEvent::JsonValue::asFloat(std::error_code& ec) const {
    return std::visit([&ec](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, NullValue>) {
            ec.assign(0, ec.category());
            return 0.0f;
        } else if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t> || std::is_same_v<T, double>) {
            ec.assign(0, ec.category());
            return static_cast<float>(arg);
        } else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, ArrayType> || std::is_same_v<T, ObjectType>) {
            ec = UserErrors::NotConvertible;
            return 0.0f;
        } else if constexpr (std::is_same_v<T, bool>) {
            ec.assign(0, ec.category());
            return arg ? 1.0f : 0.0f;
        } else {
            static_assert(FailType<T>{});
        }
    }, _value);
}

double EasyEvent::JsonValue::asDouble(std::error_code& ec) const {
    return std::visit([&ec](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, NullValue>) {
            ec.assign(0, ec.category());
            return 0.0;
        } else if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t>) {
            ec.assign(0, ec.category());
            return static_cast<double>(arg);
        } else if constexpr (std::is_same_v<T, double>) {
            ec.assign(0, ec.category());
            return arg;
        } else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, ArrayType> || std::is_same_v<T, ObjectType>) {
            ec = UserErrors::NotConvertible;
            return 0.0;
        } else if constexpr (std::is_same_v<T, bool>) {
            ec.assign(0, ec.category());
            return arg ? 1.0 : 0.0;
        } else {
            static_assert(FailType<T>{});
        }
    }, _value);
}

bool EasyEvent::JsonValue::asBool(std::error_code& ec) const {
    return std::visit([&ec](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, NullValue>) {
            ec.assign(0, ec.category());
            return false;
        } else if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t>) {
            ec.assign(0, ec.category());
            return arg != 0;
        } else if constexpr (std::is_same_v<T, double>) {
            ec.assign(0, ec.category());
            return arg != 0.0;
        } else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, ArrayType> || std::is_same_v<T, ObjectType>) {
            ec = UserErrors::NotConvertible;
            return false;
        } else if constexpr (std::is_same_v<T, bool>) {
            ec.assign(0, ec.category());
            return arg;
        } else {
            static_assert(FailType<T>{});
        }
    }, _value);
}

bool EasyEvent::JsonValue::isInt() const {
    return std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, NullValue> ||
                      std::is_same_v<T, std::string> ||
                      std::is_same_v<T, bool> ||
                      std::is_same_v<T, ArrayType> ||
                      std::is_same_v<T, ObjectType>) {
            return false;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return arg >= std::numeric_limits<int>::min() && arg <= std::numeric_limits<int>::max();
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return arg <= std::numeric_limits<int>::max();
        } else if constexpr (std::is_same_v<T, double>) {
            return arg >= std::numeric_limits<int>::min() && arg <= std::numeric_limits<int>::max() && IsIntegral(arg);
        } else {
            static_assert(FailType<T>{});
        }
    }, _value);
}

bool EasyEvent::JsonValue::isInt64() const {
    return std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, NullValue> ||
                      std::is_same_v<T, std::string> ||
                      std::is_same_v<T, bool> ||
                      std::is_same_v<T, ArrayType> ||
                      std::is_same_v<T, ObjectType>) {
            return false;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return true;
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return arg <= std::numeric_limits<int64_t>::max();
        } else if constexpr (std::is_same_v<T, double>) {
            return arg >= (double)std::numeric_limits<int64_t>::min() &&
                   arg < (double)std::numeric_limits<int64_t>::max() && IsIntegral(arg);
        } else {
            static_assert(FailType<T>{});
        }
    }, _value);
}

bool EasyEvent::JsonValue::isUInt() const {
    return std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, NullValue> ||
                      std::is_same_v<T, std::string> ||
                      std::is_same_v<T, bool> ||
                      std::is_same_v<T, ArrayType> ||
                      std::is_same_v<T, ObjectType>) {
            return false;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return arg >= 0 && arg <= std::numeric_limits<unsigned int>::max();
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return arg <= std::numeric_limits<unsigned int>::max();
        } else if constexpr (std::is_same_v<T, double>) {
            return arg >= 0 && arg <= std::numeric_limits<unsigned int>::max() && IsIntegral(arg);
        } else {
            static_assert(FailType<T>{});
        }
    }, _value);
}

bool EasyEvent::JsonValue::isUInt64() const {
    return std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, NullValue> ||
                      std::is_same_v<T, std::string> ||
                      std::is_same_v<T, bool> ||
                      std::is_same_v<T, ArrayType> ||
                      std::is_same_v<T, ObjectType>) {
            return false;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return arg >= 0;
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return true;
        } else if constexpr (std::is_same_v<T, double>) {
            return arg >= 0 && arg < maxUInt64AsDouble && IsIntegral(arg);
        } else {
            static_assert(FailType<T>{});
        }
    }, _value);
}

bool EasyEvent::JsonValue::isIntegral() const {
    return std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, NullValue> ||
                      std::is_same_v<T, std::string> ||
                      std::is_same_v<T, bool> ||
                      std::is_same_v<T, ArrayType> ||
                      std::is_same_v<T, ObjectType>) {
            return false;
        } else if constexpr (std::is_same_v<T, int64_t> ||std::is_same_v<T, uint64_t>) {
            return true;
        } else if constexpr (std::is_same_v<T, double>) {
            return arg >= std::numeric_limits<int64_t>::min()  && arg < maxUInt64AsDouble && IsIntegral(arg);
        } else {
            static_assert(FailType<T>{});
        }
    }, _value);
}

bool EasyEvent::JsonValue::isConvertibleTo(JsonType other) const {
    switch (other) {
        case JsonType::NullValue: {
            return (isNumeric() && asDouble() == 0.0)
                   || (isTypeOf(JsonType::BoolValue) && !std::get<bool>(_value))
                   || (isTypeOf(JsonType::StringValue) && std::get<std::string>(_value).empty())
                   || (isTypeOf(JsonType::ArrayValue) && std::get<ArrayType>(_value).empty())
                   || (isTypeOf(JsonType::ObjectValue) && std::get<ObjectType>(_value).empty())
                   || (isTypeOf(JsonType::NullValue));
        }
        case JsonType::IntValue: {
            return isInt()
                   || (_value.type() == typeid(double)
                       && InRange(boost::get<double>(_value), std::numeric_limits<int>::min(),
                                  std::numeric_limits<int>::max()))
                   || _value.type() == typeid(bool)
                   || _value.type() == typeid(NullValue);
        }
        case JsonType::UintValue: {
            return isUInt()
                   || (_value.type() == typeid(double)
                       && InRange(boost::get<double>(_value), 0, std::numeric_limits<unsigned int>::max()))
                   || _value.type() == typeid(bool)
                   || _value.type() == typeid(NullValue);
        }
        case JsonType::RealValue: {
            return isNumeric() || _value.type() == typeid(bool) || _value.type() == typeid(NullValue);
        }
        case JsonType::StringValue: {
            return isNumeric() || _value.type() == typeid(bool) || _value.type() == typeid(std::string)
                   || _value.type() == typeid(NullValue);
        }
        case JsonType::BoolValue: {
            return isNumeric() || _value.type() == typeid(bool) || _value.type() == typeid(NullValue);
        }
        case JsonType::ArrayValue: {
            return _value.type() == typeid(ArrayType) || _value.type() == typeid(NullValue);
        }
        case JsonType::ObjectValue: {
            return _value.type() == typeid(ObjectType) || _value.type() == typeid(NullValue);
        }
    }
    Assert(false);
    return false; // unreachable
}

size_t JsonValue::size() const {
    if (_value.type() == typeid(ArrayType)) {
        return boost::get<ArrayType>(_value).size();
    } else if (_value.type() == typeid(ObjectType)) {
        return boost::get<ObjectType>(_value).size();
    } else {
        return 0;
    }
}

void JsonValue::clear() {
    if (_value.type() == typeid(ArrayType)) {
        boost::get<ArrayType>(_value).clear();
    } else if (_value.type() == typeid(ObjectType)) {
        boost::get<ObjectType>(_value).clear();
    } else if (_value.type() == typeid(NullValue)) {

    } else {
        NET4CXX_THROW_EXCEPTION(ValueError, "clear requires complex value");
    }
}

void JsonValue::resize(size_t newSize) {
    if (_value.type() == typeid(NullValue)) {
        *this = JsonValue(JsonType::arrayValue);
    }
    if (_value.type() != typeid(ArrayType)) {
        NET4CXX_THROW_EXCEPTION(ValueError, "resize requires array value");
    }
    boost::get<ArrayType>(_value).resize(newSize);
}

JsonValue& JsonValue::operator[](size_t index) {
    if (_value.type() == typeid(NullValue)) {
        *this = JsonValue(JsonType::arrayValue);
    } else if (_value.type() != typeid(ArrayType)) {
        NET4CXX_THROW_EXCEPTION(ValueError, "operator[](index) requires array value");
    }
    auto &array = boost::get<ArrayType>(_value);
    if (index >= array.size()) {
        array.resize(index + 1);
    }
    return array[index];
}

JsonValue& JsonValue::operator[](int index) {
    if (index < 0) {
        NET4CXX_THROW_EXCEPTION(ValueError, "index cannot be negative");
    }
    return (*this)[static_cast<size_t>(index)];
}

const JsonValue& JsonValue::operator[](size_t index) const {
    if (_value.type() == typeid(NullValue)) {
        return nullSingleton();
    } else if (_value.type() == typeid(ArrayType)) {
        auto &array = boost::get<ArrayType>(_value);
        if (index < array.size()) {
            return array[index];
        } else {
            return nullSingleton();
        }
    } else {
        NET4CXX_THROW_EXCEPTION(ValueError, "operator[](index)const requires array value");
    }
}

const JsonValue& JsonValue::operator[](int index) const {
    if (index < 0) {
        NET4CXX_THROW_EXCEPTION(ValueError, "index cannot be negative");
    }
    return (*this)[static_cast<size_t>(index)];
}

JsonValue& JsonValue::operator[](const char *key) {
    if (_value.type() == typeid(NullValue)) {
        *this = JsonValue(JsonType::objectValue);
    } else if (_value.type() != typeid(ObjectType)) {
        NET4CXX_THROW_EXCEPTION(ValueError, "operator[](key) requires object value");
    }
    auto &object = boost::get<ObjectType>(_value);
    auto it = object.find(key);
    if (it == object.end()) {
        it = object.insert(it, std::make_pair(key, nullSingleton()));
    }
    return it->second;
}

const JsonValue& JsonValue::operator[](const char *key) const {
    const JsonValue *found = find(key);
    return found ? *found : nullSingleton();
}

JsonValue JsonValue::get(const char *key, const JsonValue &defaultValue) const {
    const JsonValue *found = find(key);
    return found ? *found : defaultValue;
}

const JsonValue* JsonValue::find(const char *key) const {
    if (_value.type() == typeid(NullValue)) {
        return nullptr;
    } else if (_value.type() == typeid(ObjectType)) {
        auto &object = boost::get<ObjectType>(_value);
        auto it = object.find(key);
        return it != object.end() ? &(it->second) : nullptr;
    } else {
        NET4CXX_THROW_EXCEPTION(ValueError, "find requires object value or null value");
    }
}

bool JsonValue::removeMember(const char *key, JsonValue *removed) {
    if (_value.type() != typeid(ObjectType)) {
        return false;
    }
    auto &object = boost::get<ObjectType>(_value);
    auto it = object.find(key);
    if (it == object.end()) {
        return false;
    }
    if (removed) {
        *removed = std::move(it->second);
    }
    object.erase(it);
    return true;
}

bool JsonValue::removeIndex(size_t index, JsonValue *removed) {
    if (_value.type() != typeid(ArrayType)) {
        return false;
    }
    auto &array = boost::get<ArrayType>(_value);
    if (index >= array.size()) {
        return false;
    }
    if (removed) {
        *removed = std::move(array[index]);
    }
    array.erase(std::next(array.begin(), (std::ptrdiff_t)index));
    return true;
}

StringVector JsonValue::getMemberNames() const {
    StringVector members;
    if (_value.type() == typeid(NullValue)) {

    } else if (_value.type() == typeid(ObjectType)) {
        auto &object = boost::get<ObjectType>(_value);
        members.reserve(object.size());
        for (auto &kv: object) {
            members.push_back(kv.first);
        }
    } else {
        NET4CXX_THROW_EXCEPTION(ValueError, "getMemberNames requires object value");
    }
    return members;
}

std::string JsonValue::toStyledString() const {
    StreamWriterBuilder builder;
    std::string out = hasComment(COMMENT_BEFORE) ? "\n" : "";
    out += writeString(builder, *this);
    out += "\n";
    return out;
}

const JsonValue& JsonValue::nullSingleton() {
    static const JsonValue nullStatic;
    return nullStatic;
}


BuiltStyledStreamWriter::BuiltStyledStreamWriter(
        std::string indentation,
        CommentStyle cs,
        std::string colonSymbol,
        std::string nullSymbol,
        std::string endingLineFeedSymbol,
        bool useSpecialFloats,
        unsigned int precision)
        : _rightMargin(74)
        , _indentation(std::move(indentation))
        , _cs(cs)
        , _colonSymbol(std::move(colonSymbol))
        , _nullSymbol(std::move(nullSymbol))
        , _endingLineFeedSymbol(std::move(endingLineFeedSymbol))
        , _addChildValues(false)
        , _indented(false)
        , _useSpecialFloats(useSpecialFloats)
        , _precision(precision) {

}

int BuiltStyledStreamWriter::write(const JsonValue &root, std::ostream *sout) {
    _sout = sout;
    _addChildValues = false;
    _indented = true;
    _indentString.clear();
    writeCommentBeforeValue(root);
    if (!_indented) {
        writeIndent();
    }
    _indented = true;
    writeValue(root);
    writeCommentAfterValueOnSameLine(root);
    *_sout << _endingLineFeedSymbol;
    _sout = nullptr;
    return 0;
}

void BuiltStyledStreamWriter::writeValue(const JsonValue &value) {
    switch (value.type()) {
        case JsonType::nullValue: {
            pushValue(_nullSymbol);
            break;
        }
        case JsonType::intValue: {
            pushValue(std::to_string(value.asInt64()));
            break;
        }
        case JsonType::uintValue: {
            pushValue(std::to_string(value.asUInt64()));
            break;
        }
        case JsonType::realValue: {
            pushValue(valueToString(value.asDouble(), _useSpecialFloats, _precision));
            break;
        }
        case JsonType::stringValue: {
            pushValue(valueToQuotedString(value.asString()));
            break;
        }
        case JsonType::boolValue: {
            pushValue(value.asBool() ? "true" : "false");
            break;
        }
        case JsonType::arrayValue: {
            writeArrayValue(value);
            break;
        }
        case JsonType::objectValue: {
            StringVector members = value.getMemberNames();
            if (members.empty()) {
                pushValue("{}");
            } else {
                writeWithIndent("{");
                indent();
                for (auto it = members.begin();it != members.end(); ++ it) {
                    const std::string &name = *it;
                    const JsonValue &childValue = value[name];
                    writeCommentBeforeValue(childValue);
                    writeWithIndent(valueToQuotedString(name));
                    *_sout << _colonSymbol;
                    writeValue(childValue);
                    if (std::next(it) != members.end()) {
                        *_sout << ",";
                    }
                    writeCommentAfterValueOnSameLine(childValue);
                }
                unindent();
                writeWithIndent("}");
            }
            break;
        }
    }
}

void BuiltStyledStreamWriter::writeArrayValue(const JsonValue &value) {
    size_t size = value.size();
    if (size == 0) {
        pushValue("[]");
    } else {
        bool isMultiLine = (_cs == CommentStyle::All) || isMultilineArray(value);
        if (isMultiLine) {
            writeWithIndent("[");
            indent();
            bool hasChildValue = !_childValues.empty();
            for (size_t index = 0; index != size; ++index) {
                const JsonValue &childValue = value[index];
                writeCommentBeforeValue(childValue);
                if (hasChildValue) {
                    writeWithIndent(_childValues[index]);
                } else {
                    if (!_indented) {
                        writeIndent();
                    }
                    _indented = true;
                    writeValue(childValue);
                    _indented = false;
                }
                if (index + 1 != size) {
                    *_sout << ",";
                }
                writeCommentAfterValueOnSameLine(childValue);
            }
            unindent();
            writeWithIndent("]");
        } else {
            NET4CXX_ASSERT(_childValues.size() == size);
            *_sout << "[";
            if (!_indentation.empty()) {
                *_sout << " ";
            }
            for (unsigned index = 0; index < size; ++index) {
                if (index > 0) {
                    *_sout << ((!_indentation.empty()) ? ", " : ",");
                }
                *_sout << _childValues[index];
            }
            if (!_indentation.empty()) {
                *_sout << " ";
            }
            *_sout << "]";
        }
    }
}

bool BuiltStyledStreamWriter::isMultilineArray(const JsonValue &value) {
    size_t size = value.size();
    bool isMultiLine = size * 3 >= _rightMargin;
    _childValues.clear();
    for (size_t index = 0; index < size && !isMultiLine; ++index) {
        const JsonValue &childValue = value[index];
        isMultiLine = ((childValue.isArray() || childValue.isObject()) && !childValue.empty());
    }
    if (!isMultiLine) {
        _childValues.reserve(size);
        _addChildValues = true;
        size_t lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
        for (size_t index = 0; index < size; ++index) {
            if (hasCommentForValue(value[index])) {
                isMultiLine = true;
            }
            writeValue(value[index]);
            lineLength += _childValues[index].length();
        }
        _addChildValues = false;
        isMultiLine = isMultiLine || lineLength >= _rightMargin;
    }
    return isMultiLine;
}

void BuiltStyledStreamWriter::writeCommentBeforeValue(const JsonValue &root) {
    if (_cs == CommentStyle::None) {
        return;
    }
    if (!root.hasComment(COMMENT_BEFORE)) {
        return;
    }
    if (!_indented) {
        writeIndent();
    }
    const std::string &comment = root.getComment(COMMENT_BEFORE);
    for (auto iter = comment.begin(); iter != comment.end(); ++iter) {
        *_sout << *iter;
        if (*iter == '\n' && (iter + 1) != comment.end() && *(iter + 1) == '/') {
            *_sout << _indentString;
        }
    }
    _indented = false;
}

void BuiltStyledStreamWriter::writeCommentAfterValueOnSameLine(const JsonValue &root) {
    if (_cs == CommentStyle::None) {
        return;
    }
    if (root.hasComment(COMMENT_ON_SAME_LINE)) {
        *_sout << " " << root.getComment(COMMENT_ON_SAME_LINE);
    }
    if (root.hasComment(COMMENT_AFTER)) {
        writeIndent();
        *_sout << root.getComment(COMMENT_AFTER);
    }
}

bool BuiltStyledStreamWriter::hasCommentForValue(const JsonValue &value) {
    return value.hasComment(COMMENT_BEFORE)
           || value.hasComment(COMMENT_ON_SAME_LINE)
           || value.hasComment(COMMENT_AFTER);
}


StreamWriterBuilder::StreamWriterBuilder() {
    setDefaults(&_settings);
}

StreamWriter* StreamWriterBuilder::newStreamWriter() const {
    std::string indentation = _settings["indentation"].asString();
    std::string cs_str = _settings["commentStyle"].asString();
    bool eyc = _settings["enableYAMLCompatibility"].asBool();
    bool dnp = _settings["dropNullPlaceholders"].asBool();
    bool usf = _settings["useSpecialFloats"].asBool();
    unsigned int pre = _settings["precision"].asUInt();
    CommentStyle cs;
    if (cs_str == "All") {
        cs = CommentStyle::All;
    } else if (cs_str == "None") {
        cs = CommentStyle::None;
    } else {
        NET4CXX_THROW_EXCEPTION(ValueError, "commentStyle must be 'All' or 'None'");
    }
    std::string colonSymbol = " : ";
    if (eyc) {
        colonSymbol = ": ";
    } else if (indentation.empty()) {
        colonSymbol = ":";
    }
    std::string nullSymbol = "null";
    if (dnp) {
        nullSymbol.clear();
    }
    pre = std::min(pre, 17u);
    std::string endingLineFeedSymbol;
    return new BuiltStyledStreamWriter(std::move(indentation), cs, std::move(colonSymbol), std::move(nullSymbol),
                                       std::move(endingLineFeedSymbol), usf, pre);
}

static void getValidWriterKeys(StringSet *validKeys) {
    validKeys->clear();
    validKeys->insert("indentation");
    validKeys->insert("commentStyle");
    validKeys->insert("enableYAMLCompatibility");
    validKeys->insert("dropNullPlaceholders");
    validKeys->insert("useSpecialFloats");
    validKeys->insert("precision");
}

bool StreamWriterBuilder::validate(JsonValue *invalid) const {
    JsonValue myInvalid;
    if (!invalid) {
        invalid = &myInvalid;
    }
    JsonValue &inv = *invalid;
    StringSet validKeys;
    getValidWriterKeys(&validKeys);
    StringVector keys = _settings.getMemberNames();
    for (auto &key: keys) {
        if (validKeys.find(key) == validKeys.end()) {
            inv[key] = _settings[key];
        }
    }
    return inv.empty();
}

void StreamWriterBuilder::setDefaults(JsonValue *settings) {
    (*settings)["commentStyle"] = "All";
    (*settings)["indentation"] = "\t";
    (*settings)["enableYAMLCompatibility"] = false;
    (*settings)["dropNullPlaceholders"] = false;
    (*settings)["useSpecialFloats"] = false;
    (*settings)["precision"] = 17;
}


std::string writeString(const StreamWriter::Factory &factory, const JsonValue &root) {
    std::ostringstream sout;
    std::unique_ptr<StreamWriter> writer(factory.newStreamWriter());
    writer->write(root, &sout);
    return sout.str();
}

std::ostream& operator<<(std::ostream &sout, const JsonValue &root) {
    StreamWriterBuilder builder;
    std::unique_ptr<StreamWriter> writer(builder.newStreamWriter());
    writer->write(root, &sout);
    return sout;
}


bool BuiltReader::parse(const char *beginDoc, const char *endDoc, JsonValue &root, bool collectComments) {
    if (_features.allowComments) {
        collectComments = false;
    }
    _begin = beginDoc;
    _end = endDoc;
    _collectComments = collectComments;
    _current = _begin;
    _lastValueEnd = nullptr;
    _lastValue = nullptr;
    _commentsBefore.clear();
    _errors.clear();
    while (!_nodes.empty()) {
        _nodes.pop();
    }
    _nodes.push(&root);
    bool successful = readValue();
    Token token;
    skipCommentTokens(token);
    if (_features.failIfExtra) {
        if ((_features.strictRoot || token.type != tokenError) && token.type != tokenEndOfStream) {
            addError("Extra non-whitespace after json value.", token);
            return false;
        }
    }
    if (_collectComments && !_commentsBefore.empty()) {
        root.setComment(_commentsBefore, COMMENT_AFTER);
    }
    if (_features.strictRoot) {
        if (!root.isArray() && !root.isObject()) {
            token.type = tokenError;
            token.start = beginDoc;
            token.end = endDoc;
            addError("A valid json document must be either an array or an object value.", token);
            return false;
        }
    }
    return successful;
}

std::string BuiltReader::getFormattedErrorMessages() const {
    std::string formattedMessage;
    for (auto &error: _errors) {
        formattedMessage += "* " + getLocationLineAndColumn(error.token.start) + "\n";
        formattedMessage += "  " + error.message + "\n";
        if (error.extra) {
            formattedMessage += "See " + getLocationLineAndColumn(error.extra) + " for detail.\n";
        }
    }
    return formattedMessage;
}

std::vector<BuiltReader::StructuredError> BuiltReader::getStructuredErrors() const {
    std::vector<StructuredError> allErrors;
    for (auto &error: _errors) {
        StructuredError structured;
        structured.offsetStart = error.token.start - _begin;
        structured.offsetLimit = error.token.end - _begin;
        structured.message = error.message;
        allErrors.push_back(structured);
    }
    return allErrors;
}

bool BuiltReader::readToken(Token &token) {
    skipSpaces();
    token.start = _current;
    char c = getNextChar();
    bool ok = true;
    switch (c) {
        case '{':
            token.type = tokenObjectBegin;
            break;
        case '}':
            token.type = tokenObjectEnd;
            break;
        case '[':
            token.type = tokenArrayBegin;
            break;
        case ']':
            token.type = tokenArrayEnd;
            break;
        case '"':
            token.type = tokenString;
            ok = readString();
            break;
        case '\'':
            if (_features.allowSingleQuotes) {
                token.type = tokenString;
                ok = readStringSingleQuote();
            } else {
                ok = false;
            }
            break;
        case '/':
            token.type = tokenComment;
            ok = readComment();
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            token.type = tokenNumber;
            readNumber(false);
            break;
        case '-':
            if (readNumber(true)) {
                token.type = tokenNumber;
            } else {
                token.type = tokenNegInf;
                ok = _features.allowSpecialFloats && match("nfinity", 7);
            }
            break;
        case 't':
            token.type = tokenTrue;
            ok = match("rue", 3);
            break;
        case 'f':
            token.type = tokenFalse;
            ok = match("alse", 4);
            break;
        case 'n':
            token.type = tokenNull;
            ok = match("ull", 3);
            break;
        case 'N':
            if (_features.allowSpecialFloats) {
                token.type = tokenNaN;
                ok = match("aN", 2);
            } else {
                ok = false;
            }
            break;
        case 'I':
            if (_features.allowSpecialFloats) {
                token.type = tokenPosInf;
                ok = match("nfinity", 7);
            } else {
                ok = false;
            }
            break;
        case ',':
            token.type = tokenArraySeparator;
            break;
        case ':':
            token.type = tokenMemberSeparator;
            break;
        case 0:
            token.type = tokenEndOfStream;
            break;
        default:
            ok = false;
            break;
    }
    if (!ok) {
        token.type = tokenError;
    }
    token.end = _current;
    return true;
}

void BuiltReader::skipSpaces() {
    for (;_current != _end; ++ _current) {
        char c = *_current;
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
            break;
        }
    }
}

bool BuiltReader::match(const char *pattern, int patternLength) {
    if (_end - _current < patternLength) {
        return false;
    }
    int index = patternLength;
    while (index--) {
        if (_current[index] != pattern[index]) {
            return false;
        }
    }
    _current += patternLength;
    return true;
}

bool BuiltReader::readComment() {
    const char *commentBegin = _current - 1;
    char c = getNextChar();
    bool successful = false;
    if (c == '*') {
        successful = readCStyleComment();
    } else if (c == '/') {
        successful = readCppStyleComment();
    }
    if (!successful) {
        return false;
    }
    if (_collectComments) {
        CommentPlacement placement = COMMENT_BEFORE;
        if (_lastValueEnd && !containsNewLine(_lastValueEnd, commentBegin)) {
            if (c != '*' || !containsNewLine(commentBegin, _current)) {
                placement = COMMENT_ON_SAME_LINE;
            }
        }
        addComment(commentBegin, _current, placement);
    }
    return true;
}

bool BuiltReader::readCStyleComment() {
    while ((_current + 1) < _end) {
        char c = getNextChar();
        if (c == '*' && *_current == '/') {
            break;
        }
    }
    return getNextChar() == '/';
}

bool BuiltReader::readCppStyleComment() {
    while (_current != _end) {
        char c = getNextChar();
        if (c == '\n') {
            break;
        }
        if (c == '\r') {
            // Consume DOS EOL. It will be normalized in addComment.
            if (_current != _end && *_current == '\n') {
                getNextChar();
            }
            // Break on Mac OS 9 EOL.
            break;
        }
    }
    return true;
}

bool BuiltReader::readString() {
    char c = 0;
    while (_current != _end) {
        c = getNextChar();
        if (c == '\\') {
            getNextChar();
        } else if (c == '"') {
            break;
        }
    }
    return c == '"';
}

bool BuiltReader::readStringSingleQuote() {
    char c = 0;
    while (_current != _end) {
        c = getNextChar();
        if (c == '\\') {
            getNextChar();
        } else if (c == '\'') {
            break;
        }
    }
    return c == '\'';
}

bool BuiltReader::readNumber(bool checkInf) {
    const char *p = _current;
    if (checkInf && p != _end && *p == 'I') {
        _current = ++p;
        return false;
    }
    char c = '0';
    // integral part
    while (c >= '0' && c <= '9') {
        c = (_current = p) < _end ? *p++ : '\0';
    }

    // fractional part
    if (c == '.') {
        c = (_current = p) < _end ? *p++ : '\0';
        while (c >= '0' && c <= '9') {
            c = (_current = p) < _end ? *p++ : '\0';
        }
    }
    // exponential part
    if (c == 'e' || c == 'E') {
        c = (_current = p) < _end ? *p++ : '\0';
        if (c == '+' || c == '-') {
            c = (_current = p) < _end ? *p++ : '\0';
        }
        while (c >= '0' && c <= '9') {
            c = (_current = p) < _end ? *p++ : '\0';
        }
    }
    return true;
}

bool BuiltReader::readValue() {
    if (static_cast<int>(_nodes.size()) > _features.stackLimit) {
        NET4CXX_THROW_EXCEPTION(ParsingError, "Exceeded stackLimit in readValue().");
    }
    Token token;
    skipCommentTokens(token);
    bool successful = true;
    if (_collectComments && !_commentsBefore.empty()) {
        currentValue().setComment(std::move(_commentsBefore), COMMENT_BEFORE);
    }

    switch (token.type) {
        case tokenObjectBegin: {
            successful = readObject(token);
            break;
        }
        case tokenArrayBegin: {
            successful = readArray(token);
            break;
        }
        case tokenNumber: {
            successful = decodeNumber(token);
            break;
        }
        case tokenString: {
            successful = decodeString(token);
            break;
        }
        case tokenTrue: {
            JsonValue v(true);
            currentValue().swapPayload(v);
            break;
        }
        case tokenFalse: {
            JsonValue v(false);
            currentValue().swapPayload(v);
            break;
        }
        case tokenNull: {
            JsonValue v;
            currentValue().swapPayload(v);
            break;
        }
        case tokenNaN: {
            JsonValue v(std::numeric_limits<double>::quiet_NaN());
            currentValue().swapPayload(v);
            break;
        }
        case tokenPosInf: {
            JsonValue v(std::numeric_limits<double>::infinity());
            currentValue().swapPayload(v);
            break;
        }
        case tokenNegInf: {
            JsonValue v(-std::numeric_limits<double>::infinity());
            currentValue().swapPayload(v);
            break;
        }
        case tokenArraySeparator:
        case tokenObjectEnd:
        case tokenArrayEnd: {
            if (_features.allowDroppedNullPlaceholders) {
                --_current;
                JsonValue v;
                currentValue().swapPayload(v);
                break;
            }
        } // else, fall through ...
        default: {
            return addError("Syntax error: value, object or array expected.", token);
        }
    }

    if (_collectComments) {
        _lastValueEnd = _current;
        _lastValue = &currentValue();
    }

    return successful;
}

bool BuiltReader::readObject(Token &token) {
    Token tokenName;
    std::string name;
    JsonValue init(JsonType::objectValue);
    currentValue().swapPayload(init);
    while (readToken(tokenName)) {
        bool initialTokenOk = true;
        while (tokenName.type == tokenComment && initialTokenOk) {
            initialTokenOk = readToken(tokenName);
        }
        if (!initialTokenOk) {
            break;
        }
        if (tokenName.type == tokenObjectEnd && name.empty()) {
            return true;
        }
        name.clear();
        if (tokenName.type == tokenString) {
            if (!decodeString(tokenName, name)) {
                return recoverFromError(tokenObjectEnd);
            }
        } else if (tokenName.type == tokenNumber && _features.allowNumericKeys) {
            JsonValue numberName;
            if (!decodeNumber(tokenName, numberName)) {
                return recoverFromError(tokenObjectEnd);
            }
            name = numberName.asString();
        } else {
            break;
        }

        Token colon;
        if (!readToken(colon) || colon.type != tokenMemberSeparator) {
            return addErrorAndRecover("Missing ':' after object member name", colon, tokenObjectEnd);
        }
        if (name.length() >= (1U<<30)) {
            NET4CXX_THROW_EXCEPTION(ParsingError, "keylength >= 2^30");
        }
        if (_features.rejectDupKeys && currentValue().isMember(name)) {
            std::string msg = "Duplicate key: '" + name + "'";
            return addErrorAndRecover(msg, tokenName, tokenObjectEnd);
        }
        JsonValue &value = currentValue()[name];
        _nodes.push(&value);
        bool ok = readValue();
        _nodes.pop();
        if (!ok) {
            return recoverFromError(tokenObjectEnd);
        }

        Token comma;
        if (!readToken(comma) ||
            (comma.type != tokenObjectEnd && comma.type != tokenArraySeparator && comma.type != tokenComment)) {
            return addErrorAndRecover("Missing ',' or '}' in object declaration", comma, tokenObjectEnd);
        }
        bool finalizeTokenOk = true;
        while (comma.type == tokenComment && finalizeTokenOk) {
            finalizeTokenOk = readToken(comma);
        }
        if (comma.type == tokenObjectEnd) {
            return true;
        }
    }
    return addErrorAndRecover("Missing '}' or object member name", tokenName, tokenObjectEnd);
}

bool BuiltReader::readArray(Token &token) {
    JsonValue init(JsonType::arrayValue);
    currentValue().swapPayload(init);
    skipSpaces();
    if (_current != _end && *_current == ']') {
        Token endArray;
        readToken(endArray);
        return true;
    }
    int index = 0;
    for (;;) {
        JsonValue &value = currentValue()[index++];
        _nodes.push(&value);
        bool ok = readValue();
        _nodes.pop();
        if (!ok) {
            return recoverFromError(tokenArrayEnd);
        }

        Token currentToken;
        ok = readToken(currentToken);
        while (currentToken.type == tokenComment && ok) {
            ok = readToken(currentToken);
        }
        bool badTokenType = (currentToken.type != tokenArraySeparator && currentToken.type != tokenArrayEnd);
        if (!ok || badTokenType) {
            return addErrorAndRecover("Missing ',' or ']' in array declaration", currentToken, tokenArrayEnd);
        }
        if (currentToken.type == tokenArrayEnd) {
            break;
        }
    }
    return true;
}

bool BuiltReader::decodeNumber(Token &token) {
    JsonValue decoded;
    if (!decodeNumber(token, decoded)) {
        return false;
    }
    currentValue().swapPayload(decoded);
    return true;
}

bool BuiltReader::decodeNumber(Token &token, JsonValue &decoded) {
    const char *current = token.start;
    bool isNegative = *current == '-';
    if (isNegative) {
        ++current;
    }
    uint64_t maxIntegerValue = isNegative ? static_cast<uint64_t >(std::numeric_limits<int64_t>::max()) + 1
                                          : std::numeric_limits<uint64_t>::max();
    uint64_t threshold = maxIntegerValue / 10;
    uint64_t value = 0;
    while (current < token.end) {
        char c = *current++;
        if (c < '0' || c > '9') {
            return decodeDouble(token, decoded);
        }
        auto digit = static_cast<unsigned int>(c - '0');
        if (value >= threshold) {
            // We've hit or exceeded the max value divided by 10 (rounded down). If
            // a) we've only just touched the limit, b) this is the last digit, and
            // c) it's small enough to fit in that rounding delta, we're okay.
            // Otherwise treat this number as a double to avoid overflow.
            if (value > threshold || current != token.end || digit > maxIntegerValue % 10) {
                return decodeDouble(token, decoded);
            }
        }
        value = value * 10 + digit;
    }
    if (isNegative) {
        decoded = -static_cast<int64_t>(value);
    } else if (value <= std::numeric_limits<int64_t>::max()) {
        decoded = static_cast<int64_t>(value);
    } else {
        decoded = value;
    }
    return true;
}

bool BuiltReader::decodeString(Token &token) {
    std::string decodedString;
    if (!decodeString(token, decodedString)) {
        return false;
    }
    JsonValue decoded(decodedString);
    currentValue().swapPayload(decoded);
    return true;
}

bool BuiltReader::decodeString(Token &token, std::string &decoded) {
    decoded.reserve(static_cast<size_t >(token.end - token.start + 2));
    const char *current = token.start + 1;
    const char *end = token.end - 1;
    while (current != end) {
        char c = *current++;
        if (c == '"') {
            break;
        } else if (c == '\\') {
            if (current == end) {
                return addError("Empty escape sequence in string", token, current);
            }
            char escape = *current++;
            switch (escape) {
                case '"':
                    decoded += '"';
                    break;
                case '/':
                    decoded += '/';
                    break;
                case '\\':
                    decoded += '\\';
                    break;
                case 'b':
                    decoded += '\b';
                    break;
                case 'f':
                    decoded += '\f';
                    break;
                case 'n':
                    decoded += '\n';
                    break;
                case 'r':
                    decoded += '\r';
                    break;
                case 't':
                    decoded += '\t';
                    break;
                case 'u': {
                    unsigned int unicode;
                    if (!decodeUnicodeCodePoint(token, current, end, unicode)) {
                        return false;
                    }
                    decoded += codePointToUTF8(unicode);
                    break;
                }
                default:
                    return addError("Bad escape sequence in string", token, current);
            }
        } else {
            decoded += c;
        }
    }
    return true;
}

bool BuiltReader::decodeDouble(Token &token) {
    JsonValue decoded;
    if (!decodeDouble(token, decoded)) {
        return false;
    }
    currentValue().swapPayload(decoded);
    return true;
}

bool BuiltReader::decodeDouble(Token &token, JsonValue &decoded) {
    double value = 0;
    constexpr int bufferSize = 32;
    int count;
    ptrdiff_t length = token.end - token.start;

    if (length < 0) {
        return addError("Unable to parse token length", token);
    }

    // Avoid using a string constant for the format control string given to
    // sscanf, as this can cause hard to debug crashes on OS X. See here for more
    // info:
    //
    //     http://developer.apple.com/library/mac/#DOCUMENTATION/DeveloperTools/gcc-4.0.1/gcc/Incompatibilities.html
    char format[] = "%lf";

    if (length <= bufferSize) {
        char buffer[bufferSize + 1];
        memcpy(buffer, token.start, static_cast<size_t>(length));
        buffer[length] = 0;
        fixNumericLocaleInput(buffer, buffer + length);
        count = sscanf(buffer, format, &value);
    } else {
        std::string buffer(token.start, token.end);
        count = sscanf(buffer.c_str(), format, &value);
    }

    if (count != 1) {
        return addError("'" + std::string(token.start, token.end) + "' is not a number.", token);
    }
    decoded = value;
    return true;
}

bool BuiltReader::decodeUnicodeCodePoint(Token &token, const char *&current, const char *end, unsigned int &unicode) {
    if (!decodeUnicodeEscapeSequence(token, current, end, unicode)) {
        return false;
    }
    if (unicode >= 0xD800 && unicode <= 0xDBFF) {
        // surrogate pairs
        if (end - current < 6) {
            return addError("additional six characters expected to parse unicode surrogate pair.", token, current);
        }
        unsigned int surrogatePair;
        if (*(current++) == '\\' && *(current++) == 'u') {
            if (decodeUnicodeEscapeSequence(token, current, end, surrogatePair)) {
                unicode = 0x10000 + ((unicode & 0x3FF) << 10) + (surrogatePair & 0x3FF);
            } else {
                return false;
            }
        } else {
            return addError("expecting another \\u token to begin the second half of a unicode surrogate pair", token,
                            current);
        }
    }
    return true;
}

bool BuiltReader::decodeUnicodeEscapeSequence(Token &token, const char *&current, const char *end,
                                              unsigned int &unicode) {
    if (end - current < 4) {
        return addError("Bad unicode escape sequence in string: four digits expected.", token, current);
    }
    int result = 0;
    for (int index = 0; index < 4; ++index) {
        char c = *current++;
        result *= 16;
        if (c >= '0' && c <= '9') {
            result += c - '0';
        } else if (c >= 'a' && c <= 'f') {
            result += c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            result += c - 'A' + 10;
        } else {
            return addError("Bad unicode escape sequence in string: hexadecimal digit expected.", token, current);
        }
    }
    unicode = static_cast<unsigned int>(result);
    return true;

}

bool BuiltReader::addError(const std::string &message, Token &token, const char *extra) {
    ErrorInfo info;
    info.token = token;
    info.message = message;
    info.extra = extra;
    _errors.emplace_back(std::move(info));
    return false;
}

bool BuiltReader::recoverFromError(TokenType skipUntilToken) {
    size_t errorCount = _errors.size();
    Token skip;
    for (;;) {
        if (!readToken(skip)) {
            _errors.resize(errorCount); // discard errors caused by recovery
        }
        if (skip.type == skipUntilToken || skip.type == tokenEndOfStream) {
            break;
        }
    }
    _errors.resize(errorCount);
    return false;
}

void BuiltReader::getLocationLineAndColumn(const char *location, int &line, int &column) const {
    const char *current = _begin;
    const char *lastLineStart = current;
    line = 0;
    while (current < location && current != _end) {
        char c = *current++;
        if (c == '\r') {
            if (*current == '\n') {
                ++current;
            }
            lastLineStart = current;
            ++line;
        } else if (c == '\n') {
            lastLineStart = current;
            ++line;
        }
    }
    // column & line start at 1
    column = int(location - lastLineStart) + 1;
    ++line;
}

std::string BuiltReader::getLocationLineAndColumn(const char *location) const {
    int line, column;
    getLocationLineAndColumn(location, line, column);
    char buffer[18 + 16 + 16 + 1];
    snprintf(buffer, sizeof(buffer), "Line %d, Column %d", line, column);
    return std::string(buffer);
}

void BuiltReader::addComment(const char *begin, const char *end, CommentPlacement placement) {
    NET4CXX_ASSERT(_collectComments);
    std::string normalized = normalizeEOL(begin, end);
    if (placement == COMMENT_ON_SAME_LINE) {
        NET4CXX_ASSERT(_lastValue);
        _lastValue->setComment(normalized, placement);
    } else {
        _commentsBefore += normalized;
    }
}

void BuiltReader::skipCommentTokens(Token &token) {
    if (_features.allowComments) {
        do {
            readToken(token);
        } while (token.type == tokenComment);
    } else {
        readToken(token);
    }
}

std::string BuiltReader::normalizeEOL(const char *begin, const char *end) {
    std::string normalized;
    normalized.reserve(static_cast<size_t>(end - begin));
    const char *current = begin;
    while (current != end) {
        char c = *current++;
        if (c == '\r') {
            if (current != end && *current == '\n') {
                ++current;
            }
            // convert Mac EOL
            normalized += '\n';
        } else {
            normalized += c;
        }
    }
    return normalized;
}

bool BuiltReader::containsNewLine(const char *begin, const char *end) {
    for (; begin < end; ++begin) {
        if (*begin == '\n' || *begin == '\r') {
            return true;
        }
    }
    return false;
}


bool BuiltCharReader::parse(char const *beginDoc, char const *endDoc, JsonValue *root, std::string *errs) {
    bool ok = _reader.parse(beginDoc, endDoc, *root, _collectComments);
    if (errs) {
        *errs = _reader.getFormattedErrorMessages();
    }
    return ok;
}


CharReaderBuilder::CharReaderBuilder() {
    setDefaults(&_settings);
}

CharReader* CharReaderBuilder::newCharReader() const {
    bool collectComments = _settings["collectComments"].asBool();
    ReaderFeatures features;
    features.allowComments = _settings["allowComments"].asBool();
    features.strictRoot = _settings["strictRoot"].asBool();
    features.allowDroppedNullPlaceholders = _settings["allowDroppedNullPlaceholders"].asBool();
    features.allowNumericKeys = _settings["allowNumericKeys"].asBool();
    features.allowSingleQuotes = _settings["allowSingleQuotes"].asBool();
    features.stackLimit = _settings["stackLimit"].asInt();
    features.failIfExtra = _settings["failIfExtra"].asBool();
    features.rejectDupKeys = _settings["rejectDupKeys"].asBool();
    features.allowSpecialFloats = _settings["allowSpecialFloats"].asBool();
    return new BuiltCharReader(collectComments, features);
}

static void getValidReaderKeys(StringSet *validKeys) {
    validKeys->clear();
    validKeys->insert("collectComments");
    validKeys->insert("allowComments");
    validKeys->insert("strictRoot");
    validKeys->insert("allowDroppedNullPlaceholders");
    validKeys->insert("allowNumericKeys");
    validKeys->insert("allowSingleQuotes");
    validKeys->insert("stackLimit");
    validKeys->insert("failIfExtra");
    validKeys->insert("rejectDupKeys");
    validKeys->insert("allowSpecialFloats");
}

bool CharReaderBuilder::validate(JsonValue *invalid) const {
    JsonValue myInvalid;
    if (!invalid) {
        invalid = &myInvalid;
    }
    JsonValue &inv = *invalid;
    StringSet validKeys;
    getValidReaderKeys(&validKeys);
    StringVector keys = _settings.getMemberNames();
    for (auto &key: keys) {
        if (validKeys.find(key) == validKeys.end()) {
            inv[key] = _settings[key];
        }
    }
    return inv.empty();
}

void CharReaderBuilder::setDefaults(JsonValue *settings) {
    (*settings)["collectComments"] = true;
    (*settings)["allowComments"] = true;
    (*settings)["strictRoot"] = false;
    (*settings)["allowDroppedNullPlaceholders"] = false;
    (*settings)["allowNumericKeys"] = false;
    (*settings)["allowSingleQuotes"] = false;
    (*settings)["stackLimit"] = 1000;
    (*settings)["failIfExtra"] = false;
    (*settings)["rejectDupKeys"] = false;
    (*settings)["allowSpecialFloats"] = false;
}

void CharReaderBuilder::strictMode(JsonValue *settings) {
    (*settings)["allowComments"] = false;
    (*settings)["strictRoot"] = true;
    (*settings)["allowDroppedNullPlaceholders"] = false;
    (*settings)["allowNumericKeys"] = false;
    (*settings)["allowSingleQuotes"] = false;
    (*settings)["stackLimit"] = 1000;
    (*settings)["failIfExtra"] = true;
    (*settings)["rejectDupKeys"] = true;
    (*settings)["allowSpecialFloats"] = false;
}


bool parseFromStream(const CharReader::Factory &factory, std::istream &sin, JsonValue* root, std::string *errs) {
    std::ostringstream ssin;
    ssin << sin.rdbuf();
    std::string doc = ssin.str();
    char const* begin = doc.data();
    char const* end = begin + doc.size();
    std::unique_ptr<CharReader> reader(factory.newCharReader());
    return reader->parse(begin, end, root, errs);
}

std::istream& operator>>(std::istream &sin, JsonValue &root) {
    CharReaderBuilder b;
    std::string errs;
    bool ok = parseFromStream(b, sin, &root, &errs);
    if (!ok) {
        NET4CXX_THROW_EXCEPTION(ParsingError, errs);
    }
    return sin;
}