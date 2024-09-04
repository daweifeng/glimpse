#pragma once

#include <libusockets.h>
#include <uwebsockets/App.h>

#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <string_view>

#include "room_manager.h"

namespace glimpse {

struct ErrorResponsePayload {
  std::string message;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(ErrorResponsePayload, message);
};

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

struct EndRoomRequestPayload {
  std::string userId;
  std::string roomId;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(EndRoomRequestPayload, userId, roomId);
};

struct SDPExchangePayload {
  std::string userId;
  std::string roomId;
  std::string sdp;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SDPExchangePayload, userId, roomId, sdp);
};

struct ICEExchangePayload {
  std::string userId;
  std::string roomId;
  std::string ice;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(ICEExchangePayload, userId, roomId, ice);
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

  void respondError(uWS::HttpResponse<false> *res,
                    const std::string &errorMessage);
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
  void handleSDPPost(uWS::HttpResponse<false> *res, uWS::HttpRequest *req);
  void handleICEPost(uWS::HttpResponse<false> *res, uWS::HttpRequest *req);
  void handleEndRoomPost(uWS::HttpResponse<false> *res, uWS::HttpRequest *req);

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
