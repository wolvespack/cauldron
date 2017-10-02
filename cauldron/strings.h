#pragma once

#include <random>
#include <string>
#include <algorithm>
#include <limits>
#include <memory>
#include "bases.h"
#include "integers.h"
#include "characters.h"


namespace strategies {
class Strings : public Filtered<std::string> {
 public:
  Strings(std::shared_ptr<Strategy<size_t>> lengths,
          std::shared_ptr<Strategy<char>> alphabet,
          const Sieve<std::string> &sieve = Sieve<std::string>());

 private:
  std::shared_ptr<Strategy<size_t>> lengths_;
  std::shared_ptr<Strategy<char>> alphabet_;

  std::string producer() const override;

  std::unique_ptr<Filtered<std::string>> update_sieve(
      const Sieve<std::string> &sieve
  ) const override;
};
}
