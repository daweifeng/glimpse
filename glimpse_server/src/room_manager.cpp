#include "room_manager.h"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <memory>
#include <stdexcept>

#include "ws_manager.h"

namespace glimpse {
RoomManager::RoomManager(std::shared_ptr<WsManager> wsManager)
    : wsManager_(wsManager) {}

std::string RoomManager::createNewRoom(const User& user) {
  auto id = boost::uuids::to_string(boost::uuids::random_generator()());
  rooms_.try_emplace(id, id, user);
  // TODO: remove room if no one join after a while
  return id;
};

bool RoomManager::isRoomHost(const std::string& userId,
                             const std::string& roomId) {
  if (not rooms_.contains(roomId)) {
    return false;
  }

  return rooms_.at(roomId).getHostId() == userId;
};

bool RoomManager::roomExists(const std::string& roomId) {
  return rooms_.contains(roomId);
}

std::string RoomManager::joinRoom(const User& user, const std::string& roomId) {
  if (not rooms_.contains(roomId)) {
    throw RoomManagerError("room does not exit");
  }

  auto joinRoomRequestId =
      boost::uuids::to_string(boost::uuids::random_generator()());

  if (isRoomHost(user.id, roomId)) {
    // Host is allowed immediately
    WsJoinRoomResultPayload payload = {
        .requestId = joinRoomRequestId, .roomId = roomId, .approved = true};
    wsManager_->sendMessage(
        user.id, {.type = WsMessage::ALLOW_JOIN_ROOM, .payload = payload});

    // TODO: when host quit, the room is gone and guest will be disconnected
  } else {
    // Guest needs host's approval
    WsJoinRoomRequestPayload payload = {
        .requestId = joinRoomRequestId,
        .roomId = roomId,
        .userId = user.id,
        .username = user.name,
    };

    requests_.emplace(joinRoomRequestId, payload);
    wsManager_->sendMessage(
        rooms_.at(roomId).getHostId(),
        {.type = WsMessage::REQUEST_JOIN_ROOM, .payload = payload});
  }

  return joinRoomRequestId;
}

void RoomManager::approveJoinRoomRequest(const std::string& requestId,
                                         const std::string& hostId) {
  if (not requests_.contains(requestId)) {
    throw RoomManagerError("request does not exist");
  }
  auto roomId = requests_.at(requestId).roomId;
  if (not rooms_.contains(roomId)) {
    throw RoomManagerError("room does not exist");
  }

  if (not isRoomHost(hostId, roomId)) {
    throw RoomManagerError("user is not a host");
  }

  rooms_.at(roomId).setGuest({
      .id = requests_.at(requestId).userId,
      .name = requests_.at(requestId).username,
  });

  WsJoinRoomResultPayload payload = {
      .requestId = requestId, .roomId = roomId, .approved = true};
  wsManager_->sendMessage(
      requests_.at(requestId).userId,
      {.type = WsMessage::ALLOW_JOIN_ROOM, .payload = payload});

  WsRoomReadyPayload roomReadyPayload = {.roomId = roomId};

  wsManager_->sendMessage(
      requests_.at(requestId).userId,
      {.type = WsMessage::ROOM_READY, .payload = roomReadyPayload});

  wsManager_->sendMessage(
      hostId, {.type = WsMessage::ROOM_READY, .payload = roomReadyPayload});

  requests_.erase(requestId);
};

void RoomManager::denyJoinRoomRequest(const std::string& requestId,
                                      const std::string& hostId) {
  if (not requests_.contains(requestId)) {
    throw RoomManagerError("request does not exist");
  }

  auto roomId = requests_.at(requestId).roomId;

  if (not rooms_.contains(roomId)) {
    throw RoomManagerError("room does not exist");
  }

  if (not isRoomHost(hostId, roomId)) {
    throw RoomManagerError("user is not a host");
  }

  WsJoinRoomResultPayload payload = {
      .requestId = requestId, .roomId = roomId, .approved = false};
  wsManager_->sendMessage(
      requests_.at(requestId).userId,
      {.type = WsMessage::DENY_JOIN_ROOM, .payload = payload});

  requests_.erase(requestId);
};

void RoomManager::exchangeSDPMessage(const std::string& roomId,
                                     const std::string& fromUserId,
                                     const std::string& message) {
  if (not rooms_.contains(roomId)) {
    throw RoomManagerError("room does not exist");
  }

  if (fromUserId != rooms_.at(roomId).getHostId() and
      fromUserId != rooms_.at(roomId).getGuestId()) {
    throw RoomManagerError("user is not in this room");
  }

  if (fromUserId == rooms_.at(roomId).getHostId()) {
    wsManager_->sendMessage(rooms_.at(roomId).getGuestId(),
                            {.type = WsMessage::SDP, .payload = message});
  } else {
    wsManager_->sendMessage(rooms_.at(roomId).getHostId(),
                            {.type = WsMessage::SDP, .payload = message});
  }
}

void RoomManager::exchangeICEMessage(const std::string& roomId,
                                     const std::string& fromUserId,
                                     const std::string& message) {
  if (not rooms_.contains(roomId)) {
    throw RoomManagerError("room does not exist");
  }

  if (fromUserId != rooms_.at(roomId).getHostId() and
      fromUserId != rooms_.at(roomId).getGuestId()) {
    throw RoomManagerError("user is not in this room");
  }

  if (fromUserId == rooms_.at(roomId).getHostId()) {
    wsManager_->sendMessage(rooms_.at(roomId).getGuestId(),
                            {.type = WsMessage::ICE, .payload = message});
  } else {
    wsManager_->sendMessage(rooms_.at(roomId).getHostId(),
                            {.type = WsMessage::ICE, .payload = message});
  }
}

void RoomManager::endRoom(const std::string& roomId,
                          const std::string& userId) {
  if (not rooms_.contains(roomId)) {
    throw RoomManagerError("room does not exist");
  }

  if (not(rooms_.at(roomId).getHostId() == userId or
          rooms_.at(roomId).getGuestId() == userId)) {
    throw RoomManagerError("user is not a host nor a guest");
  }

  WsRoomEndPayload payload = {.roomId = roomId};

  if (wsManager_->isUserOnline(rooms_.at(roomId).getGuestId())) {
    wsManager_->sendMessage(rooms_.at(roomId).getGuestId(),
                            {.type = WsMessage::ROOM_END, .payload = payload});
  }

  if (wsManager_->isUserOnline(rooms_.at(roomId).getHostId())) {
    wsManager_->sendMessage(rooms_.at(roomId).getHostId(),
                            {.type = WsMessage::ROOM_END, .payload = payload});
  }

  rooms_.erase(roomId);
}

};  // namespace glimpse
