#pragma once

// overloaded

#include "tdclient.h"
#include <iostream>
#include <sstream>

namespace detail {
    template <class... Fs>
    struct overload;

    template <class F>
    struct overload<F> : public F {
        explicit overload(F f) : F(f) {
        }
    };
    template <class F, class... Fs>
    struct overload<F, Fs...>
            : public overload<F>
                    , public overload<Fs...> {
        overload(F f, Fs... fs) : overload<F>(f), overload<Fs...>(fs...) {
        }
        using overload<F>::operator();
        using overload<Fs...>::operator();
    };
}  // namespace detail

template <class... F>
auto overloaded(F... f) {
    return detail::overload<F...>(f...);
}

const std::string USERNAME = "tg_cloudfilesbot";

void TdClass::Loop() {

        /// TODO: redo ts

        while (true) {
            if (need_restart_) {
                Restart();
            } else if (!are_authorized_) {
                ProcessResponse(client_manager_->receive(10));
            } else {
                std::cout << "Enter action [q] quit [u] check for updates and request results [c] show chats [m <chat_id> "
                             "<text>] send message [me] show self [l] logout: "
                          << std::endl;
                std::string line;
                std::getline(std::cin, line);
                std::istringstream ss(line);
                std::string action;
                if (!(ss >> action)) {
                    continue;
                }
                if (action == "q") {
                    return;
                }
                if (action == "u") {
                    std::cout << "Checking for updates..." << std::endl;
                    while (true) {
                        auto response = client_manager_->receive(0);
                        if (response.object) {
                            ProcessResponse(std::move(response));
                        } else {
                            break;
                        }
                    }
                } else if (action == "close") {
                    std::cout << "Closing..." << std::endl;
                    SendQuery(td_api::make_object<td_api::close>(), {});
                } else if (action == "me") {
                    SendQuery(td_api::make_object<td_api::getMe>(),
                               [this](Object object) { std::cout << to_string(object) << std::endl; });
                } else if (action == "l") {
                    std::cout << "Logging out..." << std::endl;
                    SendQuery(td_api::make_object<td_api::logOut>(), {});
                } else if (action == "m") {
                    std::int64_t chat_id;
                    ss >> chat_id;
                    ss.get();
                    std::string text;
                    std::getline(ss, text);

                    std::cout << "Sending message to chat " << chat_id << "..." << std::endl;
                    auto send_message = td_api::make_object<td_api::sendMessage>();
                    send_message->chat_id_ = chat_id;
                    auto message_content = td_api::make_object<td_api::inputMessageText>();
                    message_content->text_ = td_api::make_object<td_api::formattedText>();
                    message_content->text_->text_ = std::move(text);
                    send_message->input_message_content_ = std::move(message_content);

                    SendQuery(std::move(send_message), {});
                } else if (action == "send") {
                    SendFile(GetChatId(USERNAME), "");
                } else if (action == "reset") {
                    /// TODO: init a special chat for a file upload or find if there's an existing one

                    auto deleter = td_api::make_object<td_api::deleteChatHistory>();
                    deleter->chat_id_ = GetChatId(USERNAME);
                    SendQuery(std::move(deleter), {});
                } else if (action == "something") {
                    /// TODO: Make a easy file upload using chat above
                    GetLastMessage(GetChatId(USERNAME));
                }
            }
        }
}

td::tl_object_ptr<td_api::message> TdClass::GetLastMessage(const td_api::int53 chat_id) {
    auto history = td_api::make_object<td_api::getChatHistory>();
    history->chat_id_ = chat_id;
    history->limit_ = 10;
    history->only_local_ = false;
    td::tl_object_ptr<td_api::message> last_message = nullptr;
    SendQuery(std::move(history), [this, &last_message](Object object) {
                    if (object->get_id() == td_api::error::ID) {
                        std::cerr << to_string(object);
                        return;
                    }
                    auto mes = td::move_tl_object_as<td_api::messages>(object);
                    if (!mes->total_count_) {
                        std::cerr << "Couldn't find any messages in file chat.\n";
                        return;
                    }
                    last_message = std::move(mes->messages_[0]);
                });
    while (!last_message) {
    }
    return last_message;
}

void TdClass::SendFile(td_api::int53 chat_id, const std::string &path) {
    auto send_message = td_api::make_object<td_api::sendMessage>();
    send_message->chat_id_ = chat_id;
    auto local_file = td_api::make_object<td_api::inputFileLocal>();
    local_file->path_ = path;
    auto message_file = td_api::make_object<td_api::inputMessageDocument>();
    message_file->document_ = std::move(local_file);
    send_message->input_message_content_ = std::move(message_file);
    SendQuery(std::move(send_message), [this](Object object) {
        if (object->get_id() == td_api::error::ID) {
            std::cerr << "Problem while sending file\n";
            return;
        }
        std::cerr << "File was successfully sent\n";
    });
}

td_api::int53 TdClass::GetChatId(const std::string& username) {
    auto find_chat = td_api::make_object<td_api::searchPublicChat>();
    find_chat->username_ = username;
    td_api::int53 id = 0;
    SendQuery(std::move(find_chat), [this, &id](Object object) {

        if (object->get_id() == td_api::error::ID) {
            std::cerr << "Could not find the chat\n";
            id = -1;
            return;
        }
        auto chats = td::move_tl_object_as<td_api::chat>(object);
        std::cerr << "Chat was found/created"  << std::endl;
        id = chats->id_;
    });
    while (id == 0) {
    }
    return id;
}

void TdClass::Restart() {
    client_manager_.reset();
    *this = TdClass(api_id_, api_hash_);
}

void TdClass::SendQuery(td_api::object_ptr<td_api::Function> f, std::function<void(Object)> handler) {
    auto query_id = NextQueryId();
    if (handler) {
        handlers_.emplace(query_id, std::move(handler));
    }
    client_manager_->send(client_id_, query_id, std::move(f));
}

void TdClass::ProcessResponse(td::ClientManager::Response response) {
    if (!response.object) {
        return;
    }
    // std::cout << response.request_id << " " << to_string(response.object) << std::endl;
    if (response.request_id == 0) {
        return ProcessUpdate(std::move(response.object));
    }
    auto it = handlers_.find(response.request_id);
    if (it != handlers_.end()) {
        it->second(std::move(response.object));
        handlers_.erase(it);
    }
}

void TdClass::ProcessUpdate(td_api::object_ptr<td_api::Object> update) {
    td_api::downcast_call(
                *update, overloaded(
                        [this](td_api::updateAuthorizationState &update_authorization_state) {
                            authorization_state_ = std::move(update_authorization_state.authorization_state_);
                            OnAuthorizationStateUpdate();
                        },
                        [](auto &update) {}));
}

auto TdClass::CreateAuthenticationQueryHandler() {
    return [this, id = authentication_query_id_](Object object) {
    if (id == authentication_query_id_) {
        CheckAuthenticationError(std::move(object));
    }
};
}

void TdClass::OnAuthorizationStateUpdate() {
    authentication_query_id_++;
        td_api::downcast_call(*authorization_state_,
                              overloaded(
                                      [this](td_api::authorizationStateReady &) {
                                          are_authorized_ = true;
                                          std::cout << "Authorization is completed" << std::endl;
                                      },
                                      [this](td_api::authorizationStateLoggingOut &) {
                                          are_authorized_ = false;
                                          std::cout << "Logging out" << std::endl;
                                      },
                                      [this](td_api::authorizationStateClosing &) { std::cout << "Closing" << std::endl; },
                                      [this](td_api::authorizationStateClosed &) {
                                          are_authorized_ = false;
                                          need_restart_ = true;
                                          std::cout << "Terminated" << std::endl;
                                      },
                                      [this](td_api::authorizationStateWaitPhoneNumber &) {
                                          std::cout << "Enter phone number: " << std::flush;
                                          std::string phone_number;
                                          std::cin >> phone_number;
                                          SendQuery(
                                                  td_api::make_object<td_api::setAuthenticationPhoneNumber>(phone_number, nullptr),
                                                  CreateAuthenticationQueryHandler());
                                      },
                                      [this](td_api::authorizationStateWaitEmailAddress &) {
                                          std::cout << "Enter email address: " << std::flush;
                                          std::string email_address;
                                          std::cin >> email_address;
                                          SendQuery(td_api::make_object<td_api::setAuthenticationEmailAddress>(email_address),
                                                     CreateAuthenticationQueryHandler());
                                      },
                                      [this](td_api::authorizationStateWaitEmailCode &) {
                                          std::cout << "Enter email authentication code: " << std::flush;
                                          std::string code;
                                          std::cin >> code;
                                          SendQuery(td_api::make_object<td_api::checkAuthenticationEmailCode>(
                                                             td_api::make_object<td_api::emailAddressAuthenticationCode>(code)),
                                                     CreateAuthenticationQueryHandler());
                                      },
                                      [this](td_api::authorizationStateWaitCode &) {
                                          std::cout << "Enter authentication code: " << std::flush;
                                          std::string code;
                                          std::cin >> code;
                                          SendQuery(td_api::make_object<td_api::checkAuthenticationCode>(code),
                                                     CreateAuthenticationQueryHandler());
                                      },
                                      [this](td_api::authorizationStateWaitRegistration &) {
                                          std::string first_name;
                                          std::string last_name;
                                          std::cout << "Enter your first name: " << std::flush;
                                          std::cin >> first_name;
                                          std::cout << "Enter your last name: " << std::flush;
                                          std::cin >> last_name;
                                          SendQuery(td_api::make_object<td_api::registerUser>(first_name, last_name, false),
                                                     CreateAuthenticationQueryHandler());
                                      },
                                      [this](td_api::authorizationStateWaitPassword &) {
                                          std::cout << "Enter authentication password: " << std::flush;
                                          std::string password;
                                          std::getline(std::cin, password);
                                          SendQuery(td_api::make_object<td_api::checkAuthenticationPassword>(password),
                                                     CreateAuthenticationQueryHandler());
                                      },
                                      [this](td_api::authorizationStateWaitOtherDeviceConfirmation &state) {
                                          std::cout << "Confirm this login link on another device: " << state.link_ << std::endl;
                                      },
                                      [this](td_api::authorizationStateWaitTdlibParameters &) {
                                          auto request = td_api::make_object<td_api::setTdlibParameters>();
                                          request->database_directory_ = "tdlib";
                                          request->use_message_database_ = true;
                                          request->use_secret_chats_ = true;
                                          request->api_id_ = api_id_;
                                          request->api_hash_ = api_hash_;
                                          request->system_language_code_ = "en";
                                          request->device_model_ = "Desktop";
                                          request->application_version_ = "1.0";
                                          //request->use_test_dc_ = true;
                                          SendQuery(std::move(request), CreateAuthenticationQueryHandler());
                                      }));
}

void TdClass::CheckAuthenticationError(Object object) {
    if (object->get_id() == td_api::error::ID) {
        auto error = td::move_tl_object_as<td_api::error>(object);
        std::cout << "Error: " << to_string(error) << std::flush;
        OnAuthorizationStateUpdate();
    }
}

std::uint64_t TdClass::NextQueryId() {
    return ++current_query_id_;
}
