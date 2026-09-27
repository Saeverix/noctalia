#include "shell/bar/widgets/workspaces_widget.h"
#include "tests/test_check.h"

#include <cmath>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

class WorkspacesWidgetTestAccess {
public:
  static std::string tooltip(std::string name, const std::string& label, bool showLabel) {
    return WorkspacesWidget::workspaceTooltipText(Workspace{.name = std::move(name)}, label, showLabel);
  }

  static float snappedSize(float x, float width) { return WorkspacesWidget::snappedSpan(x, width).size; }

  // Lays out pills the way updateItemFlowPositions() does and checks that every snapped gap is exactly `gap`.
  static bool uniformGaps(const std::vector<float>& widths, float gap) {
    float cursor = 0.0F;
    float previousEnd = 0.0F;
    for (std::size_t i = 0; i < widths.size(); ++i) {
      const auto span = WorkspacesWidget::snappedSpan(cursor, widths[i]);
      if (span.start != std::round(span.start) || span.size != std::round(span.size)) {
        return false;
      }
      if (i > 0 && span.start - previousEnd != gap) {
        return false;
      }
      previousEnd = span.start + span.size;
      cursor += widths[i] + gap;
    }
    return true;
  }
};

int main() {
  // A name cut short by max_label_chars is recoverable on hover.
  TEST_CHECK(WorkspacesWidgetTestAccess::tooltip("client portal", "cli", true) == "client portal");
  // A pill that already spells out the whole name has nothing to add.
  TEST_CHECK(WorkspacesWidgetTestAccess::tooltip("web", "web", true).empty());
  // label_source = "id" labels a named workspace by number, so the name is only reachable on hover.
  TEST_CHECK(WorkspacesWidgetTestAccess::tooltip("client portal", "3", true) == "client portal");
  // A hidden label shows nothing, even when its text would have matched the name.
  TEST_CHECK(WorkspacesWidgetTestAccess::tooltip("web", "web", false) == "web");
  // Unnamed workspaces have no tooltip, never an empty one.
  TEST_CHECK(WorkspacesWidgetTestAccess::tooltip("", "3", true).empty());
  TEST_CHECK(WorkspacesWidgetTestAccess::tooltip("", "", false).empty());

  // A fractional active pill (16 * 2.2) must not shift the gaps after it, wherever it sits.
  for (std::size_t active = 0; active < 9; ++active) {
    std::vector<float> widths(9, 16.0F);
    widths[active] = 35.2F;
    TEST_CHECK(WorkspacesWidgetTestAccess::uniformGaps(widths, 4.0F));
  }
  // Mid-animation: one pill shrinking while the next grows keeps every gap exact.
  for (float t = 0.0F; t <= 1.0F; t += 0.05F) {
    std::vector<float> widths(9, 16.0F);
    widths[0] = std::lerp(35.0F, 16.0F, t);
    widths[1] = std::lerp(16.0F, 35.0F, t);
    TEST_CHECK(WorkspacesWidgetTestAccess::uniformGaps(widths, 4.0F));
  }
  // Snapping never produces a negative size.
  TEST_CHECK(WorkspacesWidgetTestAccess::snappedSize(10.6F, 0.0F) == 0.0F);

  return 0;
}
