#include "ChatProvider.hpp"
#include "boson/ChatSite.hpp"
#include "message.h"
#include <format>

namespace boson {

// StackChat {{{
StackChatProvider::StackChatProvider(const nlohmann::json& j) :
    chat({
        stackchat::ChatConfig {
            .email{j.at("email").get<std::string>()},
            .password{j.at("password").get<std::string>()},
            .prefix{"~"},
            .userAgent{userAgent},
        }
    }) 
{

    chat.registerCommand("alive", std::make_shared<AliveCommand>());

    chat.join(stackchat::StackSite::STACKOVERFLOW, 197325);
    chat.sendTo(stackchat::StackSite::STACKOVERFLOW, 197325, botHeader + "Archivist " VERSION " starting");
}

void StackChatProvider::registerRoom(Room& dst) {
    chat.join(ctToStackSite(dst.site), std::get<int>(dst.roomId));
}

void StackChatProvider::sendMessage(Room& dst, const std::string&, const std::string& author, const std::string& authorLink, const std::string& commentLink) {
    auto roomSite = ctToStackSite(dst.site);

    chat.sendTo(roomSite, std::get<int>(dst.roomId),
                fmt::format("{} New comment posted by [{}]({})", boson::botHeader, author, authorLink));
    chat.sendTo(
        roomSite, std::get<int>(dst.roomId),
        commentLink
    );
}
// }}}
// Discord {{{

DiscordChatProvider::DiscordChatProvider(const nlohmann::json& j) : bot(j.at("discord_token")) {

    const std::string log_name = "mybot.log";
     
    /* Integrate spdlog logger to D++ log events */
    bot.on_log([this](const dpp::log_t & event) {
        switch (event.severity) {
            case dpp::ll_trace:
                spdlog::trace("{}", event.message);
            break;
            case dpp::ll_debug:
            spdlog::debug("{}", event.message);
            break;
            case dpp::ll_info:
                spdlog::info("{}", event.message);
            break;
            case dpp::ll_warning:
                spdlog::warn("{}", event.message);
            break;
            case dpp::ll_error:
                spdlog::error("{}", event.message);
            break;
            case dpp::ll_critical:
            default:
                spdlog::critical("{}", event.message);
            break;
        }
    });
     

    bot.on_slashcommand([](auto event) {
        if (event.command.get_command_name() == "alive") {
            event.reply("[No](<https://www.youtube.com/watch?v=YmN5--dIICc>)");
        }
    });

    bot.on_ready([this](auto event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_command_create(
                dpp::slashcommand("alive", "Please don't, I'm busy working here.", bot.me.id)
            );
        }
    });

    bot.start(dpp::st_return);
    spdlog::info("Connected to Discord");
}

void DiscordChatProvider::sendMessage(Room& dst, const std::string& message, const std::string& author, const std::string& authorLink, const std::string& commentLink) {
    dpp::embed embed;
    embed.set_title("New comment posted")
        .set_url(commentLink)
        .set_description(
            std::format("{}\n\n**Posted by:** [{}]({})", message, author, authorLink));

    bot.message_create_sync({
        std::get<std::string>(dst.roomId),
        //std::format("New message by [{}](<{}>) (<{}>):\n>>> {}", author, authorLink, commentLink, message)
        embed
    });
}
// }}}
}
