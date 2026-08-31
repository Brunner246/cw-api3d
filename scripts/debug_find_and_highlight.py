"""Find elements by a name/group substring and highlight them in the viewport.

Set CW_DEBUG_SEARCH before launching, or edit SEARCH_TEXT below. Run against a live 3d.exe:

    ci_start.exe <model> /EXE=<exe_dir> /RUNPROGRAM=<absolute path to this file>
"""

import os

import attribute_controller
import element_controller
import visualization_controller

SEARCH_TEXT = os.environ.get("CW_DEBUG_SEARCH", "beam")
HIGHLIGHT_COLOR = 5


def matches(element_id, needle):
    needle = needle.lower()
    return (needle in attribute_controller.get_name(element_id).lower()
            or needle in attribute_controller.get_group(element_id).lower())


def main():
    element_ids = element_controller.get_all_identifiable_element_ids()
    found = [element_id for element_id in element_ids if matches(element_id, SEARCH_TEXT)]

    if not found:
        print(f"No elements match '{SEARCH_TEXT}'.")
        return

    for element_id in found:
        print(f"{element_id}: {attribute_controller.get_name(element_id)} "
              f"({attribute_controller.get_group(element_id)})")

    visualization_controller.hide_all_elements()
    visualization_controller.set_visible(found)
    visualization_controller.set_color(found, HIGHLIGHT_COLOR)
    visualization_controller.zoom_active_elements()


main()
