/**
 * @file ref.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Saturday, 24th of june 2021
 * @brief Contains the ref type alias.
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBREGLISSE_UTILS_REF_HPP
#define LIBREGLISSE_UTILS_REF_HPP

#include <functional>

namespace reglisse
{
   /**
    * @brief Helper alias for std::reference_wrapper.
    */
   template <typename Any>
   using ref = std::reference_wrapper<Any>;

   /**
    * @brief Helper alias for std::reference_wrapper to a const value.
    */
   template <typename T>
   using const_ref = std::reference_wrapper<std::add_const_t<std::remove_reference_t<T>>>;

   /**
    * @brief Helper alias for std::reference_wrapper to a mutable value.
    */
   template <typename T>
   using mut_ref = std::reference_wrapper<std::remove_const_t<std::remove_reference_t<T>>>;
} // namespace reglisse

#endif // LIBREGLISSE_UTILS_REF_HPP
