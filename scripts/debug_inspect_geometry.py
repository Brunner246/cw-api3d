"""Print dimensions, volume, weight, local axes and reference points for the current selection.

Run it against a live 3d.exe with elements selected:

    ci_start.exe <model> /EXE=<exe_dir> /RUNPROGRAM=<absolute path to this file>

With nothing selected the script falls back to every identifiable element in the model.
"""

import element_controller
import geometry_controller


def fmt_point(point):
    return f"({point.x:.1f}, {point.y:.1f}, {point.z:.1f})"


def describe(element_id):
    return "\n".join([
        f"Element {element_id}",
        f"  length x width x height: {geometry_controller.get_length(element_id):.1f} x "
        f"{geometry_controller.get_width(element_id):.1f} x "
        f"{geometry_controller.get_height(element_id):.1f}",
        f"  volume: {geometry_controller.get_volume(element_id):.1f}",
        f"  weight: {geometry_controller.get_weight(element_id):.1f}",
        f"  center of gravity: {fmt_point(geometry_controller.get_center_of_gravity(element_id))}",
        f"  p1: {fmt_point(geometry_controller.get_p1(element_id))}",
        f"  p2: {fmt_point(geometry_controller.get_p2(element_id))}",
        f"  p3: {fmt_point(geometry_controller.get_p3(element_id))}",
        f"  xl: {fmt_point(geometry_controller.get_xl(element_id))}",
        f"  yl: {fmt_point(geometry_controller.get_yl(element_id))}",
        f"  zl: {fmt_point(geometry_controller.get_zl(element_id))}",
    ])


def main():
    element_ids = element_controller.get_active_identifiable_element_ids() or element_controller.get_all_identifiable_element_ids()

    if not element_ids:
        print("No elements in the model - nothing to report.")
        return

    print("\n\n".join(map(describe, element_ids)))


main()
