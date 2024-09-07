#pragma once
#define VERSION "v2.0.0 (SE breaks everything nice-edition)"

#include <string>
#include <stackchat/StackChat.hpp>

#include <nlohmann/json.hpp>
#include <dpp/dpp.h>
#include "ChatSite.hpp"

using Clock = std::chrono::system_clock;
using Timepoint = Clock::time_point;
using namespace std::literals;

namespace boson {

const std::string botHeader = "\\[ [Boson light](https://github.com/LunarWatcher/boson-light.cpp) \\] ";
const std::string userAgent = "Boson-light.cpp (+https://github.com/LunarWatcher/boson-light.cpp)";

struct Room {
    std::variant<int, std::string> roomId;
    ChatSite site;
    std::string apiSite;

    Timepoint last;
};

class ChatProvider {
public:
    virtual void sendMessage(
        Room& dst,
        const std::string& message,
        const std::string& author,
        const std::string& authorLink,
        const std::string& commentLink,
        const std::string& license
    ) = 0;
    virtual void registerRoom(Room& dst) = 0;
};

class AliveCommand : public stackchat::Command {
public:
    void onMessage(stackchat::Room &room, const stackchat::ChatEvent& ev, const std::vector<std::string>&) override {
        room.reply(ev, "[Not for long](https://www.youtube.com/watch?v=9QlfghSkMV4)");
    }
};
class StackChatProvider : public ChatProvider {
private:
    stackchat::StackChat chat;
public:
    StackChatProvider(const nlohmann::json& j);

    void registerRoom(Room& dst) override;
    void sendMessage(
        Room& dst,
        const std::string& message,
        const std::string& author,
        const std::string& authorLink,
        const std::string& commentLink,
        const std::string& license
    ) override;
};

class DiscordChatProvider : public ChatProvider {
private:
    dpp::cluster bot;

public:
    DiscordChatProvider(const nlohmann::json& j);

    void registerRoom(Room&) override {}
    void sendMessage(
        Room& dst,
        const std::string& message,
        const std::string& author,
        const std::string& authorLink,
        const std::string& commentLink,
        const std::string& license
    ) override;

};

}
