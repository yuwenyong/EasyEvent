//
// Created by yuwenyong on 2021/1/3.
//

#ifndef EASYEVENT_CONFIGURATION_OPTIONS_H
#define EASYEVENT_CONFIGURATION_OPTIONS_H

#include "EasyEvent/Common/Config.h"
#include "EasyEvent/Common/Error.h"
#include "EasyEvent/Common/Utility.h"

namespace EasyEvent {



    class EASY_EVENT_API Options: NonCopyable {
    public:
        Options() = default;

        void addOpt(const std::string& shortOpt, const std::string& longOpt="", bool needArg=false,
                    const std::string& dflt="", bool repeat=false);

        std::vector<std::string> parse(const std::vector<std::string>& args);

        std::vector<std::string> parse(int argc, const char* const argv[]);

        bool isSet(const std::string& opt) const;

        std::string optVal(const std::string& opt) const;

        std::vector<std::string> optVals(const std::string& opt) const;

        static std::vector<std::string> split(const std::string& line);

        static Options& instance() {
            static Options options;
            return options;
        }
    private:
        enum class OptType: uint8 {
            ShortOpt = 0x00,
            LongOpt = 0x01
        };

        struct Opt {
            OptType type;
            bool needArg;
            bool repeat;
            bool hasDefault;
        };

        using OptPtr = std::shared_ptr<Opt>;

        struct OptVal {
            std::string val;
        };

        using OptValPtr = std::shared_ptr<OptVal>;

        struct OptVals {
            std::vector<std::string> vals;
        };

        using OptValsPtr = std::shared_ptr<OptVals>;

        using OptMapping = std::map<std::string, OptPtr>;
        using OptValMapping = std::map<std::string, OptValPtr>;
        using OptValsMapping = std::map<std::string, OptValsPtr>;
        using Synonyms = std::map<std::string, std::string>;

        void addValidOpt(const std::string& shortOpt, const std::string& longOpt, bool needArg,
                         const std::string& dflt, bool repeat);

        OptMapping::iterator checkOpt(const std::string& opt, OptType optType);

        void setOpt(const std::string& opt1, const std::string& opt2, const std::string& val, bool repeat);

        void setNonRepeatingOpt(const std::string& opt, const std::string& val);

        void setRepeatingOpt(const std::string& opt, const std::string& val);

        OptMapping::const_iterator checkOptIsValid(const std::string& opt) const;

        OptMapping::const_iterator checkOptHasArg(const std::string& opt) const;

        void updateSynonyms(const std::string& shortOpt, const std::string& longOpt);

        std::string getSynonym(const std::string& optName) const;

        static void checkArgs(const std::string& shortOpt, const std::string& longOpt, bool needArg,
                              const std::string& dflt);

        OptMapping _opts;
        OptValMapping _optVal;
        OptValsMapping _optVals;
        Synonyms _synonyms;
        bool _parsed{false};
        mutable std::mutex _mutex;
    };

}

#endif //EASYEVENT_CONFIGURATION_OPTIONS_H
