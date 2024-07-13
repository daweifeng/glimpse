#include "ws_manager.h"

#include <spdlog/spdlog.h>

#include <exception>

#include "WebSocketProtocol.h"

namespace glimpse {
void WsManager::handleWsOpen(WsSession *ws) {
  spdlog::info("User {} connected to ws manager", ws->getUserData()->id);
  {
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    wsSessions_.emplace(ws->getUserData()->id, ws);
  }
};

void WsManager::handleWsClose(WsSession *ws, int code,
                              std::string_view message) {
  spdlog::info("User {} disconnected from ws manager, code: {}, msg: {}",
               ws->getUserData()->id, code, message);
  {
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    wsSessions_.erase(ws->getUserData()->id);
  }
}

void WsManager::handleWsMessage(WsSession *ws, std::string_view message,
                                uWS::OpCode) {
  try {
    auto j = nlohmann::json::parse(message);
    auto wsMessage = j.template get<WsMessage>();

    if (wsMessage.type == WsMessage::Type::PING) {
      WsMessage pongMsg = {.type = WsMessage::PONG, .payload = ""};
      sendWsMessage(ws, pongMsg);
    } else if (wsMessage.type == WsMessage::Type::PONG) {
      WsMessage pingMsg = {.type = WsMessage::PING, .payload = ""};
      sendWsMessage(ws, pingMsg);
    } else if (wsMessage.type == WsMessage::Type::JOIN_ROOM) {
      // TODO
    } else {
      spdlog::error("Received unsupported message type: {}",
                    static_cast<int>(wsMessage.type));
    }
  } catch (std::exception &err) {
    spdlog::error("Failed to handel ws message: {}", err.what());
    WsMessage errMsg = {.type = WsMessage::ERROR, .payload = "Invalid message"};
    sendWsMessage(ws, errMsg);
  }
};

void WsManager::sendWsMessage(WsSession *ws, const WsMessage &message) {
  ws->send(nlohmann::json(message).dump(), uWS::OpCode::TEXT);
}

};  // namespace glimpse
