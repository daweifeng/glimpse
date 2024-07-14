#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "room.h"
#include "user.h"
#include "ws_manager.h"

namespace glimpse {
class RoomManager {
 public:
  RoomManager(std::shared_ptr<WsManager> wsManager);

  std::string createNewRoom(const User& user);
  bool isRoomHost(const std::string& userId, const std::string& roomId);
  bool roomExists(const std::string& roomId);
  std::string joinRoom(const User& user, const std::string& roomId);
  void approveJoinRoomRequest(const std::string& requestId,
                              const std::string& userId);
  void denyJoinRoomRequest(const std::string& requestId,
                           const std::string& userId);

 private:
  std::shared_ptr<WsManager> wsManager_;
  std::unordered_map<std::string, Room> rooms_;
  std::unordered_map<std::string, WsJoinRoomRequestPayload> requests_;
};
}  // namespace glimpse
