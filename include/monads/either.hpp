#pragma once

#include <monads/trivial.hpp>
#include <monads/maybe.hpp>

namespace monad
{
   // clang-format off
   template <class left_, class right_>
      requires (!(std::is_reference_v<left_> || std::is_reference_v<right_>))
   class either;
   // clang-format on

   namespace detail
   {
      consteval auto max(size_t size) noexcept -> size_t { return size; }
      consteval auto max(size_t first, size_t second, std::unsigned_integral auto... rest) noexcept
         -> size_t
      {
         return first >= second ? max(first, rest...) : max(second, rest...);
      }

      template <class in_left_, class in_right_>
      constexpr auto ensure_either_right(const either<in_left_, in_right_>& e, in_right_)
         -> either<in_left_, in_right_>
      {
         return e;
      }
      template <class in_left_, class in_right_>
      constexpr auto ensure_either_right(either<in_left_, in_right_>&& e, in_right_ &&)
         -> either<in_left_, in_right_>
      {
         return e;
      }

      template <class in_left_, class in_right_>
      constexpr auto ensure_either_left(const either<in_left_, in_right_>& e, in_left_)
         -> either<in_left_, in_right_>
      {
         return e;
      }
      template <class in_left_, class in_right_>
      constexpr auto ensure_either_left(either<in_left_, in_right_>&& e, in_left_ &&)
         -> either<in_left_, in_right_>
      {
         return e;
      }
   } // namespace detail

   template <class any_>
   struct left_t
   {
      any_ value;
   };

   template <class any_>
   struct right_t
   {
      any_ value;
   };

   template <class any_>
   constexpr auto make_left(any_&& value) -> left_t<std::decay_t<any_>>
   {
      return left_t<std::decay_t<any_>>{std::forward<any_>(value)};
   }

   template <class any_>
   constexpr auto make_right(any_&& value) -> right_t<std::decay_t<any_>>
   {
      return right_t<std::decay_t<any_>>{std::forward<any_>(value)};
   }

   // clang-format off
   template <class left_, class right_> 
      requires (!(std::is_reference_v<left_> || std::is_reference_v<right_>))
   class either
   // clang-format on
   {
      template <class first_, class second_, class dummy_ = void>
      class storage
      {
      public:
         using left_type = first_;
         using right_type = second_;

      private:
         using storage_type =
            std::array<std::byte, detail::max(sizeof(left_type), sizeof(right_type))>;

         // clang-format off
         static inline constexpr bool is_nothrow_default_constructible =
            std::is_nothrow_default_constructible_v<left_type> && 
            std::is_nothrow_default_constructible_v<storage_type>;

         static inline constexpr bool is_nothrow_copy_left_constructible =
            std::is_nothrow_copy_assignable_v<left_type> &&
            std::is_nothrow_copy_constructible_v<left_type> && 
            std::is_nothrow_default_constructible_v<storage_type>;

         static inline constexpr bool is_nothrow_move_left_constructible =
            std::is_nothrow_move_assignable_v<left_type> &&
            std::is_nothrow_move_constructible_v<left_type> &&
            std::is_nothrow_default_constructible_v<storage_type>;

         static inline constexpr bool is_nothrow_copy_right_constructible =
            std::is_nothrow_copy_assignable_v<right_type> &&
            std::is_nothrow_copy_constructible_v<right_type> && 
            std::is_nothrow_default_constructible_v<storage_type>;

         static inline constexpr bool is_nothrow_move_right_constructible = 
            std::is_nothrow_move_assignable_v<right_type> &&
            std::is_nothrow_move_constructible_v<right_type> &&
            std::is_nothrow_default_constructible_v<storage_type>;

         static inline constexpr bool is_nothrow_copy_constructible =
            is_nothrow_copy_left_constructible && 
            is_nothrow_copy_right_constructible;

         static inline constexpr bool is_nothrow_move_constructible =
            is_nothrow_move_left_constructible && 
            is_nothrow_move_right_constructible;

         static inline constexpr bool is_nothrow_destructible = 
            std::is_nothrow_destructible_v<left_type> &&
            std::is_nothrow_destructible_v<right_type> &&
            std::is_nothrow_destructible_v<storage_type>;

         static inline constexpr bool is_nothrow_copy_assignable =
            is_nothrow_copy_constructible &&
            is_nothrow_destructible;

         static inline constexpr bool is_nothrow_move_assignable =
            is_nothrow_move_constructible &&
            is_nothrow_destructible;

         static inline constexpr bool is_nothrow_left_value_copyable =
            std::is_nothrow_copy_assignable_v<left_type> &&
            std::is_nothrow_copy_constructible_v<left_type>;

         static inline constexpr bool is_nothrow_left_value_movable =
            std::is_nothrow_move_assignable_v<left_type> &&
            std::is_nothrow_move_constructible_v<left_type>;

         static inline constexpr bool is_nothrow_right_value_copyable =
            std::is_nothrow_copy_assignable_v<right_type> &&
            std::is_nothrow_copy_constructible_v<right_type>;

         static inline constexpr bool is_nothrow_right_value_movable =
            std::is_nothrow_move_assignable_v<right_type> &&
            std::is_nothrow_move_constructible_v<right_type>;
         // clang-format on

      public:
         constexpr storage() noexcept(is_nothrow_default_constructible)
         {
            std::construct_at(std::addressof(left()), left_type{});
         };
         constexpr storage(const left_t<left_type>& l) noexcept(is_nothrow_copy_left_constructible)
         {
            std::construct_at(std::addressof(left()), l.value);
         }
         constexpr storage(left_t<left_type>&& l) noexcept(is_nothrow_move_left_constructible)
         {
            std::construct_at(std::addressof(left()), std::move(l.value));
         }
         constexpr storage(const right_t<right_type>& r) noexcept(
            is_nothrow_copy_right_constructible) :
            m_is_right{true}
         {
            std::construct_at(std::addressof(right()), r.value);
         }
         constexpr storage(right_t<right_type>&& r) noexcept(is_nothrow_move_right_constructible) :
            m_is_right{true}
         {
            std::construct_at(std::addressof(right()), std::move(r.value));
         }
         constexpr storage(const storage& rhs) noexcept(is_nothrow_copy_constructible) :
            m_is_right{rhs.is_right()}
         {
            if (!is_right())
            {
               std::construct_at(std::addressof(left()), rhs.left());
            }
            else
            {
               std::construct_at(std::addressof(right()()), rhs.right());
            }
         }
         constexpr storage(storage&& rhs) noexcept(is_nothrow_move_constructible) :
            m_is_right{rhs.is_right()}
         {
            if (!is_right())
            {
               std::construct_at(std::addressof(left()), std::move(rhs.left()));
            }
            else
            {
               std::construct_at(std::addressof(right()), std::move(rhs.right()));
            }

            rhs.m_is_right = false;
         }
         ~storage() noexcept(is_nothrow_destructible)
         {
            if (!is_right())
            {
               std::destroy_at(std::addressof(left()));
            }
            else
            {
               std::destroy_at(std::addressof(right()));
            }
         }

         constexpr auto operator=(const storage& rhs) -> storage&
         {
            if (this != &rhs)
            {
               if (!is_right())
               {
                  std::destroy_at(std::addressof(left()));
               }
               else
               {
                  std::destroy_at(std::addressof(right()));
               }

               m_is_right = rhs.is_right();

               if (!is_right())
               {
                  std::construct_at(std::addressof(left()), rhs.left());
               }
               else
               {
                  std::construct_at(std::addressof(right()), rhs.right());
               }
            }

            return *this;
         }
         constexpr auto operator=(storage&& rhs) noexcept(is_nothrow_move_assignable) -> storage&
         {
            if (this != &rhs)
            {
               if (!is_right())
               {
                  std::destroy_at(std::addressof(left()));
               }
               else
               {
                  std::destroy_at(std::addressof(right()));
               }

               m_is_right = rhs.is_right();
               rhs.m_is_right = false;

               if (!is_right())
               {
                  std::construct_at(std::addressof(left()), rhs.left());
               }
               else
               {
                  std::construct_at(std::addressof(right()), rhs.right());
               }
            }

            return *this;
         }

         [[nodiscard]] constexpr auto is_right() const noexcept -> bool { return m_is_right; };

         constexpr auto left() & noexcept -> left_type& { return *l_pointer(); }
         constexpr auto left() const& noexcept(is_nothrow_left_value_copyable) -> const left_type&
         {
            return *l_pointer();
         }
         constexpr auto left() && noexcept(is_nothrow_left_value_movable) -> left_type&&
         {
            return std::move(*l_pointer());
         }
         constexpr auto left() const&& noexcept(is_nothrow_left_value_movable) -> const left_type&&
         {
            return std::move(*l_pointer());
         }

         constexpr auto right() & noexcept -> right_type& { return *r_pointer(); }
         constexpr auto right() const& noexcept(is_nothrow_right_value_copyable)
            -> const right_type&
         {
            return *r_pointer();
         }
         constexpr auto right() && noexcept(is_nothrow_right_value_movable) -> right_type&&
         {
            return std::move(*r_pointer());
         }
         constexpr auto right() const&& noexcept(is_nothrow_right_value_movable)
            -> const right_type&&
         {
            return std::move(*r_pointer());
         }

      private:
         constexpr auto l_pointer() noexcept -> left_type*
         {
            return reinterpret_cast<left_type*>(m_bytes.data()); // NOLINT
         }
         constexpr auto l_pointer() const noexcept -> const left_type*
         {
            return reinterpret_cast<const left_type*>(m_bytes.data()); // NOLINT
         }

         constexpr auto r_pointer() noexcept -> right_type*
         {
            return reinterpret_cast<right_type*>(m_bytes.data()); // NOLINT
         }
         constexpr auto r_pointer() const noexcept -> const right_type*
         {
            return reinterpret_cast<const right_type*>(m_bytes.data()); // NOLINT
         }

      private:
         alignas(detail::max(alignof(left_type), alignof(right_type))) storage_type m_bytes{};
         bool m_is_right{false};
      };

      template <class first_, class second_>
      class storage<first_, second_, std::enable_if_t<trivial<first_> && trivial<second_>>>
      {
      public:
         using left_type = first_;
         using right_type = second_;

      private:
         using storage_type =
            std::array<std::byte, detail::max(sizeof(left_type), sizeof(right_type))>;

      public:
         constexpr storage() noexcept { std::construct_at(std::addressof(left()), left_type{}); };
         constexpr storage(const left_t<left_type>& l) noexcept
         {
            std::construct_at(std::addressof(left()), l.value);
         }
         constexpr storage(left_t<left_type>&& l) noexcept
         {
            std::construct_at(std::addressof(left()), std::move(l.value));
         }
         constexpr storage(const right_t<right_type>& r) noexcept : m_is_right{true}
         {
            std::construct_at(std::addressof(right()), r.value);
         }
         constexpr storage(right_t<right_type>&& r) noexcept : m_is_right{true}
         {
            std::construct_at(std::addressof(right()), std::move(r.value));
         }

         constexpr auto left() & noexcept -> left_type& { return *l_pointer(); }
         constexpr auto left() const& noexcept -> const left_type& { return *l_pointer(); }
         constexpr auto left() && noexcept -> left_type&& { return std::move(*l_pointer()); }
         constexpr auto left() const&& noexcept -> const left_type&&
         {
            return std::move(*l_pointer());
         }

         constexpr auto right() & noexcept -> right_type& { return *r_pointer(); }
         constexpr auto right() const& noexcept -> const right_type& { return *r_pointer(); }
         constexpr auto right() && noexcept -> right_type&& { return std::move(*r_pointer()); }
         constexpr auto right() const&& noexcept -> const right_type&&
         {
            return std::move(*r_pointer());
         }

         [[nodiscard]] constexpr auto is_right() const noexcept -> bool { return m_is_right; };

      private:
         constexpr auto l_pointer() noexcept -> left_type*
         {
            return reinterpret_cast<left_type*>(m_bytes.data()); // NOLINT
         }
         constexpr auto l_pointer() const noexcept -> const left_type*
         {
            return reinterpret_cast<const left_type*>(m_bytes.data()); // NOLINT
         }

         constexpr auto r_pointer() noexcept -> right_type*
         {
            return reinterpret_cast<right_type*>(m_bytes.data()); // NOLINT
         }
         constexpr auto r_pointer() const noexcept -> const right_type*
         {
            return reinterpret_cast<const right_type*>(m_bytes.data()); // NOLINT
         }

      private:
         alignas(detail::max(alignof(left_type), alignof(right_type))) storage_type m_bytes{};
         bool m_is_right{false};
      };

   public:
      using left_type = left_;
      using right_type = right_;

   private:
      using storage_type = storage<left_type, right_type>;

      static inline constexpr bool is_nothrow_left_copy_constructible =
         std::is_nothrow_constructible_v<storage_type, left_t<left_type>>;

      static inline constexpr bool is_nothrow_left_move_constructible =
         std::is_nothrow_constructible_v<storage_type, left_t<left_type>&&>;

      static inline constexpr bool copyable = std::copyable<left_type> && std::copyable<right_type>;
      static inline constexpr bool movable = std::movable<left_type> && std::movable<right_type>;

      template <class any_>
      using left_map_result = std::invoke_result_t<any_, left_type>;

      template <class fun_>
      using left_map_either = either<left_map_result<fun_>, right_type>;

      template <class any_>
      using right_map_result = std::invoke_result_t<any_, right_type>;

      template <class fun_>
      using right_map_either = either<left_type, right_map_result<fun_>>;

      template <class left_fun, class right_fun>
      using join_result =
         std::common_type_t<left_map_result<left_fun>, right_map_result<right_fun>>;

   public:
      constexpr either(const left_t<left_type>& left) noexcept(is_nothrow_left_copy_constructible) :
         m_storage{left}
      {}
      constexpr either(left_t<left_type>&& left) noexcept(is_nothrow_left_move_constructible) :
         m_storage{std::move(left)}
      {}
      constexpr either(const right_t<right_type>& right) : m_storage{right} {}
      constexpr either(right_t<right_type>&& right) : m_storage{std::move(right)} {}

      [[nodiscard]] constexpr auto is_right() const -> bool { return m_storage.is_right(); }
      constexpr operator bool() const { return is_right(); }

      constexpr auto left() const& -> maybe<left_type> requires copyable
      {
         return !is_right() ? make_maybe(m_storage.left()) : none;
      }
      constexpr auto left() & -> maybe<left_type> requires copyable
      {
         return !is_right() ? make_maybe(m_storage.left()) : none;
      }
      constexpr auto left() const&& -> maybe<left_type> requires movable
      {
         return !is_right() ? make_maybe(std::move(m_storage.left())) : none;
      }
      constexpr auto left() && -> maybe<left_type> requires movable
      {
         return !is_right() ? make_maybe(std::move(m_storage.left())) : none;
      }

      constexpr auto right() const& -> maybe<right_type> requires copyable
      {
         return !is_right() ? none : make_maybe(m_storage.right());
      }
      constexpr auto right() & -> maybe<right_type> requires copyable
      {
         return !is_right() ? none : make_maybe(m_storage.right());
      }
      constexpr auto right() const&& -> maybe<right_type> requires movable
      {
         return !is_right() ? none : make_maybe(std::move(m_storage.right()));
      }
      constexpr auto right() && -> maybe<right_type> requires movable
      {
         return !is_right() ? none : maybe_maybe(std::move(m_storage.right()));
      }

      constexpr auto
      left_map(const std::invocable<left_type> auto& fun) const& -> left_map_either<decltype(fun)>
      {
         if (!is_right())
         {
            return make_left(std::invoke(fun, m_storage.left()));
         }
         else
         {
            return make_right(m_storage.right());
         }
      }
      constexpr auto
      left_map(const std::invocable<left_type> auto& fun) & -> left_map_either<decltype(fun)>
      {
         if (!is_right())
         {
            return make_left(std::invoke(fun, m_storage.left()));
         }
         else
         {
            return make_right(m_storage.right());
         }
      }
      constexpr auto
      left_map(const std::invocable<left_type> auto& fun) const&& -> left_map_either<decltype(fun)>
      {
         if (!is_right())
         {
            return make_left(std::invoke(fun, std::move(m_storage.left())));
         }
         else
         {
            return make_right(std::move(m_storage.right()));
         }
      }
      constexpr auto
      left_map(const std::invocable<left_type> auto& fun) && -> left_map_either<decltype(fun)>
      {
         if (!is_right())
         {
            return make_left(std::invoke(fun, std::move(m_storage.left())));
         }
         else
         {
            return make_right(std::move(m_storage.right()));
         }
      }

      constexpr auto right_map(
         const std::invocable<right_type> auto& fun) const& -> right_map_either<decltype(fun)>
      {
         if (!is_right())
         {
            return make_left(m_storage.left());
         }
         else
         {
            return make_right(std::invoke(fun, m_storage.right()));
         }
      }
      constexpr auto
      right_map(const std::invocable<right_type> auto& fun) & -> right_map_either<decltype(fun)>
      {
         if (!is_right())
         {
            return make_left(m_storage.left());
         }
         else
         {
            return make_right(std::invoke(fun, m_storage.right()));
         }
      }
      constexpr auto right_map(
         const std::invocable<right_type> auto& fun) const&& -> right_map_either<decltype(fun)>
      {
         if (!is_right())
         {
            return make_left(std::move(m_storage.left()));
         }
         else
         {
            return make_right(std::invoke(fun, std::move(m_storage.right())));
         }
      }
      constexpr auto
      right_map(const std::invocable<right_type> auto& fun) && -> right_map_either<decltype(fun)>
      {
         if (!is_right())
         {
            return make_left(std::move(m_storage.left()));
         }
         else
         {
            return make_right(std::invoke(fun, std::move(m_storage.right())));
         }
      }

      template <class inner_left_ = left_type, class inner_right_ = right_type>
      constexpr auto
      join() const& -> std::common_type_t<inner_left_, inner_right_> requires copyable
      {
         return !is_right() ? m_storage.left() : m_storage.right();
      }
      template <class inner_left_ = left_type, class inner_right_ = right_type>
      constexpr auto join() & -> std::common_type_t<inner_left_, inner_right_> requires copyable
      {
         return !is_right() ? m_storage.left() : m_storage.right();
      }

      template <class inner_left_ = left_type, class inner_right_ = right_type>
      constexpr auto
      join() const&& -> std::common_type_t<inner_left_, inner_right_> requires movable
      {
         return !is_right() ? std::move(m_storage.left()) : std::move(m_storage.right());
      }
      template <class inner_left_ = left_type, class inner_right_ = right_type>
      constexpr auto join() && -> std::common_type_t<inner_left_, inner_right_> requires movable
      {
         return !is_right() ? std::move(m_storage.left()) : std::move(m_storage.right());
      }

      constexpr auto join(const std::invocable<left_type> auto& l_fun,
                          const std::invocable<right_type> auto& r_fun)
         const& -> join_result<decltype(l_fun), decltype(r_fun)>
      {
         return !is_right() ? std::invoke(l_fun, m_storage.left())
                            : std::invoke(r_fun, m_storage.right());
      }
      constexpr auto join(const std::invocable<left_type> auto& l_fun,
                          const std::invocable<right_type> auto&
                             r_fun) & -> join_result<decltype(l_fun), decltype(r_fun)>
      {
         return !is_right() ? std::invoke(l_fun, m_storage.left())
                            : std::invoke(r_fun, m_storage.right());
      }
      constexpr auto join(const std::invocable<left_type> auto& l_fun,
                          const std::invocable<right_type> auto& r_fun)
         const&& -> join_result<decltype(l_fun), decltype(r_fun)>
      {
         return !is_right() ? std::invoke(l_fun, std::move(m_storage.left()))
                            : std::invoke(r_fun, std::move(m_storage.right()));
      }

      constexpr auto join(const std::invocable<left_type> auto& l_fun,
                          const std::invocable<right_type> auto&
                             r_fun) && -> join_result<decltype(l_fun), decltype(r_fun)>
      {
         return !is_right() ? std::invoke(l_fun, std::move(m_storage.left()))
                            : std::invoke(r_fun, std::move(m_storage.right()));
      }

   private:
      storage<left_type, right_type> m_storage{};

   public:
      constexpr auto left_flat_map(const std::invocable<left_type> auto& fun) const& -> decltype(
         detail::ensure_either_right(std::invoke(fun, m_storage.left()), m_storage.right()))
      {
         if (!is_right())
         {
            return std::invoke(fun, m_storage.left());
         }
         else
         {
            return make_right(m_storage.right());
         }
      }
      constexpr auto left_flat_map(const std::invocable<left_type> auto& fun) & -> decltype(
         detail::ensure_either_right(std::invoke(fun, m_storage.left()), m_storage.right()))
      {
         if (!is_right())
         {
            return std::invoke(fun, m_storage.left());
         }
         else
         {
            return make_right(m_storage.right());
         }
      }
      constexpr auto left_flat_map(const std::invocable<left_type> auto& fun) const&& -> decltype(
         detail::ensure_either_right(std::invoke(fun, std::move(m_storage.left())),
                                     std::move(m_storage.right())))
      {
         if (!is_right())
         {
            return std::invoke(fun, std::move(m_storage.left()));
         }
         else
         {
            return make_right(std::move(m_storage.right()));
         }
      }
      constexpr auto left_flat_map(const std::invocable<left_type> auto& fun) && -> decltype(
         detail::ensure_either_right(std::invoke(fun, std::move(m_storage.left())),
                                     std::move(m_storage.right())))
      {
         if (!is_right())
         {
            return std::invoke(fun, std::move(m_storage.left()));
         }
         else
         {
            return make_right(std::move(m_storage.right()));
         }
      }

      constexpr auto right_flat_map(const std::invocable<right_type> auto& fun) const& -> decltype(
         detail::ensure_either_left(std::invoke(fun, m_storage.right()), m_storage.left()))
      {
         if (!is_right())
         {
            return make_left(m_storage.left());
         }
         else
         {
            return std::invoke(fun, m_storage.right());
         }
      }
      constexpr auto right_flat_map(const std::invocable<right_type> auto& fun) & -> decltype(
         detail::ensure_either_left(std::invoke(fun, m_storage.right()), m_storage.left()))
      {
         if (!is_right())
         {
            return make_left(m_storage.left());
         }
         else
         {
            return std::invoke(fun, m_storage.right());
         }
      }
      constexpr auto right_flat_map(const std::invocable<right_type> auto& fun) const&& -> decltype(
         detail::ensure_either_left(std::invoke(fun, std::move(m_storage.right())),
                                    std::move(m_storage.left())))
      {
         if (!is_right())
         {
            return make_left(std::move(m_storage.left()));
         }
         else
         {
            return std::invoke(fun, std::move(m_storage.right()));
         }
      }
      constexpr auto right_flat_map(const std::invocable<right_type> auto& fun) && -> decltype(
         detail::ensure_either_left(std::invoke(fun, std::move(m_storage.right())),
                                    std::move(m_storage.left())))
      {
         if (!is_right())
         {
            return make_left(std::move(m_storage.left()));
         }
         else
         {
            return std::invoke(fun, std::move(m_storage.right()));
         }
      }
   };
} // namespace monad
