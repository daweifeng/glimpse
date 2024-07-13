#include <string>
#include <unordered_map>

#include "room.h"
#include "user.h"

namespace glimpse {
class RoomManager {
 public:
  std::string createNewRoom(const User& user);
  bool roomExists(const std::string& roomId);

 private:
  std::unordered_map<std::string, Room> rooms_;
};
}  // namespace glimpse
