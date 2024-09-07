#include <cpr/cpr.h>
#include "fmt/core.h"
#include "stackapi/data/structs/APIResponse.hpp"
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

#include <dpp/dpp.h>

#include <string>
#include <chrono>
#include <thread>

#include "ChatProvider.hpp"
#include "ChatSite.hpp"


std::atomic<bool> running = true;




void runner (stackapi::StackAPI& api, std::map<std::string, std::shared_ptr<boson::ChatProvider>> providers, boson::Room& room) {
    auto provider = providers.at(boson::ctToProvider(room.site));
    provider->registerRoom(room);

    try {
        while (running) {
            auto& [targetRoomID, roomSite, apiSite, lastTime] = room;
            stackapi::APIResponse<stackapi::Comment> res;
            do {
                int page = 1;
                res = api.get<stackapi::Comment>(
                    "comments",
                    {{"fromdate", std::to_string(std::chrono::duration_cast<std::chrono::seconds>(lastTime.time_since_epoch()).count())}},
                    { .site{apiSite}, .filter{"!6WPIompltRXVK"}, .page = page }
                );
                spdlog::debug("{}: {} new comments", room.apiSite, res.items.size());
                if (res.items.size()) {
                    for (auto& comment : res.items) {
                        //chat.sendTo(roomSite, std::get<int>(targetRoomID),
                                    //fmt::format("{} New comment posted by [{}]({})", boson::botHeader, comment.owner.display_name, comment.owner.link));
                        //chat.sendTo(
                            //roomSite, std::get<int>(targetRoomID),
                            //comment.link
                        //);
                        provider->sendMessage(room, comment.body_markdown, comment.owner.display_name, comment.owner.link, comment.link);
                    }
                }
                ++page;
            } while (res.has_more);
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


int main() {
    std::ifstream ss("config.json");
    if (!ss.is_open()) {
        throw std::runtime_error("Failed to find config file");
    }

    nlohmann::json j = nlohmann::json::parse(ss);
    bool hasError = false;

    std::map<std::string, std::shared_ptr<boson::ChatProvider>> chatProviders;

    stackapi::StackAPI api {
        stackapi::APIConfig {
            .apiKey{"FZBSdF)yDwousbUB9lIEog(("},
            .userAgent{boson::userAgent},
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

    if (j.contains("email")) {
        chatProviders["stackexchange"] = std::make_shared<boson::StackChatProvider>(j);
    }
    if (j.contains("discord_token")) {
        chatProviders["discord"] = std::make_shared<boson::DiscordChatProvider>(j);
    }

    Timepoint initNow = Clock::now() - std::chrono::seconds(60);
    std::vector<boson::Room> rooms;

    for (auto& archiveRoom : j.at("archive_rooms")) {
        auto rid = archiveRoom.at("room_id");
        auto site = archiveRoom.at("site").get<std::string>();

        std::variant<int, std::string> roomId;

        if (site == "discord") {
            roomId = rid.get<std::string>();
        } else {
            roomId = rid.get<int>();
        }

        
        rooms.push_back({
            roomId,
            boson::strToCt(site),
            archiveRoom.at("api_source"),
            initNow
        });
    }

    std::vector<std::thread> threads;

    for (auto& room : rooms) {
        threads.push_back(std::thread(std::bind(runner, std::ref(api), chatProviders, room)));
    }

    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
    spdlog::error("Thread error");

}
