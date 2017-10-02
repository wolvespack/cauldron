#include "strings.h"


namespace strategies {
Strings::Strings(std::shared_ptr<Strategy<size_t>> lengths,
                 std::shared_ptr<Strategy<char>> alphabet,
                 const Sieve<std::string> &sieve) :
    lengths_(std::move(lengths)),
    alphabet_(std::move(alphabet)),
    Filtered<std::string>(sieve) {}


std::string Strings::producer() const {
  auto length = (*lengths_)();
  std::string result(length, 0);
  // FIXME: workaround using lambda to get producer from strategy
  auto characters_producer = [&]() -> char { return (*alphabet_)(); };
  std::generate_n(result.begin(),
                  length,
                  characters_producer);
  return result;
}


std::unique_ptr<Filtered<std::string>> Strings::update_sieve(
    const Sieve<std::string> &sieve
) const {
  return std::make_unique<Strings>(lengths_,
                                   alphabet_,
                                   sieve);
}
}
