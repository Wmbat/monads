#include <libreglisse/either.hpp>

#include <catch2/catch.hpp>

#include <vector>

using namespace reglisse;

TEST_CASE("left - is left<T> trivial", "[either][left]")
{
   CHECK(std::is_trivially_copy_assignable_v<right<int>>);
   CHECK(std::is_trivially_copy_constructible_v<right<int>>);
   CHECK(std::is_trivially_move_assignable_v<right<int>>);
   CHECK(std::is_trivially_move_constructible_v<right<int>>);
   CHECK_FALSE(std::is_trivial_v<right<int>>); // No default ctor

   using vec_t = right<std::vector<int>>;
   CHECK_FALSE(std::is_trivially_copy_assignable_v<vec_t>);
   CHECK_FALSE(std::is_trivially_copy_constructible_v<vec_t>);
   CHECK_FALSE(std::is_trivially_move_assignable_v<vec_t>);
   CHECK_FALSE(std::is_trivially_move_constructible_v<vec_t>);
   CHECK_FALSE(std::is_trivial_v<vec_t>);
}

TEST_CASE("left - constructor", "[either][left]")
{
   const auto left_int = left<int>(1);
   const auto left_float = left<float>(1.0f);
   const auto left_vector = left<std::vector<int>>({1, 1, 1});

   CHECK(left_int.borrow() == 1);
   CHECK(left_float.borrow() == 1.0f);
   CHECK(std::size(left_vector.borrow()) == 3);
}

TEST_CASE("left - value()", "[either][left]")
{
   SECTION("const&")
   {
      const auto left_int = left<int>(1);
      const auto left_float = left<float>(1.0f);
      const auto left_vector = left<std::vector<int>>({1, 1, 1});

      CHECK(left_int.borrow() == 1);
      CHECK(left_float.borrow() == 1.0f);
      CHECK(std::size(left_vector.borrow()) == 3);
   }
   SECTION("&")
   {
      auto left_int = left<int>(1);
      auto left_float = left<float>(1.0f);
      auto left_vector = left<std::vector<int>>({1, 1, 1});

      CHECK(left_int.borrow() == 1);
      CHECK(left_float.borrow() == 1.0f);
      CHECK(std::size(left_vector.borrow()) == 3);
   }
   SECTION("&&")
   {
      CHECK(left<int>(1).take() == 1);
      CHECK(left<float>(1.0f).take() == 1.0f);
      CHECK(left<std::vector<int>>({1, 1, 1}).take() == std::vector<int>({1, 1, 1}));
   }
}

TEST_CASE("left - comparison")
{
   SECTION("left<T> comparison with left<T>")
   {
      const auto left_int = left<int>(1);
      const auto left_float = left<float>(1.0f);
      const auto left_vector = left<std::vector<int>>({1, 1, 1});

      CHECK(left<int>(0) != left_int);
      CHECK(left<float>(0.0f) != left_float);
      CHECK(left<std::vector<int>>({1, 1, 0}) != left_vector);

      CHECK(left<int>(1) == left_int);
      CHECK(left<float>(1.0f) == left_float);
      CHECK(left<std::vector<int>>({1, 1, 1}) == left_vector);
   }
   SECTION("left<T> comparison with right<T>")
   {
      const auto left_int = left<int>(1);
      const auto left_float = left<float>(1.0f);
      const auto left_vector = left<std::vector<int>>({1, 1, 1});

      CHECK(right<int>(0) != left_int);
      CHECK(right<float>(0.0f) != left_float);
      CHECK(right<std::vector<int>>({1, 1, 0}) != left_vector);

      CHECK(right<int>(1) == left_int);
      CHECK(right<float>(1.0f) == left_float);
      CHECK(right<std::vector<int>>({1, 1, 1}) == left_vector);
   }
}
