import json
from pathlib import Path

import attribute_controller as ac
import cadwork
import element_controller as ec
import geometry_controller as gc

OUTPUT_PATH = Path.home() / "Downloads" / "elements_export.jsonl"


def point_to_list(point: cadwork.point_3d) -> list[float]:
    return [point.x, point.y, point.z]


def build_element_row(element_id: int) -> dict:
    return {
        "guid": ec.get_element_cadwork_guid(element_id),
        "name": ac.get_name(element_id),
        "p1": point_to_list(gc.get_p1(element_id)),
        "p2": point_to_list(gc.get_p2(element_id)),
        "p3": point_to_list(gc.get_p3(element_id)),
        "xl": point_to_list(gc.get_xl(element_id)),
        "yl": point_to_list(gc.get_yl(element_id)),
        "zl": point_to_list(gc.get_zl(element_id)),
        "length": gc.get_length(element_id),
        "width": gc.get_width(element_id),
        "height": gc.get_height(element_id),
    }


def iter_all_element_rows():
    for element_id in ec.get_all_identifiable_element_ids():
        yield build_element_row(element_id)


def write_jsonl(rows, output_path: Path) -> int:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    row_count = 0
    with output_path.open("w", encoding="utf-8") as jsonl_file:
        for row in rows:
            jsonl_file.write(json.dumps(row, ensure_ascii=False) + "\n")
            row_count += 1
    return row_count


def main():
    row_count = write_jsonl(iter_all_element_rows(), OUTPUT_PATH)
    print(f"Wrote {row_count} elements to {OUTPUT_PATH}")


if __name__ == "__main__":
    main()
