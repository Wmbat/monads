/**
 * @file result.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Monday, 7th of june 2021
 * @brief Contains everything related to the result monad
 * @copyright Copyright (C) 2021 wmbat.
 */

#pragma once

#if defined(LIBREGLISSE_USE_EXCEPTIONS)
#   include <libreglisse/detail/invalid_access_exception.hpp>
#else
#   include <cassert>
#endif // defined (LIBREGLISSE_USE_EXCEPTIONS)

#include <functional>
#include <memory>

namespace reglisse::detail
{
   inline void handle_invalid_value_result_access(bool check)
   {
#if defined(LIBREGLISSE_USE_EXCEPTIONS)
      if (!check)
      {
         throw invalid_access_exception("result currently holds an error");
      }
#else
      assert(check && "result currently holds an error"); // NOLINT
#endif // defined(LIBREGLISSE_USE_EXCEPTIONS)
   }

   inline void handle_invalid_error_result_access(bool check)
   {
#if defined(LIBREGLISSE_USE_EXCEPTIONS)
      if (!check)
      {
         throw invalid_access_exception("result currently holds an value");
      }
#else
      assert(check && "result currently holds a value");  // NOLINT
#endif // defined(LIBREGLISSE_USE_EXCEPTIONS)
   }

   template <typename Fun, typename T>
   concept ensure_result = std::invocable<Fun, T> and requires
   {
      {std::invoke_result_t<Fun, T>::value_type};
      {std::invoke_result_t<Fun, T>::error_type};
   };

   template <typename Fun, typename ValueType, typename ErrorType>
   concept ensure_value_result = ensure_result<Fun, ValueType> and
      std::same_as<typename std::invoke_result_t<Fun, ValueType>::error_type, ErrorType>;

   template <typename Fun, typename ValueType, typename ErrorType>
   concept ensure_error_result = ensure_result<Fun, ErrorType> and
      std::same_as<typename std::invoke_result_t<Fun, ErrorType>::value_type, ValueType>;

} // namespace reglisse::detail

namespace reglisse
{
   template <std::movable T>
      requires(not std::is_reference_v<T>)
   class ok;

   template <std::movable T>
      requires(not std::is_reference_v<T>)
   class err
   {
   public:
      using value_type = T;

   public:
      explicit constexpr err(value_type&& value) : m_value(std::move(value)) {}

      constexpr auto value() const& noexcept -> const value_type& { return m_value; }
      constexpr auto value() & noexcept -> value_type& { return m_value; }
      constexpr auto value() const&& noexcept -> const value_type { return std::move(m_value); }
      constexpr auto value() && noexcept -> value_type { return std::move(m_value); }

      template <std::equality_comparable_with<value_type> U>
      constexpr auto operator==(const err<U>& rhs) const -> bool
      {
         return value() == rhs.value();
      }

      template <std::equality_comparable_with<value_type> U>
      constexpr auto operator==(const ok<U>& rhs) const -> bool
      {
         return value() == rhs.value();
      }

   private:
      value_type m_value;
   };

   template <std::movable T>
      requires(not std::is_reference_v<T>)
   class ok
   {
   public:
      using value_type = T;

   public:
      explicit constexpr ok(value_type&& value) : m_value(std::move(value)) {}

      constexpr auto value() const& noexcept -> const value_type& { return m_value; }
      constexpr auto value() & noexcept -> value_type& { return m_value; }
      constexpr auto value() const&& noexcept -> const value_type { return std::move(m_value); }
      constexpr auto value() && noexcept -> value_type { return std::move(m_value); }

      template <std::equality_comparable_with<value_type> U>
      constexpr auto operator==(const ok<U>& rhs) const -> bool
      {
         return value() == rhs.value();
      }

      template <std::equality_comparable_with<value_type> U>
      constexpr auto operator==(const err<U>& rhs) const -> bool
      {
         return value() == rhs.value();
      }

   private:
      value_type m_value;
   };

   template <std::movable ValueType, std::movable ErrorType>
      requires(not(std::is_reference_v<ValueType> or std::is_reference_v<ErrorType>))
   class result
   {
   public:
      using value_type = ValueType;
      using error_type = ErrorType;

   private:
      template <class value_fun, class error_fun>
      using join_result = std::common_type_t<std::invoke_result_t<value_fun, value_type>,
                                             std::invoke_result_t<error_fun, error_type>>;

   public:
      constexpr result() = delete;
      constexpr result(ok<value_type>&& value) : m_is_ok(true)
      {
         std::construct_at(&m_value, std::move(value).value()); // NOLINT
      }
      constexpr result(err<error_type>&& error) : m_is_ok(false)
      {
         std::construct_at(&m_error, std::move(error).value()); // NOLINT
      }
      constexpr result(const result& other) : m_is_ok(other.m_is_ok)
      {
         if (is_ok())
         {
            std::construct_at(&m_value, other.borrow()); // NOLINT
         }
         else
         {
            std::construct_at(&m_error, other.borrow_err()); // NOLINT
         }
      }
      constexpr result(result&& other) noexcept : m_is_ok(other.m_is_ok)
      {
         if (is_ok())
         {
            std::construct_at(&m_value, std::move(other).take()); // NOLINT
         }
         else
         {
            std::construct_at(&m_error, std::move(other).take_err()); // NOLINT
         }
      }
      constexpr ~result()
      {
         if (is_ok())
         {
            std::destroy_at(&m_value); // NOLINT
         }
         else
         {
            std::destroy_at(&m_error); // NOLINT
         }
      }

      constexpr auto operator=(const result& rhs) -> result&
      {
         if (this != &rhs)
         {
            if (is_ok())
            {
               std::destroy_at(&m_value); // NOLINT
            }
            else
            {
               std::destroy_at(&m_error); // NOLINT
            }

            m_is_ok = rhs.m_is_ok;

            if (is_ok())
            {
               std::construct_at(&m_value, rhs.borrow()); // NOLINT
            }
            else
            {
               std::construct_at(&m_error, rhs.borrow_err()); // NOLINT
            }
         }

         return *this;
      }
      constexpr auto operator=(result&& rhs) noexcept -> result&
      {
         if (this != &rhs)
         {
            if (is_ok())
            {
               std::destroy_at(&m_value); // NOLINT
            }
            else
            {
               std::destroy_at(&m_error); // NOLINT
            }

            m_is_ok = rhs.m_is_ok;

            if (is_ok())
            {
               std::construct_at(&m_value, std::move(rhs).take()); // NOLINT
            }
            else
            {
               std::construct_at(&m_error, std::move(rhs).take_err()); // NOLINT
            }
         }

         return *this;
      }

      constexpr auto borrow() const& -> const value_type&
      {
         detail::handle_invalid_value_result_access(is_ok());
         return m_value;
      }
      constexpr auto borrow() & -> value_type&
      {
         detail::handle_invalid_value_result_access(is_ok());
         return m_value;
      }
      constexpr auto take() const&& -> value_type
      {
         detail::handle_invalid_value_result_access(is_ok());
         return std::move(m_value);
      }
      constexpr auto take() && -> value_type
      {
         detail::handle_invalid_value_result_access(is_ok());
         return std::move(m_value);
      }

      template <std::convertible_to<value_type> U>
      constexpr auto take_or(U&& other) const&& -> value_type
      {
         if (is_ok())
         {
            return std::move(m_value);
         }

         return std::forward<U>(other);
      }
      template <std::convertible_to<value_type> U>
      constexpr auto take_or(U&& other) && -> value_type
      {
         if (is_ok())
         {
            return std::move(m_value);
         }

         return std::forward<U>(other);
      }

      constexpr auto borrow_err() const& -> const error_type&
      {
         detail::handle_invalid_value_result_access(is_err());
         return m_error;
      }
      constexpr auto borrow_err() & -> error_type&
      {
         detail::handle_invalid_value_result_access(is_err());
         return m_error;
      }
      constexpr auto take_err() const&& -> error_type
      {
         detail::handle_invalid_value_result_access(is_err());
         return std::move(m_error);
      }
      constexpr auto take_err() && -> error_type
      {
         detail::handle_invalid_value_result_access(is_err());
         return std::move(m_error);
      }

      template <std::convertible_to<value_type> U>
      constexpr auto take_err_or(U&& other) const&& -> error_type
      {
         if (is_ok())
         {
            return std::move(m_value);
         }

         return std::forward<U>(other);
      }
      template <std::convertible_to<value_type> U>
      constexpr auto take_err_or(U&& other) && -> error_type
      {
         if (is_err())
         {
            return std::move(m_error);
         }

         return std::forward<U>(other);
      }

      constexpr auto is_ok() const noexcept -> bool { return m_is_ok; }
      constexpr auto is_err() const noexcept -> bool { return not is_ok(); }
      constexpr explicit operator bool() const noexcept { return is_ok(); }

      template <std::invocable<value_type> Fun>
      constexpr auto
      transform(Fun&& fun) const&& -> result<std::invoke_result_t<Fun, value_type&&>, error_type>
      {
         if (is_ok())
         {
            return ok(std::invoke(std::forward<Fun>(fun), take()));
         }

         return err(take_err());
      }
      template <std::invocable<value_type> Fun>
      constexpr auto
      transform(Fun&& fun) && -> result<std::invoke_result_t<Fun, value_type&&>, error_type>
      {
         if (is_ok())
         {
            return ok(std::invoke(std::forward<Fun>(fun), take()));
         }

         return err(take_err());
      }

      template <std::invocable<value_type> Fun>
      constexpr auto transform_err(
         Fun&& err_fun) const&& -> result<value_type, std::invoke_result_t<Fun, error_type>>
      {
         if (is_err())
         {
            return err(std::invoke(std::forward<Fun>(err_fun), take_err()));
         }

         return ok(take());
      }

      template <std::invocable<error_type> Fun>
      constexpr auto
      transform_err(Fun&& err_fun) && -> result<value_type, std::invoke_result_t<Fun, error_type>>
      {
         if (is_err())
         {
            return err(std::invoke(std::forward<Fun>(err_fun), take_err()));
         }

         return ok(take());
      }

      template <detail::ensure_value_result<value_type, error_type> Fun>
      constexpr auto and_then(Fun&& some_fun) const&& -> std::invoke_result_t<Fun, value_type>
      {
         if (is_ok())
         {
            return std::invoke(std::forward<Fun>(some_fun), std::move(m_value)); // NOLINT
         }

         return err(take_err());
      }
      template <detail::ensure_value_result<value_type, error_type> Fun>
      constexpr auto and_then(Fun&& some_fun) && -> std::invoke_result_t<Fun, value_type>
      {
         if (is_ok())
         {
            return std::invoke(std::forward<Fun>(some_fun), std::move(m_value)); // NOLINT
         }

         return err(take_err());
      }

      template <detail::ensure_error_result<value_type, error_type> Fun>
      constexpr auto or_else(Fun&& none_fun) const&& -> std::invoke_result_t<Fun, error_type>
      {
         if (is_ok())
         {
            return ok(take());
         }

         return std::invoke(std::forward<Fun>(none_fun), take_err());
      }
      template <detail::ensure_error_result<value_type, error_type> Fun>
      constexpr auto or_else(Fun&& none_fun) && -> std::invoke_result_t<Fun, error_type>
      {
         if (is_ok())
         {
            return ok(take());
         }

         return std::invoke(std::forward<Fun>(none_fun), take_err());
      }

      template <class inner_value_ = value_type, class inner_error_ = error_type>
      constexpr auto join() const&& -> std::common_type_t<inner_value_, inner_error_>
      {
         return is_ok() ? take() : take_err();
      }
      template <class inner_value_ = value_type, class inner_error_ = error_type>
      constexpr auto join() && -> std::common_type_t<inner_value_, inner_error_>
      {
         return is_ok() ? take() : take_err();
      }

      template <std::invocable<value_type> OkFun, std::invocable<error_type> ErrFun>
      constexpr auto join(OkFun&& ok_fun, ErrFun&& err_fun) const&& -> join_result<OkFun, ErrFun>
      {
         return is_ok() ? std::invoke(std::forward<OkFun>(ok_fun), take())
                        : std::invoke(std::forward<ErrFun>(err_fun), take_err());
      }

      template <std::invocable<value_type> OkFun, std::invocable<error_type> ErrFun>
      constexpr auto join(OkFun&& ok_fun, ErrFun&& err_fun) && -> join_result<OkFun, ErrFun>
      {
         return is_ok() ? std::invoke(std::forward<OkFun>(ok_fun), take())
                        : std::invoke(std::forward<ErrFun>(err_fun), take_err());
      }

   private:
      bool m_is_ok;

      union
      {
         value_type m_value;
         error_type m_error;
      };
   };
} // namespace reglisse
