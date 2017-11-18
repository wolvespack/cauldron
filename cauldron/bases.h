#pragma once

#include <memory>
#include <random>

#include "sieve.h"
#include "facility.h"


namespace cauldron {
template<typename Value>
class Union;


template<typename Value>
class Filtered;


template<typename Value>
class Mapped;


/**
 * Strategies base class.
 *
 * Hereafter **strategy** is an object
 * which encapsulates an algorithm
 * for generating specific type of values.
 * @tparam Value: type of values generated by strategy.
 */
template<typename Value>
class Strategy {
 public:
  /**
   * Generates data.
   */
  virtual Value operator()() const = 0;

  /**
   * Returns a new strategy
   * that generates values from either original strategy or provided one.
   */
  virtual Union<Value> operator||(const Strategy<Value> &strategy) const {
    return Union<Value>{*this, strategy};
  }

  /**
   * Same result as for any other strategy.
   * Basically added for possible overriding
   * with support of strategies' union associativity.
   */
  virtual Union<Value> operator||(const Union<Value> &strategy) const {
    return strategy || static_cast<const Strategy<Value> &>(*this);
  }

  /**
   * Returns a new strategy
   * that generates values from the strategy
   * which satisfy provided ``strategies::Requirement`` instance.
   *
   * Note that if the ``requirement`` is too hard to satisfy
   * this might result in failing with ``OutOfCycles``.
   */
  virtual Filtered<Value> filter(const Requirement<Value> &requirement) const {
    Sieve<Value> sieve{requirement};
    return Filtered<Value>(*this, sieve);
  }

  /**
   * Returns a new strategy
   * that generates values from the strategy
   * modified with provided ``strategies::Converter`` instance.
   */
  virtual Mapped<Value> map(const Converter<Value> &converter) const {
    Facility<Value> facility{converter};
    return Mapped<Value>(*this, facility);
  }

  /**
   * Creates a copy of the strategy instance
   * and returns pointer to it.
   */
  virtual std::unique_ptr<Strategy<Value>> clone() const = 0;
};


/**
 * Helper class for implementing ``Strategy::clone`` method
 * using
 * <a href="https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern">
 * curiously recurring template pattern
 * </a>.
 * @tparam Value: type of values generated by strategy.
 * @tparam Derived: derivative strategy class.
 */
template<typename Value, class Derived>
class CloneHelper : public Strategy<Value> {
 public:
  std::unique_ptr<Strategy<Value>> clone() const override {
    return std::make_unique<Derived>(static_cast<const Derived &>(*this));
  }
};


/**
 * Union of strategies.
 * Generates values from any one of given strategies.
 *
 * @tparam Value: type of values generated by strategy.
 */
template<class Value>
class Union : public CloneHelper<Value, Union<Value>> {
 public:
  explicit Union(const Strategy<Value> &strategy,
                 const Strategy<Value> &other_strategy) {
    strategies_.push_back(strategy.clone());
    strategies_.push_back(other_strategy.clone());
  }

  /**
   * Default copy constructor doesn't fit
   * since we're using ``std::unique_ptr`` in class member
   * which is not copyable.
   */
  Union(const Union<Value> &strategy) {
    strategies_.reserve(strategy.strategies_.size());
    for (const auto &sub_strategy: strategy.strategies_) {
      strategies_.push_back(sub_strategy->clone());
    }
  }

  /**
   * Generates value from one of ``strategies_`` elements.
   */
  Value operator()() const override {
    static std::random_device random_device;
    size_t max_index = strategies_.size() - 1;
    std::uniform_int_distribution<size_t> distribution(0, max_index);
    size_t index = distribution(random_device);
    return (*strategies_[index])();
  }

  Union<Value> operator||(const Strategy<Value> &strategy) const override {
    Union<Value> result(*this);
    result.strategies_.push_back(strategy.clone());
    return result;
  }

  Union<Value> operator||(const Union<Value> &strategy) const override {
    Union<Value> result(*this);
    for (const auto &sub_strategy: strategy.strategies_) {
      result.strategies_.push_back(sub_strategy->clone());
    }
    return result;
  }

 private:
  std::vector<std::unique_ptr<Strategy<Value>>> strategies_;
};


/**
 * Strategy which filters out values
 * that satisfies defined ``strategies::Requirement`` instances.
 *
 * Hereafter a **requirement** is an
 * <a href="https://en.wikipedia.org/wiki/Predicate_(mathematical_logic)">
 * unary predicate
 * </a>.
 * @tparam Value: type of values generated by strategy.
 */
template<typename Value>
class Filtered : public CloneHelper<Value, Filtered<Value>> {
 public:
  explicit Filtered(const Strategy<Value> &strategy,
                    const Sieve<Value> &sieve) :
      strategy_(strategy.clone()),
      sieve_(sieve) {};

  /**
   * Default copy constructor doesn't fit
   * since we're using ``std::unique_ptr`` as class member
   * which is not copyable.
   */
  Filtered(const Filtered<Value> &strategy) :
      strategy_(strategy.strategy_->clone()),
      sieve_(strategy.sieve_) {}

  Filtered<Value> filter(
      const Requirement<Value> &requirement
  ) const override {
    auto sieve = sieve_.expand(requirement);
    return Filtered<Value>(*strategy_, sieve);
  }

  /**
   * Generates value
   * that satisfies defined ``strategies::Requirement`` instances.
   * @throws strategies::OutOfCycles
   */
  Value operator()() const override {
    std::function<Value()> producer([&]() -> Value {
      return (*strategy_)();
    });
    return sieve_.sift(producer);
  }

 protected:
  std::unique_ptr<Strategy<Value>> strategy_;
  Sieve<Value> sieve_;
};


/**
 * Strategy which modifies values
 * with defined ``strategies::Converter`` instances.
 *
 * Hereafter a **converter** is an
 * <a href="https://en.wikipedia.org/wiki/Operator_(mathematics)">
 * operator
 * </a>.
 * @tparam Value: type of values generated by strategy.
 */
template<typename Value>
class Mapped : public CloneHelper<Value, Mapped<Value>> {
 public:
  explicit Mapped(const Strategy<Value> &strategy,
                  const Facility<Value> &facility) :
      strategy_(strategy.clone()),
      facility_(facility) {};

  /**
   * Default copy constructor doesn't fit
   * since we're using ``std::unique_ptr`` as class member
   * which is not copyable.
   */
  Mapped(const Mapped<Value> &strategy) :
      strategy_(strategy.strategy_->clone()),
      facility_(strategy.facility_) {}

  Mapped<Value> map(const Converter<Value> &converter) const override {
    auto facility = facility_.expand(converter);
    return Mapped(*strategy_, facility);
  }

  /**
   * Generates value
   * and modifies it with defined ``strategies::Converter`` instances.
   */
  Value operator()() const override {
    Value product = (*strategy_)();
    return facility_.convert(product);
  }

 protected:
  std::unique_ptr<Strategy<Value>> strategy_;
  Facility<Value> facility_;
};
}
