#include "room.h"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <utility>

namespace glimpse {
Room::Room(const std::string& id, const User& host)
    : id_(id), host_(std::move(host)) {};

std::string Room::getId() { return id_; }
std::string Room::getHostId() { return host_.id; }

void Room::setHost(const User& user) { host_ = std::move(user); }
void Room::setGuest(const User& user) { guest_ = std::move(user); }
}  // namespace glimpse
