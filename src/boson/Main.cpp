#include <cpr/cpr.h>
#include "fmt/core.h"
#include "stackchat/StackChat.hpp"
#include "stackchat/chat/ChatEvent.hpp"
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

#define VERSION "v1.0.0 (Sysadmin Version)"

using Clock = std::chrono::system_clock;
using Timepoint = Clock::time_point;
using namespace std::literals;

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
            auto& [targetRoomID, roomSite, apiSite, lastTime] = room;
            // there's realistically never going to be 100 comments in a short span of time
            auto res = api.get<stackapi::Comment>(
                "comments",
                {{"fromdate", std::to_string(std::chrono::duration_cast<std::chrono::seconds>(lastTime.time_since_epoch()).count())}},
                { .site{apiSite}, .filter{"!nOedRLmfyw"} }
            );
            spdlog::debug("{}: {} new comments", room.apiSite, res.items.size());
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

            lastTime = Clock::now();


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
    bool hasError = false;

    stackchat::StackChat chat {
        stackchat::ChatConfig {
            .email{email},
            .password{password},
            .prefix{"~"},
            .userAgent{userAgent},
        }
    };
    chat.registerCommand("alive", std::make_shared<AliveCommand>());

    stackapi::StackAPI api {
        stackapi::APIConfig {
            .apiKey{"FZBSdF)yDwousbUB9lIEog(("},
            .userAgent{userAgent},
            .errorCallback = [&](stackapi::FaultType type, const std::string& message) {
                if (type == stackapi::FaultType::RESTORED && hasError) {
                    hasError = false;
                    spdlog::info("Cloudflare stopped being a cunt");
                } else if (type == stackapi::FaultType::CLOUDFLARE) {
                    hasError = true;
                    spdlog::error("Cloudflare is being a cunt again");
                }
            }
        }
    };

    std::thread uptimeMonitor;
    if (j.contains("uptime_monitor") && j.at("uptime_monitor").is_string()) {
        std::string uptimeURL = j.at("uptime_monitor").get<std::string>();
        uptimeMonitor = std::thread([uptimeURL, &hasError]() {
            // Make sure the service isn't erroneously reported as being up 
            std::this_thread::sleep_for(15s);

            while (true) {
                if (!hasError) {
                    cpr::Get(cpr::Url{uptimeURL}, cpr::VerifySsl(0));
                }
                std::this_thread::sleep_for(60s);
            }
        });
    }

    // The workshop room; this is the only room the bot always joins.
    // This should be fun if anyone else starts an instance for a child meta.
    chat.join(stackchat::StackSite::STACKOVERFLOW, 197325);
    chat.sendTo(stackchat::StackSite::STACKOVERFLOW, 197325, botHeader + "Archivist " VERSION " starting");

    Timepoint initNow = Clock::now() - std::chrono::seconds(60);
    std::vector<Room> rooms;

    for (auto& archiveRoom : j.at("archive_rooms")) {
        rooms.push_back({
            archiveRoom.at("room_id").get<int>(),
            archiveRoom.at("site").get<stackchat::StackSite>(),
            archiveRoom.at("api_source"),
            initNow
        });
    }

    std::vector<std::thread> threads;

    for (auto& room : rooms) {
        threads.push_back(std::thread(std::bind(runner, std::ref(chat), std::ref(api), room)));
    }

    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
    spdlog::error("Thread error");

}
