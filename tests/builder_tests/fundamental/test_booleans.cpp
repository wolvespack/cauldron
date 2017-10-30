#include <catch.hpp>
#include "../../../cauldron/booleans.h"
#include "../../../cauldron/builder.h"
#include "../wrapper.h"


TEST_CASE("booleans \"Builder\" strategy", "[Builder]") {
  using BooleanWrapper = Wrapper<bool>;

  cauldron::Requirement<BooleanWrapper> is_false_wrapper(
      [&](BooleanWrapper wrapper) -> bool {
        return !wrapper.field();
      });
  cauldron::Requirement<BooleanWrapper> is_true_wrapper(
      [&](BooleanWrapper wrapper) -> bool {
        return wrapper.field();
      });

  SECTION("single element domain") {
    auto true_values = std::make_shared<cauldron::Booleans>(1.);
    auto false_values = std::make_shared<cauldron::Booleans>(0.);
    cauldron::Builder<BooleanWrapper, bool> true_wrappers(true_values);
    cauldron::Builder<BooleanWrapper, bool> false_wrappers(false_values);

    auto true_set = true_wrappers();
    auto false_set = false_wrappers();

    REQUIRE(is_true_wrapper(true_set));
    REQUIRE(is_false_wrapper(false_set));
  }

  SECTION("filtration") {
    auto booleans = std::make_shared<cauldron::Booleans>();
    cauldron::Builder<BooleanWrapper, bool> booleans_wrappers(booleans);

    SECTION("truthfulness") {
      auto true_wrappers = booleans_wrappers.filter(is_true_wrapper);
      auto false_wrappers = booleans_wrappers.filter(is_false_wrapper);

      auto true_wrapper = (*true_wrappers)();
      auto false_wrapper = (*false_wrappers)();

      REQUIRE(is_true_wrapper(true_wrapper));
      REQUIRE(is_false_wrapper(false_wrapper));
    }

    SECTION("impossible") {
      auto invalid_wrappers =
          booleans_wrappers.filter(is_true_wrapper)->filter(is_false_wrapper);
      REQUIRE_THROWS_AS((*invalid_wrappers)(),
                        cauldron::OutOfCycles);
    }
  }

  SECTION("mapping") {
    cauldron::Converter<BooleanWrapper> to_false_wrapper(
        [&](const BooleanWrapper &wrapper) -> BooleanWrapper {
          return BooleanWrapper(false);
        });
    cauldron::Converter<BooleanWrapper> to_true_wrapper(
        [&](const BooleanWrapper &wrapper) -> BooleanWrapper {
          return BooleanWrapper(true);
        });

    auto booleans = std::make_shared<cauldron::Booleans>();
    cauldron::Builder<BooleanWrapper, bool> booleans_wrappers(booleans);

    SECTION("truthfulness") {
      auto false_wrappers = booleans_wrappers.map(to_false_wrapper);
      auto true_wrappers = booleans_wrappers.map(to_true_wrapper);

      auto false_set = (*false_wrappers)();
      auto true_set = (*true_wrappers)();

      REQUIRE(is_false_wrapper(false_set));
      REQUIRE(is_true_wrapper(true_set));
    }

    SECTION("impossible") {
      auto invalid_false_wrappers =
          booleans_wrappers.map(to_false_wrapper)->filter(is_true_wrapper);
      auto invalid_true_wrappers =
          booleans_wrappers.map(to_false_wrapper)->filter(is_true_wrapper);
      REQUIRE_THROWS_AS((*invalid_false_wrappers)(),
                        cauldron::OutOfCycles);
      REQUIRE_THROWS_AS((*invalid_true_wrappers)(),
                        cauldron::OutOfCycles);
    }
  }
}
