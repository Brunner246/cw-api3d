#pragma once

#include "src/application/ElementSnapshot.h"

#include <concepts>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace cw_api3d::adapters::driven::cadwork
{

  /// @brief The narrowed host contracts the cadwork adapters translate from.
  /// Every adapter is constrained on one of the *Source concepts below rather than on a concrete
  /// CwAPI3D interface, so a test double needs only the handful of methods actually exercised
  /// instead of the whole SDK interface. They live in one header because the string and ID-list
  /// shapes are shared: defining them per adapter previously produced two conflicting
  /// concepts::HostString in this namespace.
  namespace concepts
  {

    /// @brief Host-owned string as the utility controller exposes it: narrow character data.
    /// Deliberately excludes destroy() so adapter bodies cannot reach it.
    template<typename T>
    concept HostNarrowString = requires(T& hostString) {
      { hostString.narrowData() } -> std::convertible_to<const char*>;
    };

    /// @brief Host-owned string as the attribute controller exposes it: wide character data.
    template<typename T>
    concept HostWideString = requires(T& hostString) {
      { hostString.data() } -> std::convertible_to<const wchar_t*>;
    };

    template<typename T>
    concept HostIdList = requires(T& list, std::uint32_t index) {
      { list.count() } -> std::convertible_to<std::uint32_t>;
      { list.at(index) } -> std::convertible_to<application::ElementId>;
    };

    template<typename T>
    concept HostMutableIdList = HostIdList<T> && requires(T& list, application::ElementId id) {
      list.append(id);
    };

    template<typename T>
    concept HostElementType = requires(T& type) {
      { type.isFramedWall() } -> std::convertible_to<bool>;
      { type.isSolidWoodWall() } -> std::convertible_to<bool>;
      { type.isLogWall() } -> std::convertible_to<bool>;
      { type.isWall() } -> std::convertible_to<bool>;
      { type.isRectangularBeam() } -> std::convertible_to<bool>;
      { type.isCircularBeam() } -> std::convertible_to<bool>;
      { type.isPanel() } -> std::convertible_to<bool>;
      { type.isOpening() } -> std::convertible_to<bool>;
    };

    /// @brief Minimal host contract the utility adapter needs: a plugin path as a host string.
    template<typename T>
    concept PluginPathSource = requires(T& controller) {
      requires HostNarrowString<std::remove_pointer_t<decltype(controller.getPluginPath())>>;
    };

    template<typename T>
    concept ElementCatalogSource = requires(T& host, application::ElementId id) {
      requires HostIdList<std::remove_pointer_t<decltype(host.getActiveIdentifiableElementIDs())>>;
      requires HostIdList<std::remove_pointer_t<decltype(host.getAllIdentifiableElementIDs())>>;
      requires HostWideString<std::remove_pointer_t<decltype(host.getName(id))>>;
      requires HostWideString<std::remove_pointer_t<decltype(host.getElementMaterialName(id))>>;
      requires HostElementType<std::remove_pointer_t<decltype(host.getElementType(id))>>;
      { host.getLength(id) } -> std::convertible_to<double>;
      { host.getWidth(id) } -> std::convertible_to<double>;
      { host.getHeight(id) } -> std::convertible_to<double>;
      { host.getVolume(id) } -> std::convertible_to<double>;
    };

    template<typename T>
    concept ElementActivationSource = requires(T& host) {
      requires HostMutableIdList<std::remove_pointer_t<decltype(host.createEmptyElementIDList())>>;
      host.setActive(host.createEmptyElementIDList());
    };

  } // namespace concepts

  namespace detail
  {

    [[nodiscard]] constexpr std::string_view trimWhitespace(std::string_view text) noexcept
    {
      constexpr auto isSpace = [](const char c) noexcept {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
      };
      while (!text.empty() && isSpace(text.front()))
      {
        text.remove_prefix(1);
      }
      while (!text.empty() && isSpace(text.back()))
      {
        text.remove_suffix(1);
      }
      return text;
    }

  } // namespace detail

} // namespace cw_api3d::adapters::driven::cadwork
