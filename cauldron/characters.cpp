#include "characters.h"


namespace cauldron {
void validate_characters(const std::string &characters) {
  if (characters.length() == 0) {
    throw std::invalid_argument("Characters container should not be empty.");
  };
}


Characters::Characters(const std::string &domain) {
  validate_characters(domain);
  domain_ = domain;
}


Characters::Characters(const char domain[]) {
  const auto &domain_string = std::string(domain);
  validate_characters(domain_string);
  domain_ = domain_string;
}


char Characters::operator()() const {
  static std::random_device random_device;
  size_t max_index = domain_.length() - 1;
  std::uniform_int_distribution<size_t> distribution(0, max_index);
  size_t index = distribution(random_device);
  return domain_[index];
}
}
