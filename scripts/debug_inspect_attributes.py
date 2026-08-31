"""Print name/group/comment, material and populated user attributes for the current selection.

Run it against a live 3d.exe with elements selected:

    ci_start.exe <model> /EXE=<exe_dir> /RUNPROGRAM=<absolute path to this file>

With nothing selected the script falls back to every identifiable element in the model.
"""

import attribute_controller
import element_controller

# The API has no way to discover how many user attributes a project defines - raise this if a
# project configures more than the cadwork-standard 20.
USER_ATTRIBUTE_COUNT = 20


def describe_user_attributes(element_id):
    pairs = (
        (attribute_controller.get_user_attribute_name(number),
         attribute_controller.get_user_attribute(element_id, number))
        for number in range(1, USER_ATTRIBUTE_COUNT + 1)
    )
    populated = [f"{name}={value}" for name, value in pairs if value]
    return ", ".join(populated) if populated else "(none)"


def describe(element_id):
    return "\n".join([
        f"Element {element_id}",
        f"  name:     {attribute_controller.get_name(element_id)}",
        f"  group:    {attribute_controller.get_group(element_id)}",
        f"  comment:  {attribute_controller.get_comment(element_id)}",
        f"  material: {attribute_controller.get_element_material_name(element_id)}",
        f"  user attributes: {describe_user_attributes(element_id)}",
    ])


def main():
    element_ids = element_controller.get_active_identifiable_element_ids() or element_controller.get_all_identifiable_element_ids()

    if not element_ids:
        print("No elements in the model - nothing to report.")
        return

    print("\n\n".join(map(describe, element_ids)))


main()
