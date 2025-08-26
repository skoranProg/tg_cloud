#include "tdclient.h"
#include "../parser/parser.h"

#include <cstring>
#include <iostream>
#include <sstream>
#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstdio>

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
        explicit overload(F f, Fs... fs) : overload<F>(f), overload<Fs...>(fs...) {
        }
        using overload<F>::operator();
        using overload<Fs...>::operator();
    };
}  // namespace detail

template <class... F>
auto overloaded(F... f) {
    return detail::overload<F...>(f...);
}

/* Username for file messages */

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
                    are_alive_ = false;
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


                    auto deleter = td_api::make_object<td_api::deleteChatHistory>();
                    deleter->chat_id_ = GetChatId(USERNAME);
                    SendQuery(std::move(deleter), {});
                } else if (action == "something") {

                    auto mes = GetLastMessage(GetChatId(USERNAME));

                    auto fl = td::move_tl_object_as<td_api::messageDocument>(mes->content_);
                    auto dw_f = DownloadFile(fl->document_->document_->id_);
                    std::cout << dw_f->local_->path_ << std::endl;
                }
            }
        }
}

td::tl_object_ptr<td_api::file> TdClass::DownloadFile(int32_t file_id, bool wait) {
    auto dw = td_api::make_object<td_api::downloadFile>();
    dw->file_id_ = file_id;
    dw->priority_ = 1;
    dw->synchronous_ = true;
    td::tl_object_ptr<td_api::file> result = nullptr;
    bool isDownloaded = false;
    SendQuery(std::move(dw), [this, &result, &isDownloaded](Object object) {

        if (object->get_id() == td_api::error::ID) {
            std::cerr << "Problem downloading file:\n"<<to_string(object) << std::endl;
            return;
        }
        isDownloaded = true;
        result = td::move_tl_object_as<td_api::file>(object);
        completed_downloads_[result->id_] = result->local_->is_downloading_completed_;
    });
    while (!isDownloaded) {
        ProcessResponse(client_manager_->receive(0));;
    }
    while (!completed_downloads_[file_id]) {
        ProcessResponse(client_manager_->receive(0));;
    }
    return result;
}

td::tl_object_ptr<td_api::message> TdClass::GetLastMessage(const td_api::int53 chat_id) {
    auto history = td_api::make_object<td_api::getChatHistory>();
    history->chat_id_ = chat_id;
    history->limit_ = 1;
    history->offset_ = 0;
    history->only_local_ = false;
    history->from_message_id_ = 0;
    td::tl_object_ptr<td_api::message> last_message = nullptr;
    bool wait = true;
    SendQuery(std::move(history), [this, &wait, &last_message](Object object) {
                    wait = false;
                    if (object->get_id() == td_api::error::ID) {
                        std::cerr << "Problem Getting last message in the chat:\n" << to_string(object) << std::endl;
                        return;
                    }
                    auto mes = td::move_tl_object_as<td_api::messages>(object);
                    if (!mes->total_count_) {

                        std::cerr << "Couldn't find any messages in file chat.\n";
                        return;
                    }
                    last_message = std::move(mes->messages_[0]);
    });
    while (wait) {
        ProcessResponse(client_manager_->receive(0));;
    }
    return last_message;
}

td::tl_object_ptr<td_api::file> TdClass::DownloadFileFromMes(td::tl_object_ptr<td_api::message> mes) {
    auto fl = td::move_tl_object_as<td_api::messageDocument>(mes->content_);
    return DownloadFile(fl->document_->document_->id_);
}

td_api::int53 TdClass::SendFile(td_api::int53 chat_id, const std::string &path) {
    auto send_message = td_api::make_object<td_api::sendMessage>();
    send_message->chat_id_ = chat_id;
    auto local_file = td_api::make_object<td_api::inputFileLocal>();
    local_file->path_ = path;
    auto message_file = td_api::make_object<td_api::inputMessageDocument>();
    message_file->document_ = std::move(local_file);
    send_message->input_message_content_ = std::move(message_file);
    int64_t file_id = -1;
    td::tl_object_ptr<td_api::file> result = nullptr;
    bool wait = true;
    td_api::int53 result_mes_id = -1;
    SendQuery(std::move(send_message), [this, &wait, &file_id, &result_mes_id](Object object) {
        if (object->get_id() == td_api::error::ID) {
            std::cerr << "Problem while sending file\n" << to_string(object) << std::endl;
            return;
        }
        auto mes = td::move_tl_object_as<td_api::message>(object);
        result_mes_id = mes->id_;
        auto fl = td::move_tl_object_as<td_api::messageDocument>(mes->content_);
        file_id = fl->document_->document_->id_;
        this->completed_uploads_[file_id] = fl->document_->document_->remote_->is_uploading_completed_;
        wait = false;
    });
    while (wait) {
        ProcessResponse(client_manager_->receive(0));;
    }
    if (result_mes_id == -1) {
        return result_mes_id;
    }
    while (!completed_uploads_[file_id]) {

        ProcessResponse(client_manager_->receive(0));;
    }
    while (!sent_message_[result_mes_id]) {
        ProcessResponse(client_manager_->receive(0));;
    }
    return sent_message_[result_mes_id];
}

td_api::int53 TdClass::GetChatId(const std::string& username) {
    auto find_chat = td_api::make_object<td_api::searchPublicChat>();
    find_chat->username_ = username;
    td_api::int53 id = 0;
    SendQuery(std::move(find_chat), [this, &id](Object object) {

        if (object->get_id() == td_api::error::ID) {
            std::cerr << "Problem Getting chat id:\n" << to_string(object) << std::endl;
            id = -1;
            return;
        }
        auto chats = td::move_tl_object_as<td_api::chat>(object);
        id = chats->id_;
    });
    while (id == 0) {
        ProcessResponse(client_manager_->receive(0));
    }
    return id;
}

void TdClass::DeleteMessage(td_api::int53 chat_id, td_api::int53 message_id) {
    auto del_mes = td_api::make_object<td_api::deleteMessages>();
    del_mes->chat_id_ = chat_id;
    del_mes->message_ids_ = std::vector<td_api::int53>{message_id};
    bool wait = true;
    SendQuery(std::move(del_mes), [this, &wait](Object object) {
        if (object->get_id() == td_api::error::ID) {
            std::cerr << "Problem Deleting message:\n" << to_string(object) << std::endl;
            wait = false;
            return;
        }
        wait = false;
    });
    while (wait) {
        ProcessResponse(client_manager_->receive(0));
    };
}


void TdClass::SetMainChatId(const std::string &username) {
    main_chat_id_ = GetChatId(username);
}

td_api::int53 TdClass::GetMainChatId() const {
    return main_chat_id_;
}

void TdClass::Restart()  {
    client_manager_.reset();
    *this = TdClass(api_id_, api_hash_, database_directory_);
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
                        [this](td_api::updateFile &update_file) {
                            completed_downloads_[update_file.file_->id_] = update_file.file_->local_->is_downloading_completed_;
                            completed_uploads_[update_file.file_->id_] = update_file.file_->remote_->is_uploading_completed_;

                        },
                        [this](td_api::updateMessageSendSucceeded &update_message_state) {
                            sent_message_[update_message_state.old_message_id_] = update_message_state.message_->id_;
                            sent_message_[update_message_state.message_->id_] = update_message_state.message_->id_;
                        },
                        [](auto &update) {
                        }));
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
                                          request->database_directory_ = database_directory_;
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

void TdClass::Start() {
    while (!are_authorized_) {
        ProcessResponse(client_manager_->receive(10));
    }
}

td::tl_object_ptr<td_api::message> TdClass::GetMessage(td_api::int53 chat_id, td_api::int53 message_id) {
    auto get_mes = td_api::make_object<td_api::getMessage>();
    get_mes->chat_id_ = chat_id;
    get_mes->message_id_ = message_id;
    td::tl_object_ptr<td_api::message> result = nullptr;
    bool wait = true;
    SendQuery(std::move(get_mes), [this, &result, &wait](Object object) {
        wait = false;
        if (object->get_id() == td_api::error::ID) {
            std::cerr << "Problem getting message\n";
            return;
        }
        result = td::move_tl_object_as<td_api::message>(object);
    });
    while (wait) {
        ProcessResponse(client_manager_->receive(0));
    }
    return result;
}

void TdClass::PinMessage(td_api::int53 chat_id, td_api::int53 message_id) {
    auto req = td_api::make_object<td_api::pinChatMessage>();
    req->message_id_ = message_id;
    req->chat_id_ = chat_id;
    req->disable_notification_ = false;
    req->only_for_self_ = false;
    bool wait = true;
    SendQuery(std::move(req),  [this, &wait](Object object) {
        wait = false;
        });
    while (wait) {
        ProcessResponse(client_manager_->receive(0));
    }
}

td::tl_object_ptr<td_api::message> TdClass::GetLastPinnedMessage(td_api::int53 chat_id) {
    auto get_mes = td_api::make_object<td_api::getChatPinnedMessage>();
    get_mes->chat_id_ = chat_id;
    td::tl_object_ptr<td_api::message> result = nullptr;
    bool wait = true;
    SendQuery(std::move(get_mes), [this, &result, &wait](Object object) {
        wait = false;
        if (object->get_id() == td_api::error::ID) {
            std::cerr << "Problem getting pinned message\n";
            return;
        }
        result = td::move_tl_object_as<td_api::message>(object);
    });
    while (wait) {
        ProcessResponse(client_manager_->receive(0));
    }
    return result;
}

std::string create_fd_path(const char* path) {
    int fd_dir = open(path, O_RDONLY | O_DIRECTORY);
    if (fd_dir == -1) {
        std::cerr << "Error opening directory: " << strerror(errno) << std::endl;
        return path;
    }
    pid_t pid = getpid();
    return "/proc/" + std::to_string(pid) + "/fd/" + std::to_string(fd_dir);
}

TdClass create_td_client(int argc, char** argv, const char* database_dir) {
    if (argc == 0) {
        return {};
        // Treat error
    }
    for (auto &s : stop_options) {
        if (strcmp(argv[0], s.c_str()) == 0) {
            if (strcmp(argv[0], "--help") == 0 || strcmp(argv[0], "-h") == 0 || strcmp(argv[0], "-hv") == 0) {
                std::cout << "    -api_hash [api_hash]   api hash of telegram app\n    -api_id [api_id]       api id of telegram app\n";
            }
            if (strcmp(argv[0], "--version") == 0 || strcmp(argv[0], "-v") == 0 || strcmp(argv[0], "-hv") == 0) {
                // TODO
                std::cout << "TDlib version:\n";

            }
            return {};
        }
    }
    TdClass td_client(std::stoi(argv[0]), argv[1], create_fd_path(database_dir));
    td_client.Start();
    td_client.SetMainChatId("@tg_cloudfiles1bot");
    return td_client;
}
