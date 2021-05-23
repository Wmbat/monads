#pragma once

#include <libreglisse/type_traits.hpp>

#include <cassert>
#include <compare>
#include <functional>
#include <memory>
#include <utility>

namespace monad
{
   /**
    * Helper struct for the "in place" construction of an object within a maybe monad
    */
   struct in_place_t
   {
      in_place_t() noexcept = default;
   };

   static constexpr in_place_t in_place;

   // clang-format off

   /**
    * A class used for a functional programming way of holding values that may or may not be returned from a function
    */
   template <class Any> 
      requires(!std::is_reference_v<Any>) 
   class [[nodiscard("a maybe should never be discarded")]] maybe;

   // clang-format on

   /**
    * Specialization of the maybe monad class for void types. Used for the representation of an
    * empty maybe monad
    */
   template <>
   class maybe<void>
   {
   };

   /**
    * Helper type alias for a maybe<void> monad, used to represent an empty maybe monad
    */
   using none_t = maybe<void>;

   static inline constexpr auto none = none_t{}; // NOLINT

   // clang-format off
   template <class Any> 
      requires(!std::is_reference_v<Any>) 
   class [[nodiscard("a maybe should never be discarded")]] maybe
   // clang-format on
   {
      template <class T>
      class storage
      {
      public:
         using value_type = T;

      private:
         // clang-format off
         static constexpr bool is_nothrow_value_copyable =
            std::is_nothrow_copy_assignable_v<value_type> &&
            std::is_nothrow_copy_constructible_v<value_type>;

         static constexpr bool is_nothrow_value_movable =
            std::is_nothrow_move_assignable_v<value_type> &&
            std::is_nothrow_move_constructible_v<value_type>;

         static constexpr bool is_nothrow_swappable =
            std::is_nothrow_move_assignable_v<value_type> &&
            std::is_nothrow_destructible_v<value_type> && 
            std::is_nothrow_swappable_v<value_type>;
         // clang-format on

      public:
         constexpr storage() noexcept = default;
         constexpr storage(const value_type& value) noexcept(
            std::is_nothrow_copy_constructible_v<value_type>) :
            m_is_engaged{true}
         {
            std::construct_at(pointer(), value);
         }
         constexpr storage(value_type&& value) noexcept(
            std::is_nothrow_move_constructible_v<value_type>) :
            m_is_engaged{true}
         {
            std::construct_at(pointer(), std::move(value));
         }
         constexpr storage(in_place_t, auto&&... args) noexcept(
            std::is_nothrow_constructible_v<value_type, decltype(args)...>) :
            m_is_engaged{true}
         {
            std::construct_at(pointer(), std::forward<decltype(args)>(args)...);
         }
         constexpr storage(const storage& rhs) noexcept(
            std::is_nothrow_copy_constructible_v<value_type>) :
            m_is_engaged{rhs.engaged()}
         {
            if (engaged())
            {
               std::construct_at(pointer(), rhs.value());
            }
         }
         constexpr storage(storage&& rhs) noexcept(
            std::is_nothrow_move_constructible_v<value_type>) :
            m_is_engaged{rhs.engaged()}
         {
            if (engaged())
            {
               std::construct_at(pointer(), std::move(rhs.value()));
               rhs.m_is_engaged = false;
            }
         }
         ~storage() noexcept(std::is_nothrow_destructible_v<value_type>)
         {
            if (engaged())
            {
               std::destroy_at(pointer());
               m_is_engaged = false;
            }
         }

         constexpr auto
         operator=(const storage& rhs) noexcept(std::is_nothrow_copy_constructible_v<value_type>)
            -> storage&
         {
            if (this != &rhs)
            {
               if (engaged())
               {
                  std::destroy_at(pointer());
               }

               m_is_engaged = rhs.engaged();

               if (m_is_engaged)
               {
                  std::construct_at(pointer(), rhs.value());
               }
            }

            return *this;
         }
         constexpr auto
         operator=(storage&& rhs) noexcept(std::is_nothrow_move_constructible_v<value_type>)
            -> storage&
         {
            if (this != &rhs)
            {
               if (engaged())
               {
                  std::destroy_at(pointer());
               }

               m_is_engaged = rhs.engaged();
               rhs.m_is_engaged = false;

               if (engaged())
               {
                  std::construct_at(pointer(), std::move(rhs.value()));
               }
            }

            return *this;
         }

         [[nodiscard]] constexpr auto engaged() const noexcept -> bool { return m_is_engaged; }

         constexpr void
         swap(storage& other) noexcept(is_nothrow_swappable) requires std::swappable<value_type>
         {
            if (engaged() && other.engaged())
            {
               std::swap(value(), other.value());
            }
            else if (engaged() && !other.engaged())
            {
               other.m_is_engaged = true;
               other.value() = value();

               m_is_engaged = false;
               std::destroy_at(pointer());
            }
            else if (!engaged() && other.engaged())
            {
               m_is_engaged = true;
               value() = other.value();

               other.m_is_engaged = false;
               std::destroy_at(other.pointer());
            }
         }

         constexpr auto pointer() noexcept -> value_type*
         {
            return reinterpret_cast<value_type*>(m_bytes.data()); // NOLINT
         }
         constexpr auto pointer() const noexcept -> const value_type*
         {
            return reinterpret_cast<const value_type*>(m_bytes.data()); // NOLINT
         }

         constexpr auto value() & noexcept -> value_type& { return *pointer(); }
         constexpr auto value() const& noexcept(is_nothrow_value_copyable) -> const value_type&
         {
            return *pointer();
         }
         constexpr auto value() && noexcept(is_nothrow_value_movable) -> value_type&&
         {
            return std::move(*pointer());
         }
         constexpr auto value() const&& noexcept(is_nothrow_value_movable) -> const value_type&&
         {
            return std::move(*pointer());
         }

         constexpr void reset() noexcept(std::is_nothrow_destructible_v<value_type>)
         {
            if (engaged())
            {
               std::destroy_at(pointer());
               m_is_engaged = false;
            }
         }

      private:
         alignas(value_type) std::array<std::byte, sizeof(value_type)> m_bytes;
         bool m_is_engaged{false};
      };

      // clang-format off
      template <class T> requires trivial<T> 
      class storage<T>
      // clang-format on
      {
      public:
         using value_type = T;

         constexpr storage() noexcept = default;
         constexpr storage(const value_type& value) noexcept : m_is_engaged{true}
         {
            std::construct_at(pointer(), value);
         }
         constexpr storage(value_type&& value) noexcept : m_is_engaged{true}
         {
            std::construct_at(pointer(), std::move(value));
         }
         constexpr storage(in_place_t, auto&&... args) noexcept : m_is_engaged{true}
         {
            std::construct_at(pointer(), std::forward<decltype(args)>(args)...);
         }

         [[nodiscard]] constexpr auto engaged() const noexcept -> bool { return m_is_engaged; }

         constexpr void swap(storage& other) noexcept requires std::swappable<value_type>
         {
            if (engaged() && other.engaged())
            {
               std::swap(value(), other.value());
            }
            else if (engaged() && !other.engaged())
            {
               other.m_is_engaged = true;
               other.value() = value();
               m_is_engaged = false;
            }
            else if (!engaged() && other.engaged())
            {
               m_is_engaged = true;
               value() = other.value();
               other.m_is_engaged = false;
            }
         }

         constexpr auto pointer() noexcept -> value_type*
         {
            return reinterpret_cast<value_type*>(m_bytes.data()); // NOLINT
         }
         constexpr auto pointer() const noexcept -> const value_type*
         {
            return reinterpret_cast<const value_type*>(m_bytes.data()); // NOLINT
         }

         constexpr auto value() & noexcept -> value_type& { return *pointer(); }
         constexpr auto value() const& noexcept -> const value_type& { return *pointer(); }
         constexpr auto value() && noexcept -> value_type&& { return std::move(*pointer()); }
         constexpr auto value() const&& noexcept -> const value_type&&
         {
            return std::move(*pointer());
         }

         constexpr void reset() noexcept(std::is_nothrow_destructible_v<value_type>)
         {
            if (engaged())
            {
               std::destroy_at(pointer());
               m_is_engaged = false;
            }
         }

      private:
         alignas(value_type) std::array<std::byte, sizeof(value_type)> m_bytes;
         bool m_is_engaged{false};
      };

      using storage_type = storage<Any>;

      static constexpr bool is_nothrow_value_copyable =
         std::is_nothrow_copy_assignable_v<Any> && std::is_nothrow_copy_constructible_v<Any>;

      static constexpr bool is_nothrow_value_movable =
         std::is_nothrow_move_assignable_v<Any> && std::is_nothrow_move_constructible_v<Any>;

      static constexpr bool is_nothrow_swappable =
         std::is_nothrow_move_constructible_v<Any> && std::is_nothrow_swappable_v<Any>;

   public:
      // clang-format off

      using value_type = typename storage_type::value_type;

      /**
       * Default construct a maybe monad. It will be interpret as being an empty maybe
       */
      constexpr maybe() noexcept = default;
      /**
       * Construct a empty monad
       */
      constexpr maybe(none_t) noexcept {}

      /**
       * Construct a maybe monad using a `value` by copy
       */
      constexpr maybe(const value_type& value) noexcept(
         std::is_nothrow_constructible_v<storage_type, Any>) :
         m_storage{value}
      {}
      /**
       * Construct a maybe monad using a `value` by move
       */
      constexpr maybe(value_type &&value) 
         noexcept(std::is_nothrow_constructible_v<storage_type, Any&&>) :
         m_storage{std::move(value)}
      {}
      /**
       * Construct a maybe monad by creating a value in place from a range of variadic arguments
       */
      template<class... Args>
      constexpr maybe(in_place_t, Args&&... args) 
         noexcept(std::is_nothrow_constructible_v<value_type, Args...>) 
      requires std::constructible_from<value_type, Args...> 
         : m_storage{in_place, std::forward<Args>(args)...}
      {}

      /**
       * A shorthand operator to acces the underlying value. This operator does not guarentee the
       * returned value will be valid
       */
      constexpr auto operator->() noexcept -> value_type*
      {
         assert(has_value());

         return m_storage.pointer();
      }
      /**
       * A shorthand operator to acces the underlying value. This operator does not guarentee the
       * returned value will be valid
       */
      constexpr auto operator->() const noexcept -> const value_type*
      {
         assert(has_value());

         return m_storage.pointer();
      }

      /**
       * Return the stored value
       */
      constexpr auto operator*() & noexcept -> value_type& { return m_storage.value(); }
      /**
       * Return the stored value
       */
      constexpr auto operator*() const & noexcept(is_nothrow_value_copyable) -> const value_type&
      {
         return m_storage.value();
      }
      /**
       * Return the stored value
       */
      constexpr auto operator*() && noexcept(is_nothrow_value_movable) -> value_type&&
      {
         return std::move(m_storage.value());
      }
      /**
       * Return the stored value
       */
      constexpr auto operator*() const && noexcept(is_nothrow_value_movable) -> const value_type&&
      {
         return std::move(m_storage.value());
      }

      /**
       * Check if a value is present
       */
      [[nodiscard]] constexpr auto has_value() const noexcept -> bool 
      { 
         return m_storage.engaged(); 
      }
      constexpr operator bool() const noexcept { return has_value(); }

      /**
       * Return the stored value
       */
      constexpr auto value() & noexcept -> value_type&
      {
         assert(has_value());

         return m_storage.value();
      }
      /**
       * Return the stored value
       */
      constexpr auto value() const& noexcept(is_nothrow_value_copyable) -> const value_type&
      {
         assert(has_value());

         return m_storage.value();
      }
      /**
       * Return the stored value
       */
      constexpr auto value() && noexcept(is_nothrow_value_movable) -> value_type&&
      {
         assert(has_value());

         return std::move(m_storage.value());
      }
      /**
       * Return the stored value
       */
      constexpr auto value() const&& noexcept(is_nothrow_value_movable) -> const value_type&&
      {
         assert(has_value());

         return std::move(m_storage.value());
      }

      /**
       * Take the value out of the maybe into a new maybe, leaving it empty
       */
      constexpr auto take() -> maybe
      {
         maybe ret = std::move(*this);
         reset();
         return ret;
      }

      /**
       * Return the stored value or a specified value
       */
      constexpr auto value_or(std::convertible_to<value_type> auto&& default_value) const& 
         -> value_type
      {
         return has_value()
            ? value()
            : static_cast<value_type>(std::forward<decltype(default_value)>(default_value));
      }
      /**
       * Return the stored value or a specified value
       */
      constexpr auto value_or(std::convertible_to<value_type> auto&& default_value) && -> value_type
      {
         return has_value()
            ? std::move(value())
            : static_cast<value_type>(std::forward<decltype(default_value)>(default_value));
      }

      constexpr void swap(maybe & other) noexcept(noexcept(m_storage.swap(other.m_storage)))
      {
         m_storage.swap(other.m_storage);
      }

      /**
       * Destroy the stored value if it exists and set itselfs to being empty
       */
      constexpr void reset() noexcept(std::is_nothrow_destructible_v<value_type>)
      {
         m_storage.reset();
      }

      /**
       * Carries out some operation on the stored object if there is one
       */
      template<std::invocable<value_type> Fun> 
      constexpr auto map(Fun&& fun) const& -> maybe<std::invoke_result_t<Fun, value_type>>
      {
         if(has_value())
         {
            return {std::invoke(std::forward<Fun>(fun), value())};
         }
         else
         {
            return none;
         }
      }
      /**
       * Carries out some operation on the stored object if there is one
       */
      template<std::invocable<value_type> Fun> 
      constexpr auto map(Fun&& fun) & -> maybe<std::invoke_result_t<Fun, value_type>>
      {
         if(has_value())
         {
            return {std::invoke(std::forward<Fun>(fun), value())};
         }
         else
         {
            return none;
         }
      }
      /**
       * Carries out some operation on the stored object if there is one
       */
      template<std::invocable<value_type> Fun> 
      constexpr auto map(Fun&& fun) const&& -> maybe<std::invoke_result_t<Fun, value_type&&>>
      {
         if(has_value())
         {
            return {std::invoke(std::forward<Fun>(fun), std::move(value()))};
         }
         else
         {
            return none;
         }
      }
      /**
       * Carries out some operation on the stored object if there is one
       */
      template<std::invocable<value_type> Fun> 
      constexpr auto map(Fun&& fun) && -> maybe<std::invoke_result_t<Fun, value_type&&>> 
      {
         if(has_value())
         {
            return {std::invoke(std::forward<Fun>(fun), std::move(value()))};
         }
         else
         {
            return none;
         }
      }

      /**
       * Carries out an operation on the stored object if there is one, or returns
       * a default value
       */
      template<std::invocable<value_type> Fun, class Other>
      constexpr auto map_or(Fun&& fun, Other&& other) const& 
         -> std::common_type_t<std::invoke_result_t<Fun, value_type>, Other>
      {
         if(has_value())
         {
            return std::invoke(std::forward<decltype(fun)>(fun), value());
         }
         else
         {
            return std::forward<decltype(other)>(other);
         }
      }
      /**
       * Carries out an operation on the stored object if there is one, or returns
       * a default value
       */
      template<std::invocable<value_type> Fun, class Other>
      constexpr auto map_or(Fun&& fun, Other&& other) & 
         -> std::common_type_t<std::invoke_result_t<Fun, value_type>, Other>
      {
         if(has_value())
         {
            return std::invoke(std::forward<decltype(fun)>(fun), value());
         }
         else
         {
            return std::forward<decltype(other)>(other);
         }
      }
      /**
       * Carries out an operation on the stored object if there is one, or returns
       * a default value
       */
      template<std::invocable<value_type> Fun, class Other>
      constexpr auto map_or(Fun&& fun, Other&& other) const&&
         -> std::common_type_t<std::invoke_result_t<Fun, value_type&&>, Other>
      {
         if(has_value())
         {
            return std::invoke(std::forward<Fun>(fun), std::move(value()));
         }
         else
         {
            return std::forward<Other>(other);
         }
      }
      /**
       * Carries out an operation on the stored object if there is one, or returns
       * a default value
       */
      template<std::invocable<value_type> Fun, class Other>
      constexpr auto map_or(Fun&& fun, Other&& other) && 
         -> std::common_type_t<std::invoke_result_t<Fun, value_type&&>, Other>
      {
         if(has_value())
         {
            return std::invoke(std::forward<Fun>(fun), std::move(value()));
         }
         else
         {
            return std::forward<Other>(other);
         }
      }

      /**
       * Carries out some operation that returns a monad::maybe on the stored object
       * if there is one
       */
      template<std::invocable<value_type> Fun> 
      constexpr auto and_then(Fun&& fun) const& -> std::invoke_result_t<Fun, value_type>
      {
         if(has_value())
         {
            return std::invoke(std::forward<Fun>(fun), value());
         }
         else
         {
            return none;
         }
      }
      /**
       * Carries out some operation that returns a monad::maybe on the stored object
       * if there is one
       */
      template<std::invocable<value_type> Fun> 
      constexpr auto and_then(Fun&& fun) & -> std::invoke_result_t<Fun, value_type>
      {
         if(has_value())
         {
            return std::invoke(std::forward<Fun>(fun), value());
         }
         else
         {
            return none;
         }
      }
      /**
       * Carries out some operation that returns a monad::maybe on the stored object
       * if there is one
       */
      template<std::invocable<value_type> Fun> 
      constexpr auto and_then(Fun&& fun) const&& -> std::invoke_result_t<Fun, value_type&&> 
      {
         if(has_value())
         {
            return std::invoke(std::forward<Fun>(fun), std::move(value()));
         }
         else
         {
            return none;
         }
      }
      /**
       * Carries out some operation that returns a monad::maybe on the stored object
       * if there is one
       */
      template<std::invocable<value_type> Fun> 
      constexpr auto and_then(Fun&& fun) && -> std::invoke_result_t<Fun, value_type&&>
      {
         if(has_value())
         {
            return std::invoke(std::forward<Fun>(fun), std::move(value()));
         }
         else
         {
            return none;
         }
      }

      /**
       * Carries out an operation if there is no value stored
       */
      template<std::invocable Fun>
      constexpr auto or_else(Fun&& fun) const& -> maybe<value_type>
      {
         if (has_value())
         {
            return *this;
         }
         else
         {
            return std::invoke(std::forward<Fun>(fun));
         }
      }
      /**
       * Carries out an operation if there is no value stored
       */
      template<std::invocable Fun>
      constexpr auto or_else(Fun&& fun) & -> maybe<value_type>
      {
         if (has_value())
         {
            return *this;
         }
         else
         {
            return std::invoke(std::forward<Fun>(fun));
         }
      }
      /**
       * Carries out an operation if there is no value stored
       */
      template<std::invocable Fun>
      constexpr auto or_else(Fun&& fun) const&& -> maybe<value_type>
      {
         if (has_value())
         {
            return std::move(*this);
         }
         else
         {
            return std::invoke(std::forward<Fun>(fun));
         }
      }
      /**
       * Carries out an operation if there is no value stored
       */
      template<std::invocable Fun>
      constexpr auto or_else(Fun&& fun) && -> maybe<value_type>
      {
         if (has_value())
         {
            return std::move(*this);
         }
         else
         {
            return std::invoke(std::forward<Fun>(fun));
         }
      }

      /**
       * Carries out an operation on the stored object if there is one, or return
       * a the result of a given function
       */
      template<std::invocable<value_type> Fun, std::invocable Def>
      constexpr auto map_or_else(Fun&& fun, Def&& def) const& -> std::invoke_result_t<Def> 
      requires 
         std::convertible_to<std::invoke_result_t<Fun, value_type&&>, 
                             std::invoke_result_t<Def>>
      {
         if(has_value())
         {
            return std::invoke(std::forward<Fun>(fun), value());
         }
         else
         {
            return std::invoke(std::forward<Def>(def));
         }
      }
      /**
       * Carries out an operation on the stored object if there is one, or return
       * a the result of a given function
       */
      template<std::invocable<value_type> Fun, std::invocable Def>
      constexpr auto map_or_else(Fun&& fun, Def&& def) & -> std::invoke_result_t<Def> 
      requires 
         std::convertible_to<std::invoke_result_t<Fun, value_type&&>, 
                             std::invoke_result_t<Def>>
      {
         if(has_value())
         {
            return std::invoke(std::forward<Fun>(fun), value());
         }
         else
         {
            return std::invoke(std::forward<Def>(def));
         }
      }
      /**
       * Carries out an operation on the stored object if there is one, or return
       * a the result of a given function
       */
      template<std::invocable<value_type> Fun, std::invocable Def>
      constexpr auto map_or_else(Fun&& fun, Def&& def) const&& -> std::invoke_result_t<Def> 
      requires 
         std::convertible_to<std::invoke_result_t<Fun, value_type&&>, 
                             std::invoke_result_t<Def>>
      {
         if (has_value())
         {
            return std::invoke(std::forward<Fun>(fun), std::move(value()));
         }
         else
         {
            return std::invoke(std::forward<Def>(def));
         }
      }
      /**
       * Carries out an operation on the stored object if there is one, or return
       * a the result of a given function
       */
      template<std::invocable<value_type> Fun, std::invocable Def>
      constexpr auto map_or_else(Fun&& fun, Def&& def) && -> std::invoke_result_t<Def> 
      requires 
         std::convertible_to<std::invoke_result_t<Fun, value_type&&>, 
                             std::invoke_result_t<Def>>
      {
         if (has_value())
         {
            return std::invoke(std::forward<Fun>(fun), std::move(value()));
         }
         else
         {
            return std::invoke(std::forward<Def>(def));
         }
      }
      // clang-format off

   private:
      storage<value_type> m_storage{};
   };

   /**
    * Handy function to create a maybe from any value.
    */
   template <class Any>
   constexpr auto make_maybe(Any&& value) -> maybe<std::decay_t<Any>>
   {
      return {std::forward<Any>(value)};
   }

   /**
    * Compare two maybe libreglisse. If either of the two libreglisse are empty, it will return false. If both
    * libreglisse are empty, true will be returned, and if both libreglisse have values, a comparison between
    * the value held by the libreglisse will be performed
    */
   template <class First, std::equality_comparable_with<First> Second>
   constexpr auto
   operator==(const maybe<First>& lhs, const maybe<Second>& rhs) 
      noexcept(noexcept(lhs.value() == rhs.value())) 
      -> bool
   {
      if (lhs.has_value() != rhs.has_value())
      {
         return false;
      }
      else if (!lhs.has_value())
      {
         return true;
      }
      else
      { 
         return lhs.value() == rhs.value();
      }
   }

   /**
    * Compare a maybe monad to an empty maybe. Returns true if the first maybe is empty.
    */
   template <class Any>
   constexpr auto operator==(const maybe<Any>& m, none_t) noexcept -> bool
   {
      return !m.has_value();
   }

   /**
    * Compare a maybe monad to a generic type that may be compared with the inner type of the maybe
    * monad. If the maybe monad does not hold a value, otherwise, a comparison between the value
    * held by the maybe monad and the value provided will be performed.
    */
   template <class Any, class Other>
   constexpr auto operator==(const maybe<Any>& m, const Other& value) 
      noexcept(noexcept(m.value() == value))
      -> bool
   {
      return m.has_value() ? m.value() == value : false;
   }

   /**
    * three-way compare two maybe libreglisse. If both libreglisse have values, a three-way comparison will be
    * performed on their inner values otherwise, a three-way comparison will be performed on whether
    * they hold values or not.
    */
   template <class First, class Second>
   constexpr auto operator<=>(const maybe<First>& lhs, const maybe<Second>& rhs)
      noexcept(noexcept(lhs.value() <=> rhs.value()))
      -> std::compare_three_way_result_t<First, Second>
   {
      if (lhs.has_value() && rhs.has_value())
      {
         return lhs.value() <=> rhs.value();
      }
      else
      {
         return lhs.has_value() <=> rhs.has_value();
      }
   }

   /**
    * Compare a maybe to an empty maybe. Returns a `strong_ordering` based on whether the first
    * maybe has a value or not
    */
   template <class Any>
   constexpr auto operator<=>(const maybe<Any>& m, none_t) noexcept -> std::strong_ordering
   {
      return m.has_value() <=> false;
   }

   /**
    * Perform a three-way comparison between a maybe monad and a value. If the maybe monad does not
    * hold a value, `strong_ordering::less` will be returned. Otherwise, a three-way comparison
    * between the value held by the monad and the value provided as parameter will be performed and
    * its ordering returned.
    */
   template <class Any, class Other>
   constexpr auto operator<=>(const maybe<Any>& m, const Other& value) 
      noexcept(noexcept(m.value() <=> value))
      -> std::compare_three_way_result_t<Any, Other>
   {
      return m.has_value() ? m.value() <=> value : std::strong_ordering::less;
   }
   // clang-format on
} // namespace monad

namespace std // NOLINT
{
   template <class Any>
   constexpr void swap(monad::maybe<Any>& lhs,
                       monad::maybe<Any>& rhs) noexcept(noexcept(lhs.swap(rhs)))
   {
      lhs.swap(rhs);
   }
} // namespace std