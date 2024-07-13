#include "room_manager.h"

namespace glimpse {
std::string RoomManager::createNewRoom(const User& user) {
  auto id = boost::uuids::to_string(boost::uuids::random_generator()());
  rooms_.try_emplace(id, id, user.id);
  // TODO: remove room if no one join after a while
  return id;
};

bool RoomManager::roomExists(const std::string& roomId) {
  return rooms_.contains(roomId);
}
};  // namespace glimpse
