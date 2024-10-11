# boson-light.cpp

Light-weight version of [Boson](https://github.com/sobotics/boson) to run the meta comment archives on Stack Exchange chat (~~[Meta.SO](https://chat.stackoverflow.com/rooms/197298/meta-stack-overflow-comment-archive), [Meta.SE](https://chat.meta.stackexchange.com/rooms/1702/meta-stack-exchange-comment-archive)~~ Moved to [Discord](https://discord.gg/pcBMbMqA79) after SE broke bot login)

## Configuring

Configuring is done in `config.json`. Start by renaming `config.json.example` if you don't already have a `config.json`. From there, fill in the values:

* email and password: self-explanatory, the credentials for the archive chatbot. Include only if using SE chat
* discord_token: token for discord. Include only if using Discord.
* `archive_rooms`: list of rooms and sites, in the format documented in the example. At least one `archive_room` is required. Multiple sites can feed into the same room, but this is discouraged for readability purposes. Child keys for each room object:
    * `site`: The site of the chatroom. Either stackoverflow, stackexchange, metastackexchange, or discord
    * `room_id`: The room to post in on the chat site. Numeric if using SE chat, string if using Discord
    * `api_source`: The name of the site as it's referred to in the API. See https://api.stackexchange.com/docs/sites for the list
* `uptime_monitor`: optional; sends a ping to an uptime monitor. Only functions if not null. Note that an empty payload is sent, so this only works with active ping uptime monitors (such as [Uptime-Kuma](https://github.com/louislam/uptime-kuma)). If you have no idea what this is or why you need it, you don't need to worry about it. For those who do, the only difference between this and monitoring if Boson's service is still up is that this also responds to cloudflare outages that don't necessarily take down the bot.

### Discord vs. SE chat

In September 2024, SE made it a pain in the ass for bots to function. In response, Boson now supports Discord as an alternative (or in addition to) main SE.

To disable discord support, don't include `discord_token` in the config file.

To disable SE chat support, don't include `email` and `password` in the config file.


