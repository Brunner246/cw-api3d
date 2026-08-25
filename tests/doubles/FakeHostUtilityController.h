#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace cw_api3d::tests::doubles
{

  /// @brief Host string double satisfying the adapter's HostString contract.
  /// destroy() records the call instead of freeing, so tests can assert the adapter never invokes it.
  class FakeHostString final
  {
  public:
    explicit FakeHostString(std::string narrow = "")
      : mNarrow(std::move(narrow))
      , mWide(wideFromUtf8(mNarrow))
    {
    }

    const wchar_t* data() noexcept
    {
      return mReturnNullData ? nullptr : mWide.c_str();
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
      mWide = wideFromUtf8(mNarrow);
    }

    void setWide(std::wstring wide)
    {
      mWide = std::move(wide);
    }

    void setReturnNullData(const bool returnNull) noexcept
    {
      mReturnNullData = returnNull;
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
    [[nodiscard]] static std::wstring wideFromUtf8(const std::string_view utf8)
    {
      std::wstring wide;
      std::size_t index = 0;
      while (index < utf8.size())
      {
        const auto lead = static_cast<unsigned char>(utf8[index]);
        char32_t codePoint = 0;
        std::size_t need = 1;
        if (lead < 0x80)
        {
          codePoint = lead;
        }
        else if ((lead & 0xE0) == 0xC0)
        {
          need = 2;
          codePoint = lead & 0x1F;
        }
        else if ((lead & 0xF0) == 0xE0)
        {
          need = 3;
          codePoint = lead & 0x0F;
        }
        else if ((lead & 0xF8) == 0xF0)
        {
          need = 4;
          codePoint = lead & 0x07;
        }
        else
        {
          ++index;
          continue;
        }
        if (index + need > utf8.size())
        {
          break;
        }
        for (std::size_t offset = 1; offset < need; ++offset)
        {
          codePoint = (codePoint << 6) | (static_cast<unsigned char>(utf8[index + offset]) & 0x3F);
        }
        index += need;
        if (codePoint <= 0xFFFF)
        {
          wide.push_back(static_cast<wchar_t>(codePoint));
          continue;
        }
        const auto payload = codePoint - 0x10000;
        wide.push_back(static_cast<wchar_t>(0xD800 + (payload >> 10)));
        wide.push_back(static_cast<wchar_t>(0xDC00 + (payload & 0x3FF)));
      }
      return wide;
    }

    std::string mNarrow;
    std::wstring mWide;
    bool mReturnNullData{false};
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
    explicit FakeHostUtilityController(std::string pluginPath)
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
