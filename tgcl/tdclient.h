#pragma once

#include <../td/td/telegram/Client.h>
#include <../td/td/generate/auto/td/telegram/td_api.h>
#include <../td/td/generate/auto/td/telegram/td_api.hpp>

#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>



namespace td_api = td::td_api;

/// TODO: some testing

class TdClass {
public:

    TdClass(std::int32_t api_id, const std::string &api_hash, const std::string &database_directory) : api_id_(api_id), api_hash_(api_hash), database_directory_(database_directory) {
        td::ClientManager::execute(td_api::make_object<td_api::setLogVerbosityLevel>(0));
        client_manager_ = std::make_unique<td::ClientManager>();
        client_id_ = client_manager_->create_client_id();
        SendQuery(td_api::make_object<td_api::getOption>("version"), {});
    }

    TdClass() = default;

    /* Method to authorize into tg*/

    void Start();

    /* Temporary method used for testing features */

    void Loop();

    /* Method that tries to get last message in chat*/

    td::tl_object_ptr<td_api::message> GetLastMessage(td_api::int53 chat_id);

    /* Method that downloads a file, returns the td_api::file, which contains file path, name, etc. */

    td::tl_object_ptr<td_api::file> DownloadFile(int32_t file_id, bool wait = true);

    /* Method that sends file, located at path, in chat with chat_id*/

    td_api::int53 SendFile(td_api::int53 chat_id, const std::string& path);

    /* Method to get chat_id from personal chat with @username*/

    td_api::int53 GetChatId(const std::string& username);

    void SetMainChatId(const std::string& username);

    td_api::int53 GetMainChatId() const;

    /* Method to get information about a message */

    td::tl_object_ptr<td_api::message> GetMessage(td_api::int53 chat_id, td_api::int53 message_id);

    /* Method to pin message */

    void PinMessage(td_api::int53 chat_id, td_api::int53 message_id);

    /* Method to get pinned message */

    td::tl_object_ptr<td_api::message> GetLastPinnedMessage(td_api::int53 chat_id);

    /* Method to delete message */

    void DeleteMessage(td_api::int53 chat_id, td_api::int53 message_id);

    /* DownloadFile, knowing message ID */

    td::tl_object_ptr<td_api::file> DownloadFileFromMes(td::tl_object_ptr<td_api::message> mes);

    void SetFd(int fd_);

private:

    class FD {
        public:
            FD() : fd(-1) {}

            FD(int fd) : fd(fd) {}

            ~FD() {
                if (fd >= 0) {
                    close(fd);
                }
            }

            int fd;

    };

    using Object = td_api::object_ptr<td_api::Object>;
    std::unique_ptr<td::ClientManager> client_manager_;
    std::int32_t client_id_{0};

    td_api::object_ptr<td_api::AuthorizationState> authorization_state_;
    bool are_authorized_{false};
    bool need_restart_{false};
    std::int32_t api_id_{0};
    std::string api_hash_;
    std::string database_directory_;
    std::uint64_t current_query_id_{0};
    std::uint64_t authentication_query_id_{0};
    bool are_alive_{true};
    td_api::int53 main_chat_id_{0};

    std::unordered_map<std::uint64_t, std::function<void(Object)>> handlers_;
    std::unordered_map<std::int64_t, int> completed_uploads_;
    std::unordered_map<std::int64_t, int> completed_downloads_;
    std::unordered_map<std::int64_t, td_api::int53> sent_message_;

    FD database_fd_; // database_dir fd

    void Restart();

    void SendQuery(td_api::object_ptr<td_api::Function> f, std::function<void(Object)> handler);

    void ProcessResponse(td::ClientManager::Response response);

    void ProcessUpdate(td_api::object_ptr<td_api::Object> update);

    auto CreateAuthenticationQueryHandler();

    void OnAuthorizationStateUpdate();

    void CheckAuthenticationError(Object object);

    std::uint64_t NextQueryId();
};

TdClass create_td_client(int argc, char** argv, const char* database_dir);
