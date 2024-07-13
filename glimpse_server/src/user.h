#pragma once

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace glimpse {

struct User {
  std::string id;
  std::string name;
};
}  // namespace glimpse
