"""Print a one-block identity dump for the current selection.

Run it against a live 3d.exe with elements selected:

    ci_start.exe <model> /EXE=<exe_dir> /RUNPROGRAM=<absolute path to this file>

With nothing selected the script falls back to every identifiable element in the model.
"""

import attribute_controller
import element_controller


def type_labels(element_type):
    # element_type has dozens of is_*() predicates and no name/string accessor, so the true
    # ones are discovered by reflection rather than hand-maintaining the predicate list here.
    predicate_names = (name for name in dir(element_type) if name.startswith("is_")) # REFLECTION =) 
    return [name for name in predicate_names if getattr(element_type, name)()]


def describe(element_id):
    element_type = attribute_controller.get_element_type(element_id)
    return "\n".join([
        f"Element {element_id}",
        f"  name:    {attribute_controller.get_name(element_id)}",
        f"  group:   {attribute_controller.get_group(element_id)}",
        f"  comment: {attribute_controller.get_comment(element_id)}",
        f"  type:    {', '.join(type_labels(element_type))}",
        f"  guid:    {element_controller.get_element_cadwork_guid(element_id)}",
    ])


def main():
    element_ids = element_controller.get_active_identifiable_element_ids() or element_controller.get_all_identifiable_element_ids()

    if not element_ids:
        print("No elements in the model - nothing to report.")
        return

    print("\n\n".join(map(describe, element_ids)))


main()
