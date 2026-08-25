#pragma once

#include <QColor>
#include <QString>

#include <array>
#include <cstddef>

namespace cw_api3d::adapters::driving
{

  class CategoricalPalette final
  {
  public:
    CategoricalPalette() = delete;

    [[nodiscard]] static constexpr std::size_t size() noexcept
    {
      return 12;
    }

    [[nodiscard]] static QColor colorAt(const std::size_t index)
    {
      static const std::array<QColor, 12> hues{
        QColor{QStringLiteral("#4E79A7")},
        QColor{QStringLiteral("#F28E2B")},
        QColor{QStringLiteral("#E15759")},
        QColor{QStringLiteral("#76B7B2")},
        QColor{QStringLiteral("#59A14F")},
        QColor{QStringLiteral("#EDC948")},
        QColor{QStringLiteral("#B07AA1")},
        QColor{QStringLiteral("#FF9DA7")},
        QColor{QStringLiteral("#9C755F")},
        QColor{QStringLiteral("#BAB0AC")},
        QColor{QStringLiteral("#86BCB6")},
        QColor{QStringLiteral("#D37295")},
      };
      return hues[index % size()];
    }
  };

} // namespace cw_api3d::adapters::driving
