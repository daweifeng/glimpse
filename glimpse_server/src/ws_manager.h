#pragma once
#include <uwebsockets/WebSocket.h>

#include <cstdint>
#include <mutex>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <variant>

#include "user.h"

namespace glimpse {
using WsSession = uWS::WebSocket<false, true, glimpse::User>;

constexpr uint32_t WS_MAX_PAYLOAD_LENGTH = 16 * 1024;       // kB
constexpr uint32_t WS_IDLE_TIMEOUT = 10;                    // second
constexpr uint32_t WS_MAX_BACK_PRESSURE = 1 * 1024 * 1024;  // kB

struct WsJoinRoomResultPayload {
  std::string requestId;
  std::string roomId;
  bool approved;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WsJoinRoomResultPayload, requestId, roomId,
                                 approved);
};

struct WsJoinRoomRequestPayload {
  std::string requestId;
  std::string roomId;
  std::string userId;
  std::string username;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WsJoinRoomRequestPayload, requestId, roomId,
                                 username, userId);
};

struct WsRoomReadyPayload {
  std::string roomId;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WsRoomReadyPayload, roomId);
};

using WsPayload = std::variant<std::string, WsJoinRoomResultPayload,
                               WsJoinRoomRequestPayload, WsRoomReadyPayload>;

struct WsMessage {
  enum Type : int {
    PING,
    PONG,
    ERROR,
    REQUEST_JOIN_ROOM,
    ALLOW_JOIN_ROOM,
    DENY_JOIN_ROOM,
    ROOM_READY,
  };

  Type type;
  WsPayload payload;
};

class WsManagerError : public std::exception {
 public:
  WsManagerError(const char* message) : msg_(message) {}

  virtual const char* what() const noexcept override { return msg_; }

 private:
  const char* msg_;
};

class WsManager {
 public:
  void handleWsOpen(WsSession* ws);
  void handleWsClose(WsSession* ws, int code, std::string_view message);
  void handleWsMessage(WsSession* ws, std::string_view message,
                       uWS::OpCode opCode);

  void sendMessage(const std::string& userId, const WsMessage& message);

 private:
  void sendWsMessage(WsSession* ws, const WsMessage& message);

 private:
  std::mutex sessionsMutex_;
  std::unordered_map<std::string, WsSession*> wsSessions_;
};
}  // namespace glimpse

NLOHMANN_JSON_NAMESPACE_BEGIN

template <>
struct adl_serializer<glimpse::WsMessage> {
  static void to_json(json& j, const glimpse::WsMessage& msg) {
    j["type"] = msg.type;
    std::visit([&j](const auto& v) { j["payload"] = v; }, msg.payload);
  }

  static void from_json(const json& j, glimpse::WsMessage& msg) {
    msg.type = j.at("type").get<glimpse::WsMessage::Type>();
    switch (j.at("type").get<glimpse::WsMessage::Type>()) {
      case glimpse::WsMessage::Type::REQUEST_JOIN_ROOM: {
        msg.payload = j.at("payload").get<glimpse::WsJoinRoomResultPayload>();
        break;
      }
      case glimpse::WsMessage::Type::ALLOW_JOIN_ROOM:
      case glimpse::WsMessage::Type::DENY_JOIN_ROOM: {
        msg.payload = j.at("payload").get<glimpse::WsJoinRoomResultPayload>();
        break;
      }

      case glimpse::WsMessage::Type::ROOM_READY: {
        msg.payload = j.at("payload").get<glimpse::WsRoomReadyPayload>();
        break;
      }

      default: {
        msg.payload = j.at("payload").get<std::string>();
      }
    }
  }
};

NLOHMANN_JSON_NAMESPACE_END
