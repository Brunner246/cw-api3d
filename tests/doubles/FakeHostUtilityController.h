#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace cw_api3d::tests::doubles
{

  /// @brief Host string double satisfying the adapter's HostString contract.
  /// destroy() records the call instead of freeing, so tests can assert the adapter never invokes it.
  class FakeHostString final
  {
  public:
    explicit FakeHostString(std::string narrow = "") noexcept
      : mNarrow(std::move(narrow))
    {
    }

    const char* narrowData() noexcept
    {
      return mReturnNullNarrowData ? nullptr : mNarrow.c_str();
    }

    void destroy() noexcept
    {
      mDestroyCalled = true;
    }

    void setNarrow(std::string narrow)
    {
      mNarrow = std::move(narrow);
    }

    void setReturnNullNarrowData(const bool returnNull) noexcept
    {
      mReturnNullNarrowData = returnNull;
    }

    [[nodiscard]] bool destroyCalled() const noexcept
    {
      return mDestroyCalled;
    }

  private:
    std::string mNarrow;
    bool mReturnNullNarrowData{false};
    bool mDestroyCalled{false};
  };

  /// @brief Host utility controller double satisfying the adapter's PluginPathSource contract.
  /// Deliberately does NOT inherit ICwAPI3DUtilityController — the adapter is constrained on a
  /// concept, so this double needs only the one method it actually exercises.
  class FakeHostUtilityController final
  {
  public:
    enum class Behaviour
    {
      ReturnsString,
      ReturnsNullptr,
      Throws
    };

    FakeHostUtilityController() = default;
    explicit FakeHostUtilityController(std::string pluginPath) noexcept
      : mString(std::move(pluginPath))
    {
    }

    FakeHostString* getPluginPath()
    {
      ++mCallCount;
      switch (mBehaviour)
      {
        case Behaviour::ReturnsNullptr:
          return nullptr;
        case Behaviour::Throws:
          throw std::runtime_error("Simulated host exception");
        case Behaviour::ReturnsString:
          break;
      }
      return &mString;
    }

    void setBehaviour(const Behaviour behaviour) noexcept
    {
      mBehaviour = behaviour;
    }

    [[nodiscard]] FakeHostString& stringDouble() noexcept
    {
      return mString;
    }

    [[nodiscard]] int callCount() const noexcept
    {
      return mCallCount;
    }

  private:
    FakeHostString mString{""};
    Behaviour mBehaviour{Behaviour::ReturnsString};
    int mCallCount{0};
  };

} // namespace cw_api3d::tests::doubles
