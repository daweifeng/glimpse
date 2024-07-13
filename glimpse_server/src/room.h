#include <string>

namespace glimpse {
class Room {
 public:
  Room(const std::string& id, const std::string& hostId);

  std::string getId();
  std::string getHostId();

  void setHost(const std::string& hostId);

 private:
  std::string id_;
  std::string hostId_;
};
}  // namespace glimpse
