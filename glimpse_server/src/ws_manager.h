#pragma once
#include <uwebsockets/WebSocket.h>

#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <variant>

#include "controller.h"
#include "user.h"

namespace glimpse {
using WsSession = uWS::WebSocket<false, true, glimpse::User>;
using WsPayload = std::variant<std::string>;

constexpr uint32_t WS_MAX_PAYLOAD_LENGTH = 16 * 1024;       // kB
constexpr uint32_t WS_IDLE_TIMEOUT = 10;                    // second
constexpr uint32_t WS_MAX_BACK_PRESSURE = 1 * 1024 * 1024;  // kB

struct WsMessage {
  enum Type : int {
    PING,
    PONG,
    ERROR,
    JOIN_ROOM,
  };

  Type type;
  WsPayload payload;
};

class WsManager {
 public:
  void handleWsOpen(WsSession* ws);
  void handleWsClose(WsSession* ws, int code, std::string_view message);
  void handleWsMessage(WsSession* ws, std::string_view message,
                       uWS::OpCode opCode);
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
    switch (j.at("type").get<glimpse::WsMessage::Type>()) {
      default: {
        msg.type = j.at("type").get<glimpse::WsMessage::Type>();
        msg.payload = j.at("payload").get<std::string>();
      }
    }
  }
};

NLOHMANN_JSON_NAMESPACE_END
