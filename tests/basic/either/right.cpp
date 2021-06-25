#include <libreglisse/either.hpp>

#include <catch2/catch.hpp>

#include <vector>

using namespace reglisse;

TEST_CASE("right - is right<T> trivial", "[either][right]")
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

TEST_CASE("right - constructor", "[either][right]")
{
   const auto right_int = right<int>(1);
   const auto right_float = right<float>(1.0f);
   const auto right_vector = right<std::vector<int>>({1, 1, 1});

   CHECK(right_int.borrow() == 1);
   CHECK(right_float.borrow() == 1.0f);
   CHECK(std::size(right_vector.borrow()) == 3);
}

TEST_CASE("right - value()", "[either][right]")
{
   SECTION("const&")
   {
      const auto right_int = right<int>(1);
      const auto right_float = right<float>(1.0f);
      const auto right_vector = right<std::vector<int>>({1, 1, 1});

      CHECK(right_int.borrow() == 1);
      CHECK(right_float.borrow() == 1.0f);
      CHECK(std::size(right_vector.borrow()) == 3);
   }
   SECTION("&")
   {
      auto right_int = right<int>(1);
      auto right_float = right<float>(1.0f);
      auto right_vector = right<std::vector<int>>({1, 1, 1});

      CHECK(right_int.borrow() == 1);
      CHECK(right_float.borrow() == 1.0f);
      CHECK(std::size(right_vector.borrow()) == 3);
   }
   SECTION("&&")
   {
      CHECK(right<int>(1).take() == 1);
      CHECK(right<float>(1.0f).take() == 1.0f);
      CHECK(right<std::vector<int>>({1, 1, 1}).take() == std::vector<int>({1, 1, 1}));
   }
}

TEST_CASE("right - comparison")
{
   SECTION("right<T> comparison with right<T>")
   {
      const auto right_int = right<int>(1);
      const auto right_float = right<float>(1.0f);
      const auto right_vector = right<std::vector<int>>({1, 1, 1});

      CHECK(right<int>(0) != right_int);
      CHECK(right<float>(0.0f) != right_float);
      CHECK(right<std::vector<int>>({1, 1, 0}) != right_vector);

      CHECK(right<int>(1) == right_int);
      CHECK(right<float>(1.0f) == right_float);
      CHECK(right<std::vector<int>>({1, 1, 1}) == right_vector);
   }

   SECTION("right<T> comparison with left<T>")
   {
      const auto right_int = right<int>(1);
      const auto right_float = right<float>(1.0f);
      const auto right_vector = right<std::vector<int>>({1, 1, 1});

      CHECK(left<int>(0) != right_int);
      CHECK(left<float>(0.0f) != right_float);
      CHECK(left<std::vector<int>>({1, 1, 0}) != right_vector);

      CHECK(left<int>(1) == right_int);
      CHECK(left<float>(1.0f) == right_float);
      CHECK(left<std::vector<int>>({1, 1, 1}) == right_vector);
   }
}
