#pragma once

#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>



namespace td_api = td::td_api;

class TdClass {
public:

    TdClass(std::int32_t api_id, std::string api_hash) : api_id_(api_id), api_hash_(std::move(api_hash)) {
        td::ClientManager::execute(td_api::make_object<td_api::setLogVerbosityLevel>(1));
        client_manager_ = std::make_unique<td::ClientManager>();
        client_id_ = client_manager_->create_client_id();
        SendQuery(td_api::make_object<td_api::getOption>("version"), {});
    }

    void Loop();

    td::tl_object_ptr<td_api::message> GetLastMessage(td_api::int53 chat_id);

    void DownloadFile(); // Implement

    void SendFile(td_api::int53 chat_id, const std::string& path);

    /* Method to get chat id*/

    td_api::int53 GetChatId(const std::string& username);



private:
    using Object = td_api::object_ptr<td_api::Object>;
    std::unique_ptr<td::ClientManager> client_manager_;
    std::int32_t client_id_{0};

    td_api::object_ptr<td_api::AuthorizationState> authorization_state_;
    bool are_authorized_{false};
    bool need_restart_{false};
    std::int32_t api_id_{0};
    std::string api_hash_;
    std::uint64_t current_query_id_{0};
    std::uint64_t authentication_query_id_{0};

    std::map<std::uint64_t, std::function<void(Object)>> handlers_;

    void Restart();

    void SendQuery(td_api::object_ptr<td_api::Function> f, std::function<void(Object)> handler);

    void ProcessResponse(td::ClientManager::Response response);

    void ProcessUpdate(td_api::object_ptr<td_api::Object> update);

    auto CreateAuthenticationQueryHandler();

    void OnAuthorizationStateUpdate();

    void CheckAuthenticationError(Object object);

    std::uint64_t NextQueryId();
};