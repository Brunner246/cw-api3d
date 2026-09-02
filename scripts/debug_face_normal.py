import cadwork
import element_controller as ec
import geometry_controller as gc


from typing import Iterator, NamedTuple

EPSILON = 1e-6


class FaceNormal(NamedTuple):
    idx: int
    normal: "cadwork.point_3d"


def is_invalid_normal(normal: "cadwork.point_3d", epsilon: float = EPSILON) -> bool:
    return normal.magnitude() < epsilon


def describe(face: FaceNormal) -> str:
    status = "invalid" if is_invalid_normal(face.normal) else "valid"
    return f"Face {face.idx} has a {status} normal vector: {face.normal}"


def indexed_normals(facets: "cadwork.facet_list") -> Iterator[FaceNormal]:
    return (
        FaceNormal(idx, facets.get_normal_vector(idx)) for idx in range(len(facets))
    )


def all_faces(
    gc, elements: list
) -> Iterator[tuple[int, "cadwork.facet_list", FaceNormal]]:
    """Lazily yields (element_id, facets, face) triples across all elements."""
    for element in elements:
        facets = gc.get_element_facets(element)
        for face in indexed_normals(facets):
            yield element, facets, face


def report_and_patch_invalid_faces_for_elements(ec, gc, elements: list) -> None:
    current_element = None
    for element, facets, face in all_faces(gc, elements):
        if element != current_element:
            print(f"Analyzing element {element} for invalid faces...")
            current_element = element

        print(f"[Element {element}] {describe(face)}")

        if is_invalid_normal(face.normal):
            ec.create_surface(facets.at(face.idx))



if __name__ == "__main__":
    elements: list = ec.get_active_identifiable_element_ids()
    report_and_patch_invalid_faces_for_elements(ec, gc, elements)
