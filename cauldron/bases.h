#pragma once

#include <functional>
#include <memory>
#include "sieve.h"
#include "facility.h"


namespace strategies {
template<typename T>
class Filtered;


template<typename T>
class Strategy {
 public:
  virtual T operator()() const = 0;

  virtual std::unique_ptr<Filtered<T>> filter(
      const Requirement<T> &requirement
  ) const {
    auto sieve = Sieve<T>().expand(requirement);
    return std::make_unique<Filtered<T>>(sieve,
                                         std::move(clone()));
  }

  virtual std::unique_ptr<Strategy<T>> clone() const = 0;
};


template<typename T, class Derived>
class CloneHelper : public Strategy<T> {
 public:
  std::unique_ptr<Strategy<T>> clone() const override {
    return std::make_unique<Derived>(static_cast<const Derived &>(*this));
  }
};


template<typename T>
class Filtered : public CloneHelper<T, Filtered<T>> {
 public:
  explicit Filtered(const Sieve<T> &sieve,
                    std::shared_ptr<Strategy<T>> strategy) :
      sieve_(sieve),
      strategy_(std::move(strategy)) {};

  std::unique_ptr<Filtered<T>> filter(
      const Requirement<T> &requirement
  ) const override {
    auto sieve = sieve_.expand(requirement);
    return std::make_unique<Filtered<T>>(sieve,
                                         strategy_);
  }

  T operator()() const override {
    std::function<T()> producer([&]() -> T {
      return (*strategy_)();
    });
    return sieve_.sift(producer);
  }

 protected:
  Sieve<T> sieve_;
  std::shared_ptr<Strategy<T>> strategy_;
};
}
