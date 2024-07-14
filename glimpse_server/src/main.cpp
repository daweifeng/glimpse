#include <spdlog/spdlog.h>
#include <uwebsockets/App.h>

#include <functional>
#include <memory>

#include "controller.h"
#include "ws_manager.h"

constexpr int PORT = 8080;

int main() {
  auto wsManager = std::make_shared<glimpse::WsManager>();
  auto roomManager = std::make_shared<glimpse::RoomManager>(wsManager);
  glimpse::RootController rootController;
  glimpse::RoomController roomController(roomManager);
  glimpse::WsController wsController;

  uWS::App()
      .get("/", std::bind(&glimpse::RootController::handleGet, rootController,
                          std::placeholders::_1, std::placeholders::_2))
      .options("/*",
               std::bind(&glimpse::RootController::handleOption, rootController,
                         std::placeholders::_1, std::placeholders::_2))
      .post("/room",
            std::bind(&glimpse::RoomController::handleCreateNewRoomPost,
                      roomController, std::placeholders::_1,
                      std::placeholders::_2))
      .post("/room/join",
            std::bind(&glimpse::RoomController::handleJoinRoomPost,
                      roomController, std::placeholders::_1,
                      std::placeholders::_2))
      .post("/room/join/approve",
            std::bind(&glimpse::RoomController::handleApproveJoinRoomPost,
                      roomController, std::placeholders::_1,
                      std::placeholders::_2))
      .post("/room/join/deny",
            std::bind(&glimpse::RoomController::handleDenyJoinRoomPost,
                      roomController, std::placeholders::_1,
                      std::placeholders::_2))
      .ws<glimpse::User>(
          "/ws",
          {.compression = uWS::SHARED_COMPRESSOR,
           .maxPayloadLength = glimpse::WS_MAX_PAYLOAD_LENGTH,
           .idleTimeout = glimpse::WS_IDLE_TIMEOUT,
           .maxBackpressure = glimpse::WS_MAX_BACK_PRESSURE,
           /* Handlers */
           .upgrade = std::bind(&glimpse::WsController::handleWsRouteUpgrade,
                                wsController, std::placeholders::_1,
                                std::placeholders::_2, std::placeholders::_3),
           .open = std::bind(&glimpse::WsManager::handleWsOpen, wsManager,
                             std::placeholders::_1),
           .message = std::bind(&glimpse::WsManager::handleWsMessage, wsManager,
                                std::placeholders::_1, std::placeholders::_2,
                                std::placeholders::_3),
           .drain =
               [](auto* /*ws*/) {
                 /* Check ws->getBufferedAmount() here */
               },
           .close = std::bind(&glimpse::WsManager::handleWsClose, wsManager,
                              std::placeholders::_1, std::placeholders::_2,
                              std::placeholders::_3)})
      .listen(PORT,
              [](auto* socket) {
                if (socket) {
                  spdlog::info("Listening on port {}", PORT);
                }
              })
      .run();
}
