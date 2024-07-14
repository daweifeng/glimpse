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
    throw std::runtime_error("room does not exit");
  }

  auto joinRoomRequestId =
      boost::uuids::to_string(boost::uuids::random_generator()());

  if (isRoomHost(user.id, roomId)) {
    // Host is allowed immediately
    WsJoinRoomResultPayload payload = {.roomId = roomId, .approved = true};
    wsManager_->sendMessage(
        user.id, {.type = WsMessage::ALLOW_JOIN_ROOM, .payload = payload});
  } else {
    // Guest needs host's approval
    WsJoinRoomRequestPayload payload = {
        .roomId = roomId, .userId = user.id, .username = user.name};

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
    throw std::runtime_error("request does not exist");
  }

  if (not rooms_.contains(requests_.at(requestId).roomId)) {
    throw std::runtime_error("room does not exist");
  }

  if (not isRoomHost(hostId, requests_.at(requestId).roomId)) {
    throw std::runtime_error("user is not a host");
  }

  rooms_.at(requests_.at(requestId).roomId)
      .setGuest({
          .id = requests_.at(requestId).userId,
          .name = requests_.at(requestId).username,
      });

  WsJoinRoomResultPayload payload = {.roomId = requests_.at(requestId).roomId,
                                     .approved = true};
  wsManager_->sendMessage(
      requests_.at(requestId).userId,
      {.type = WsMessage::ALLOW_JOIN_ROOM, .payload = payload});

  requests_.erase(requestId);
};

void RoomManager::denyJoinRoomRequest(const std::string& requestId,
                                      const std::string& hostId) {
  if (not requests_.contains(requestId)) {
    throw std::runtime_error("request does not exist");
  }

  if (not rooms_.contains(requests_.at(requestId).roomId)) {
    throw std::runtime_error("room does not exist");
  }

  if (not isRoomHost(hostId, requests_.at(requestId).roomId)) {
    throw std::runtime_error("user is not a host");
  }

  WsJoinRoomResultPayload payload = {.roomId = requests_.at(requestId).roomId,
                                     .approved = false};
  wsManager_->sendMessage(
      requests_.at(requestId).userId,
      {.type = WsMessage::DENY_JOIN_ROOM, .payload = payload});

  requests_.erase(requestId);
};

};  // namespace glimpse
