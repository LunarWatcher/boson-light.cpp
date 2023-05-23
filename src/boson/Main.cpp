#include "fmt/core.h"
#include "stackchat/chat/Command.hpp"
#include "stackchat/rooms/StackSite.hpp"
#include <exception>
#include <fstream>
#include <nlohmann/json.hpp>

#include <stackchat/StackChat.hpp>
#include <stackapi/StackAPI.hpp>
#include <stackapi/data/structs/comments/Comment.hpp>

#include <string>
#include <chrono>
#include <thread>

using Clock = std::chrono::system_clock;
using Timepoint = Clock::time_point;

struct Room {
    int roomId;
    stackchat::StackSite site;
    std::string apiSite;

    Timepoint last;
};

const std::string botHeader = "\\[ [Boson light](https://github.com/LunarWatcher/boson-light.cpp) \\] ";
const std::string userAgent = "Boson-light.cpp (+https://github.com/LunarWatcher/boson-light.cpp)";
std::atomic<bool> running = true;

void runner (stackchat::StackChat& chat, stackapi::StackAPI& api, Room room) {
    chat.join(room.site, room.roomId);
    try {
        while (running) {
            Timepoint cycleTime = Clock::now();
            auto& [targetRoomID, roomSite, apiSite, lastTime] = room;
            // there's realistically never going to be 100 comments in a short span of time
            auto res = api.get<stackapi::Comment>(
                "comments",
                {{"fromdate", std::to_string(std::chrono::duration_cast<std::chrono::seconds>(lastTime.time_since_epoch()).count())}},
                { .site{apiSite}, .filter{"!nOedRLmfyw"} }
            );
            spdlog::info("{}: {} new comments", room.apiSite, res.items.size());
            if (res.items.size()) {
                for (auto& comment : res.items) {
                    chat.sendTo(roomSite, targetRoomID,
                                fmt::format("{} New comment posted by [{}]({})", botHeader, comment.owner.display_name, comment.owner.link));
                    chat.sendTo(
                        roomSite, targetRoomID,
                        comment.link
                    );
                }
            }

            lastTime = cycleTime;


            std::this_thread::sleep_for(std::chrono::minutes(1));
        }
    } catch (std::exception& e) {
        spdlog::error(e.what());
        running.store(false);
        throw;
    } catch (...) {
        spdlog::error("Non-exception error caught.");
        running.store(false);
        throw;
    }
}

class AliveCommand : public stackchat::Command {
public:
    void onMessage(stackchat::Room &room, const stackchat::ChatEvent& ev, const std::vector<std::string>&) override {
        room.reply(ev, "[Not for long](https://www.youtube.com/watch?v=9QlfghSkMV4)");
    }
};

int main() {
    std::ifstream ss("config.json");
    if (!ss.is_open()) {
        throw std::runtime_error("Failed to find config file");
    }

    nlohmann::json j = nlohmann::json::parse(ss);

    auto email = j["email"].get<std::string>();
    auto password = j["password"].get<std::string>();

    stackchat::StackChat chat {
        stackchat::ChatConfig {
            .email{email},
                .password{password},
                .userAgent{userAgent}
        }
    };
    chat.registerCommand("alive", std::make_shared<AliveCommand>());

    stackapi::StackAPI api {
        stackapi::APIConfig {
            .apiKey{"FZBSdF)yDwousbUB9lIEog(("},
                .userAgent{userAgent},

        }
    };
    chat.join(stackchat::StackSite::STACKOVERFLOW, 197325);
    chat.sendTo(stackchat::StackSite::STACKOVERFLOW, 197325, botHeader + "Archivist starting");

    Timepoint initNow = Clock::now() - std::chrono::seconds(60);
    std::vector<Room> rooms = {
        {
            197298,
            stackchat::StackSite::STACKOVERFLOW,
            "meta.stackoverflow",
            initNow
        },

    };

    std::vector<std::thread> threads;

    for (auto& room : rooms) {
        threads.push_back(std::thread(std::bind(runner, std::ref(chat), std::ref(api), room)));
    }

    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
    spdlog::error("Thread error");

}
