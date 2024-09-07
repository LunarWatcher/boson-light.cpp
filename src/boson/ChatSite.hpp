#pragma once

#include "stackchat/rooms/StackSite.hpp"
#include <stdexcept>
namespace boson {
enum class ChatSite {
    SO,
    SE,
    MSE,
    DISCORD
};

inline stackchat::StackSite ctToStackSite(const ChatSite& src) {
    switch (src) {
    case ChatSite::SO:
        return stackchat::StackSite::STACKOVERFLOW;
    case ChatSite::SE:
        return stackchat::StackSite::STACKEXCHANGE;
    case ChatSite::MSE:
        return stackchat::StackSite::META_STACKEXCHANGE;
    case ChatSite::DISCORD:
        throw std::runtime_error("Wrong type (programmer error)");
    }
}

inline std::string ctToProvider(const ChatSite& ct) {
    switch (ct) {
    case ChatSite::SO:
    case ChatSite::SE:
    case ChatSite::MSE:
        return "stackexchange";
    case ChatSite::DISCORD:
        return "discord";
    }
}

inline ChatSite strToCt(const std::string& src) {
    if (src == "discord") {
        return ChatSite::DISCORD;
    } else if (src == "stackoverflow") {
        return ChatSite::SO;
    } else if (src == "stackexchange") {
        return ChatSite::SE;
    } else if (src == "metastackexchange") {
        return ChatSite::MSE;
    }

    throw std::runtime_error("Invalid provider: " + src);
}

}

