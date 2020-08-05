#pragma once

#include <concepts>
#include <type_traits>

namespace monad
{
   // clang-format off
   template <typename any_>
   concept trivially_default_constructible = std::is_trivially_default_constructible_v<any_>;

   template <typename any_>
   concept trivially_copy_constructible = std::is_trivially_copy_constructible_v<any_>;

   template <typename any_>
   concept trivially_copy_assignable = std::is_trivially_copy_assignable_v<any_>;

   template <typename any_>
   concept trivially_copyable = std::is_trivially_copyable_v<any_>;

   template <typename any_>
   concept trivially_destructible = std::is_trivially_destructible_v<any_>;

   template <typename any_>
   concept trivial = 
      trivially_default_constructible<any_> &&
      trivially_copyable<any_> &&  
      trivially_destructible<any_>;
   // clang-format on
} // namespace monad
