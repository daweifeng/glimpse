#include "room.h"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace glimpse {
Room::Room(const std::string& id, const std::string& hostId)
    : id_(id), hostId_(hostId){};

std::string Room::getId() { return id_; }
std::string Room::getHostId() { return hostId_; }

void Room::setHost(const std::string& hostId) { hostId_ = hostId; }
}  // namespace glimpse
