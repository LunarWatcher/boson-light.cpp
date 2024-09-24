#pragma once

#include <string>
#include <map>

namespace boson {

const static std::map<std::string, std::string> htmlEntities = {
    {"&lt;", "<"},
    {"&#60", "<"},
    {"&gt;", ">"},
    {"&#62", ">"},
    {"&quot;", "\""},
    {"&#34;", "\""},
    {"&#39;", "'"},
    {"&#xA;", " "},
    // I hope this is never needed
    // But if it is, y u do dis to meh
    {"&#0;", ""},

    // NOTE: This always has to be last
    {"&amp;", "&"},
};

inline void replaceAll(std::string& inout, const std::string& source, const std::string& repl) {
    size_t start_pos = 0;
    while((start_pos = inout.find(source, start_pos)) != std::string::npos) {
        inout.replace(start_pos, source.length(), repl);
        start_pos += repl.length();
    }
}

inline std::string htmlDecode(std::string in) {
    for (auto& [entity, real] : htmlEntities) {
        replaceAll(in, entity, real);
    }

    return in;
}
}

