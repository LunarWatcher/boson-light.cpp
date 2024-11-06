#pragma once

#include "boson/ChatProvider.hpp"
#include "boson/util/Parsing.hpp"
#include "stackapi/StackAPI.hpp"
#include <sstream>
#include <string>
#include <shared_mutex>
#include <map>
#include <stackapi/data/structs/posts/Post.hpp>

namespace boson::TitleProvider {

struct PostInfo {
    
    std::string title;
    Clock::time_point lastChecked;
};

static std::shared_mutex m;
static std::map<std::pair<std::string, long long>, PostInfo> posts;

/**
 * Note that a lock must be acquired prior to calling this function.
 */
inline void purgePosts() {
    std::erase_if(
        posts,
        [](const auto& entry) -> bool {
            return Clock::now() - entry.second.lastChecked > std::chrono::days(14);
        }
    );
}

inline std::string getTitle(const std::string& apiSite, long long postId) {
    std::shared_lock<std::shared_mutex> l(m);
    std::pair<std::string, long long> k{apiSite, postId};
    if (posts.contains(k)) {
        return posts.at(k).title;
    }
    return "[Failed to resolve title; please ping Zoe]";
}

inline void resolveTitles(
    stackapi::StackAPI& api,
    const std::string& apiSite,
    std::set<long long> ids
) {
    // No point in locking if the list is empty
    if (ids.size() == 0) {
        return;
    }
    {
        std::shared_lock<std::shared_mutex> l(m);

        // Check if there's a need to update any titles.
        // The update happens if there's unindexed titles, or if any of the titles haven't been
        // checked in over 30 minutes.
        // To conserve quota, all items in the batch are checked regardless of whether or not each entry
        // needs updates.
        if (std::find_if(
            ids.begin(), ids.end(),
            [&](const auto& id) {
                std::pair<std::string, long long> k{apiSite, id};
                bool res = posts.contains(k);
                
                return !res || Clock::now() - posts.at(k).lastChecked > 30min;
            }
        ) == ids.end()) {
            spdlog::debug("No title updates needed.");
            return;
        }
    }

    spdlog::debug("Updating titles for {} posts", ids.size());

    // We know ids.size() <= 100
    std::stringstream idList;
    for (auto id : ids) {
        if (idList.tellp() > 0) {
            idList << ";";
        }
        idList << id;
    }
    
    auto titleResponse = api.get<stackapi::Post>(
        "posts/" + idList.str(),
        {},
        {
            .site{apiSite},
            .filter{"!nNPvSNOTRz"},
            .pageSize = 100,
        }
    );
    std::unique_lock<std::shared_mutex> l(m);

    for (auto& post : titleResponse.items) {
        std::pair<std::string, long long> k = {
            apiSite, post.post_id
        };
        posts[k] = {
            htmlDecode(post.title),
            Clock::now()
        };
    }

    // If there's more than 2048 entries, trim any old entries
    if (posts.size() >= 2048) {
        spdlog::debug("posts.size() exceeds 2048; trimming.");
        purgePosts();
    }
}

/**
 * Extracts the question ID from the link to save a few API calls
 */
inline long long getQuestionId(const std::string& link) { 
    // Cut off the protocol
    auto noProto = link.substr(link.find('.'));
    // Cut off the domain
    auto path = noProto.substr(noProto.find('/'));
    // Jump to the second slash
    auto questionLoc = path.substr(path.find('/', 1));
    // Extract the question ID
    auto idStr = questionLoc.substr(1, questionLoc.find('/', 1));

    // Convert to long
    // The nice part is that this does not care about the trailing slash the last substr
    // introduces.
    // Could I strip it? Yes. Do I need to? No :p
    return std::stoll(idStr);
}

}
