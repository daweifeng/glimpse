#include <string>

#include "user.h"

namespace glimpse {
class Room {
 public:
  Room(const std::string& id, const User& host);

  std::string getId();
  std::string getHostId();
  std::string getGuestId();

  void setHost(const User& user);
  void setGuest(const User& user);

 private:
  std::string id_;
  User host_;
  User guest_;
};
}  // namespace glimpse
