#include "fmt/core.h"
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

struct Rooms {
    int roomId;
    stackchat::StackSite site;
    std::string apiSite;

    Timepoint last;
};

const std::string botHeader = "\\[ [Boson light](https://github.com/LunarWatcher/boson-light.cpp) \\] ";
const std::string userAgent = "Boson-light.cpp (+https://github.com/LunarWatcher/boson-light.cpp)";

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

    stackapi::StackAPI api {
        stackapi::APIConfig {
            .apiKey{"FZBSdF)yDwousbUB9lIEog(("},
            .userAgent{userAgent},
            
        }
    };
    chat.join(stackchat::StackSite::STACKOVERFLOW, 197325);
    chat.sendTo(stackchat::StackSite::STACKOVERFLOW, 197325, botHeader + "Archivist starting");

    Timepoint initNow = Clock::now() - std::chrono::seconds(60);
    std::vector<Rooms> rooms = {
        {
            197325,
            stackchat::StackSite::STACKOVERFLOW,
            "meta.stackoverflow",
            initNow
        },

    };


    try {
        for (auto& room : rooms) {
            chat.join(room.site, room.roomId);
        }

        while (true) {
            Timepoint cycleTime = Clock::now();
            for (auto& [targetRoomID, roomSite, apiSite, lastTime] : rooms) {
                // there's realistically never going to be 100 comments in a short span of time
                auto res = api.get<stackapi::Comment>(
                    "comments",
                    {{"fromdate", std::to_string(std::chrono::duration_cast<std::chrono::seconds>(lastTime.time_since_epoch()).count())}},
                    { .site{apiSite}, .filter{"!nOedRLmfyw"} }
                );
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

            }

            std::this_thread::sleep_for(std::chrono::minutes(1));
        }
    } catch (...) {
        // I'm sure this won't go wrong :)
        chat.sendTo(stackchat::StackSite::STACKOVERFLOW, 197325, "@Zoe Boson died. See logs and restart");
        throw;
    }


}
