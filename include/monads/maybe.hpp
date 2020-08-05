#pragma once

#include <cassert>
#include <compare>
#include <functional>
#include <memory>
#include <utility>

namespace monad
{
   // clang-format off
   template <class any_>
      requires(!std::is_reference_v<any_>) 
   class maybe;
   // clang-format on

   template <>
   class maybe<void>
   {
   };

   using none_t = maybe<void>;
   static inline constexpr auto none = none_t{}; // NOLINT

   // clang-format off
   template <class any_>
      requires(!std::is_reference_v<any_>) 
   class maybe
   // clang-format on
   {
      static inline constexpr bool is_nt_copy_value_constructible =
         std::is_nothrow_copy_constructible_v<any_>;

      static inline constexpr bool is_nt_move_value_constructible =
         std::is_nothrow_move_constructible_v<any_>;

      static inline constexpr bool is_nt_value_copyable =
         std::is_nothrow_copy_assignable_v<any_> && std::is_nothrow_copy_constructible_v<any_>;

      static inline constexpr bool is_nt_value_movable =
         std::is_nothrow_move_assignable_v<any_> && std::is_nothrow_move_constructible_v<any_>;

      static inline constexpr bool is_nt_swappable =
         std::is_nothrow_move_constructible_v<any_> && std::is_nothrow_swappable_v<any_>;

      static inline constexpr bool is_nt_destructible = std::is_nothrow_destructible_v<any_>;

   public:
      using value_type = any_;

      constexpr maybe() noexcept = default;
      constexpr maybe(const value_type& value) noexcept(is_nt_copy_value_constructible)
      {
         std::construct_at(pointer(), value);
      }
      constexpr maybe(value_type&& value) noexcept(is_nt_move_value_constructible)
      {
         std::construct_at(pointer(), std::move(value));
      }
      constexpr maybe(const maybe& rhs) noexcept(is_nt_copy_value_constructible) requires(
         !std::is_trivially_copy_constructible_v<value_type>) :
         m_is_engaged{rhs.engaged()}
      {
         if (has_value())
         {
            std::construct_at(pointer(), rhs.value());
         }
      }
      constexpr maybe(const maybe& rhs) noexcept = default;
      constexpr maybe(maybe&& rhs) noexcept(is_nt_move_value_constructible) requires(
         !std::is_trivially_move_constructible_v<value_type>) :
         m_is_engaged{rhs.engaged()}
      {
         if (has_value())
         {
            std::construct_at(pointer(), std::move(rhs.value));
            rhs.m_is_engaged = false;
         }
      }
      constexpr maybe(maybe&& rhs) noexcept = default;
      constexpr maybe(none_t) noexcept {}
      constexpr ~maybe() noexcept(is_nt_destructible) requires(
         !std::is_trivially_destructible_v<value_type>)
      {
         if (m_is_engaged)
         {
            std::destroy_at(pointer());
         }
      }
      constexpr ~maybe() noexcept = default;

      constexpr auto operator=(const maybe& rhs) noexcept(is_nt_value_copyable)
         -> maybe& requires(!std::is_trivially_copy_assignable_v<value_type>)
      {
         if (this != &rhs)
         {
            if (has_value())
            {
               std::destroy_at(pointer());
            }

            m_is_engaged = rhs.m_is_engaged;

            if (m_is_engaged)
            {
               std::construct_at(pointer(), rhs.value());
            }
         }

         return *this;
      }
      constexpr auto operator=(maybe&& rhs) noexcept(is_nt_value_movable)
         -> maybe& requires(!std::is_trivially_move_assignable_v<value_type>)
      {
         if (this != &rhs)
         {
            if (has_value())
            {
               std::destroy_at(pointer());
            }

            m_is_engaged = rhs.m_is_engaged;
            rhs.m_is_engaged = false;

            if (has_value())
            {
               std::construct_at(pointer(), std::move(rhs.value()));
            }
         }

         return *this;
      }

      constexpr auto operator->() noexcept -> value_type*
      {
         assert(has_value());

         return pointer();
      }
      constexpr auto operator->() const noexcept -> const value_type*
      {
         assert(has_value());

         return pointer();
      }

      constexpr auto operator*() & noexcept -> value_type& { value(); }
      constexpr auto operator*() const& noexcept(is_nt_value_copyable) -> const value_type&
      {
         return value();
      }
      constexpr auto operator*() && noexcept(is_nt_value_movable) -> value_type&&
      {
         return std::move(value());
      }
      constexpr auto operator*() const&& noexcept(is_nt_value_movable) -> const value_type&&
      {
         return std::move(value());
      }

      [[nodiscard]] constexpr auto has_value() const noexcept -> bool { return m_is_engaged; }
      constexpr operator bool() const noexcept { return has_value(); }

      constexpr void
      swap(maybe& other) noexcept(is_nt_swappable) requires std::swappable<value_type>
      {
         if (has_value() && other.has_value())
         {
            std::swap(value(), other.value());
         }

         if (has_value() && !other.has_value())
         {
            std::swap(m_is_engaged, other.m_is_engaged);
            other.value() = value();

            std::destroy_at(pointer());
         }

         if (!has_value() && other.has_value()())
         {
            std::swap(m_is_engaged, other.m_is_engaged);
            value() = other.value();

            std::destroy_at(other.pointer());
         }
      }

      constexpr auto value() & noexcept -> value_type&
      {
         assert(has_value());

         return *pointer();
      }
      constexpr auto value() const& noexcept(is_nt_value_copyable) -> const value_type&
      {
         assert(has_value());

         return *pointer();
      }
      constexpr auto value() && noexcept(is_nt_value_movable) -> value_type&&
      {
         assert(has_value());

         return std::move(*pointer());
      }
      constexpr auto value() const&& noexcept(is_nt_value_movable) -> const value_type&&
      {
         assert(has_value());

         return std::move(*pointer());
      }

      constexpr auto
      value_or(std::convertible_to<value_type> auto&& default_value) const& -> value_type
      {
         return has_value()
            ? value()
            : static_cast<value_type>(std::forward<decltype(default_value)>(default_value));
      }
      constexpr auto value_or(std::convertible_to<value_type> auto&& default_value) && -> value_type
      {
         return has_value()
            ? std::move(value())
            : static_cast<value_type>(std::forward<decltype(default_value)>(default_value));
      }

      constexpr void reset() noexcept(std::is_nothrow_destructible_v<value_type>)
      {
         if (has_value())
         {
            std::destroy_at(pointer());
         }
      }

      constexpr auto map(const std::invocable<value_type> auto& fun) const&
      {
         using result_type = std::invoke_result_t<decltype(fun), value_type>;

         return !has_value() ? maybe<result_type>{} : maybe<result_type>{std::invoke(fun, value())};
      }
      constexpr auto map(const std::invocable<value_type> auto& fun) &
      {
         using result_type = std::invoke_result_t<decltype(fun), value_type>;

         return !has_value() ? maybe<result_type>{} : maybe<result_type>{std::invoke(fun, value())};
      }
      constexpr auto map(const std::invocable<value_type> auto& fun) const&&
      {
         using result_type = std::invoke_result_t<decltype(fun), value_type>;

         return !has_value() ? maybe<result_type>{}
                             : maybe<result_type>{std::invoke(fun, std::move(value()))};
      }
      constexpr auto map(const std::invocable<value_type> auto& fun) &&
      {
         using result_type = std::invoke_result_t<decltype(fun), value_type>;

         return !has_value() ? maybe<result_type>{}
                             : maybe<result_type>{std::invoke(fun, std::move(value()))};
      }

   private:
      constexpr auto pointer() noexcept -> value_type*
      {
         return reinterpret_cast<value_type*>(m_bytes.data()); // NOLINT
      }
      constexpr auto pointer() const noexcept -> const value_type*
      {
         return reinterpret_cast<const value_type*>(m_bytes.data()); // NOLINT
      }

   private:
      alignas(value_type) std::array<std::byte, sizeof(value_type)> m_bytes;
      bool m_is_engaged{false};
   };

   template <class any_>
   constexpr auto make_maybe(any_&& value) -> maybe<std::decay_t<any_>>
   {
      return maybe<std::decay_t<any_>>{std::forward<any_>(value)};
   }

   template <class first_, std::equality_comparable_with<first_> second_>
   constexpr auto
   operator==(const maybe<first_>& lhs,
              const maybe<second_>& rhs) noexcept(noexcept(lhs.value() == rhs.value())) -> bool
   {
      if (lhs.has_value() != rhs.has_value())
      {
         return false;
      }

      if (!lhs.has_value())
      {
         return true;
      }

      return lhs.value() == rhs.value();
   }

   template <class any_>
   constexpr auto operator==(const maybe<any_>& m, none_t) noexcept
   {
      return !m.has_value();
   }

   template <class any_>
   constexpr auto operator==(
      const maybe<any_>& m,
      const std::equality_comparable_with<any_> auto& value) noexcept(noexcept(m.value() == value))
   {
      return m.has_value() ? m.value() == value : false;
   }

   template <class first_, std::three_way_comparable_with<first_> second_>
   constexpr auto operator<=>(const maybe<first_>& lhs, const maybe<second_>& rhs)
      -> std::compare_three_way_result_t<first_, second_>
   {
      if (lhs.has_value() && rhs.has_value())
      {
         return lhs.value() <=> rhs.has_value();
      }

      return lhs.has_value() <=> rhs.has_value();
   }

   template <class any_>
   constexpr auto operator<=>(const maybe<any_>& m, none_t) noexcept -> std::strong_ordering
   {
      return m.has_value() <=> false;
   }

   template <class any_>
   constexpr auto operator<=>(const maybe<any_>& m,
                              const std::three_way_comparable_with<any_> auto&
                                 value) noexcept(noexcept(m.value() <=> value))
      -> std::compare_three_way_result_t<any_, decltype(value)>
   {
      return m.has_value() ? m.value() <=> value : std::strong_ordering::less;
   }
} // namespace monad

namespace std // NOLINT
{
   template <class any_>
   constexpr void swap(monad::maybe<any_>& lhs,
                       monad::maybe<any_>& rhs) noexcept(noexcept(lhs.swap(rhs)))
   {
      lhs.swap(rhs);
   }
} // namespace std
