#include "controller.h"

#include <spdlog/spdlog.h>

#include <charconv>
#include <cstdint>
#include <exception>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <stdexcept>
#include <string>
#include <string_view>

#include "user.h"

namespace glimpse {

void Controller::handlePost(
    uWS::HttpResponse<false> *res, uWS::HttpRequest *req,
    std::function<void(uWS::HttpResponse<false> *res, uWS::HttpRequest *req,
                       std::shared_ptr<std::string> body)>
        bodyHandler) {
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
  res->onData([res, req, body, isAborted, bodyHandler](std::string_view chunk,
                                                       bool isLast) {
    body->append(chunk);
    if (isLast and not *isAborted) {
      bodyHandler(res, req, body);
    }
  });
}

void Controller::respondError(uWS::HttpResponse<false> *res,
                              const std::string &errorMessage) {
  ErrorResponsePayload response = {.message = errorMessage};
  nlohmann::json responseJ = response;
  res->writeStatus(HTTP_STATUS_400)
      ->writeHeader("Access-Control-Allow-Origin", ALLOWED_ORIGIN)
      ->end(responseJ.dump());
}

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
  handlePost(res, req, [this](auto *res, auto *, auto body) {
    try {
      auto j = nlohmann::json::parse(*body);
      auto payload = j.template get<CreateNewRoomRequestPayload>();
      auto roomId =
          roomManager_->createNewRoom({payload.userId, payload.username});
      CreateNewRoomResponsePayload response = {roomId};
      nlohmann::json responseJ = response;

      res->writeHeader("Access-Control-Allow-Origin", ALLOWED_ORIGIN)
          ->end(responseJ.dump());
    } catch (const nlohmann::json::exception &e) {
      auto errMsg = fmt::format("Invalid payload: {}", e.what());
      spdlog::error(errMsg);
      res->cork([this, res, errMsg]() { respondError(res, errMsg); });
    }
  });
};

void RoomController::handleJoinRoomPost(uWS::HttpResponse<false> *res,
                                        uWS::HttpRequest *req) {
  handlePost(res, req, [this](auto *res, auto *, auto body) {
    try {
      auto j = nlohmann::json::parse(*body);
      JoinRoomRequestPayload payload = j.template get<JoinRoomRequestPayload>();

      if (payload.roomId.empty() or payload.userId.empty() or
          payload.username.empty()) {
        throw std::runtime_error("empty payload fields");
      }

      // Client will receive a join room request id in the response
      // Once the join room request is approved, it will receive
      // approval event with the id in web socket
      auto requestId = roomManager_->joinRoom(
          {payload.userId, payload.username}, payload.roomId);

      JoinRoomResponsePayload response = {requestId};
      nlohmann::json responseJ = response;

      res->writeHeader("Access-Control-Allow-Origin", ALLOWED_ORIGIN)
          ->end(responseJ.dump());
    } catch (const nlohmann::json::exception &e) {
      auto errMsg = fmt::format("Invalid payload: {}", e.what());
      spdlog::error(errMsg);
      res->cork([this, res, errMsg]() { respondError(res, errMsg); });
    } catch (std::exception &err) {
      auto errMsg = fmt::format("Could not join room: {}", err.what());
      spdlog::error(errMsg);
      res->cork([this, res, errMsg]() { respondError(res, errMsg); });
    }
  });
};

void RoomController::handleApproveJoinRoomPost(uWS::HttpResponse<false> *res,
                                               uWS::HttpRequest *req) {
  handlePost(res, req, [this](auto *res, auto *, auto body) {
    try {
      auto j = nlohmann::json::parse(*body);
      auto payload = j.template get<ApproveJoinRoomRequestPayload>();

      if (payload.requestId.empty() or payload.userId.empty()) {
        throw std::runtime_error("empty payload field");
      }

      roomManager_->approveJoinRoomRequest(payload.requestId, payload.userId);

      res->writeHeader("Access-Control-Allow-Origin", ALLOWED_ORIGIN)
          ->end("{}");
    } catch (const nlohmann::json::exception &e) {
      auto errMsg = fmt::format("Invalid payload: {}", e.what());
      spdlog::error(errMsg);
      res->cork([this, res, errMsg]() { respondError(res, errMsg); });
    } catch (std::exception &err) {
      auto errMsg = fmt::format("Could not approve join room: {}", err.what());
      spdlog::error(errMsg);
      res->cork([this, res, errMsg]() { respondError(res, errMsg); });
    }
  });
}

void RoomController::handleDenyJoinRoomPost(uWS::HttpResponse<false> *res,
                                            uWS::HttpRequest *req) {
  handlePost(res, req, [this](auto *res, auto *, auto body) {
    try {
      auto j = nlohmann::json::parse(*body);
      auto payload = j.template get<DenyJoinRoomRequestPayload>();

      if (payload.requestId.empty() or payload.userId.empty()) {
        throw std::runtime_error("empty payload field");
      }

      roomManager_->denyJoinRoomRequest(payload.requestId, payload.userId);

      res->writeHeader("Access-Control-Allow-Origin", ALLOWED_ORIGIN)
          ->end("{}");
    } catch (const nlohmann::json::exception &e) {
      auto errMsg = fmt::format("Invalid payload: {}", e.what());
      spdlog::error(errMsg);
      res->cork([this, res, errMsg]() { respondError(res, errMsg); });
    } catch (std::exception &err) {
      auto errMsg = fmt::format("Could not join room: {}", err.what());
      spdlog::error(errMsg);
      res->cork([this, res, errMsg]() { respondError(res, errMsg); });
    }
  });
}

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
