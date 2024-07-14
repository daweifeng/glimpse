#pragma once

#include <libusockets.h>
#include <uwebsockets/App.h>

#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <string_view>

#include "room_manager.h"

namespace glimpse {

struct CreateNewRoomRequestPayload {
  std::string userId;
  std::string username;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(CreateNewRoomRequestPayload, userId, username);
};

struct CreateNewRoomResponsePayload {
  std::string roomId;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(CreateNewRoomResponsePayload, roomId);
};

struct JoinRoomRequestPayload {
  std::string userId;
  std::string username;
  std::string roomId;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(JoinRoomRequestPayload, userId, username,
                                 roomId);
};

struct JoinRoomResponsePayload {
  std::string requestId;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(JoinRoomResponsePayload, requestId);
};

struct ApproveJoinRoomRequestPayload {
  std::string userId;
  std::string requestId;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(ApproveJoinRoomRequestPayload, userId,
                                 requestId);
};

struct DenyJoinRoomRequestPayload {
  std::string userId;
  std::string requestId;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(DenyJoinRoomRequestPayload, userId, requestId);
};

constexpr std::string_view ALLOWED_ORIGIN = "*";

constexpr std::string_view HTTP_STATUS_200 = "200 Ok";
constexpr std::string_view HTTP_STATUS_400 = "400 Bad Request";

class Controller {
 protected:
  void handlePost(
      uWS::HttpResponse<false> *res, uWS::HttpRequest *req,
      std::function<void(uWS::HttpResponse<false> *res, uWS::HttpRequest *req,
                         std::shared_ptr<std::string> body)>
          bodyHandler);
};

class RootController : Controller {
 public:
  void handleGet(uWS::HttpResponse<false> *res, uWS::HttpRequest *req);
  void handleOption(uWS::HttpResponse<false> *res, uWS::HttpRequest *req);
};

class RoomController : Controller {
 public:
  RoomController(std::shared_ptr<RoomManager> roomManager);
  void handleCreateNewRoomPost(uWS::HttpResponse<false> *res,
                               uWS::HttpRequest *req);
  void handleJoinRoomPost(uWS::HttpResponse<false> *res, uWS::HttpRequest *req);
  void handleApproveJoinRoomPost(uWS::HttpResponse<false> *res,
                                 uWS::HttpRequest *req);
  void handleDenyJoinRoomPost(uWS::HttpResponse<false> *res,
                              uWS::HttpRequest *req);

 private:
  std::shared_ptr<RoomManager> roomManager_;
};

class WsController : Controller {
 public:
  void handleWsRouteUpgrade(uWS::HttpResponse<false> *res,
                            uWS::HttpRequest *req,
                            us_socket_context_t *context);
};
};  // namespace glimpse
