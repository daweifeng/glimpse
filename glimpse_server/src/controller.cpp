#include "controller.h"

#include <spdlog/spdlog.h>

#include <charconv>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <string_view>

#include "user.h"

namespace glimpse {

void RootController::handleGet(uWS::HttpResponse<false> *res,
                               uWS::HttpRequest *) {
  res->end("Hey, this is Glimpse Server!");
};

void RootController::handleOption(uWS::HttpResponse<false> *res,
                                  uWS::HttpRequest *) {
  res->writeHeader("Access-Control-Allow-Origin", ALLOWED_ORIGIN)
      ->writeHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS")
      ->writeHeader("Access-Control-Allow-Headers", "Content-Type")
      ->end();
}

RoomController::RoomController(std::shared_ptr<RoomManager> roomManager)
    : roomManager_(roomManager) {}

void RoomController::handleCreateNewRoomPost(uWS::HttpResponse<false> *res,
                                             uWS::HttpRequest *req) {
  /* Display the headers */
  uint32_t contentLength = 0;
  auto lengthStr = req->getHeader("content-length");
  auto result = std::from_chars(
      lengthStr.data(), lengthStr.data() + lengthStr.size(), contentLength);
  if (result.ec == std::errc::invalid_argument) {
    spdlog::error("Could not convert content length to int");
  }
  auto isAborted = std::make_shared<bool>(false);
  auto body = std::make_shared<std::string>();
  body->reserve(contentLength);
  res->onAborted([isAborted]() { *isAborted = true; });
  res->onData(
      [res, body, isAborted, this](std::string_view chunk, bool isLast) {
        body->append(chunk);
        if (isLast and not *isAborted) {
          try {
            auto j = nlohmann::json::parse(*body);
            auto payload = j.template get<CreateNewRoomRequestPayload>();
            auto roomId = roomManager_->createNewRoom(
                {.id = payload.userId, .name = payload.username});
            CreateNewRoomResponsePayload response = {roomId};
            nlohmann::json responseJ = response;

            res->writeHeader("Access-Control-Allow-Origin", ALLOWED_ORIGIN)
                ->end(responseJ.dump());
          } catch (const nlohmann::json::exception &e) {
            auto errMsg = fmt::format("Invalid payload: {}", e.what());
            spdlog::error(errMsg);
            res->cork([res, errMsg]() {
              res->writeStatus(HTTP_STATUS_400)->end(errMsg);
            });
          }
        }
      });
};

void WsController::handleWsRouteUpgrade(uWS::HttpResponse<false> *res,
                                        uWS::HttpRequest *req,
                                        us_socket_context_t *context) {
  auto userId = std::string(req->getQuery("userId"));
  auto userName = std::string(req->getQuery("username"));

  if (userId.empty() or userName.empty()) {
    spdlog::error("Missing queries");
    res->writeStatus(HTTP_STATUS_400)->end("Missing queries");
    return;
  }

  res->template upgrade<User>(
      {.id = userId, .name = userName}, req->getHeader("sec-websocket-key"),
      req->getHeader("sec-websocket-protocol"),
      req->getHeader("sec-websocket-extensions"), context);
};
}  // namespace glimpse
