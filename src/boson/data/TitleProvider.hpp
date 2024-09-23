#pragma once

#include "boson/ChatProvider.hpp"
#include "stackapi/StackAPI.hpp"
#include <sstream>
#include <string>
#include <shared_mutex>
#include <map>
#include <stackapi/data/structs/posts/Post.hpp>

namespace boson::TitleProvider {

struct PostInfo {
    std::string title;
    Clock::time_point lastSeen;
};

static std::shared_mutex m;
static std::map<long long, PostInfo> posts;

/**
 * Note that a lock must be acquired prior to calling this function.
 */
inline void purgePosts() {
    std::erase_if(
        posts,
        [](const auto& entry) -> bool {
            return Clock::now() - entry.second.lastSeen > std::chrono::days(14);
        }
    );
}

inline std::string getTitle(long long postId) {
    std::shared_lock<std::shared_mutex> l(m);
    if (posts.contains(postId)) {
        return posts.at(postId).title;
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
        // Get rid of titles already stored in the index
        std::erase_if(
            ids,
            [](const auto& id) {
                bool res = posts.contains(id);
                /**
                 * This should prevent a race condition
                 */
                if (res) {
                    posts.at(id).lastSeen = Clock::now();
                }
                return res;
            }
        );
    }

    // The list may have been emptied out by the previous block
    if (ids.size() == 0) {
        return;
    }

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
        posts[post.post_id] = {
            post.title,
            Clock::now()
        };
    }

    if (posts.size() >= 2048) {
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
