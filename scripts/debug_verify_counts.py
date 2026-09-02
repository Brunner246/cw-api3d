import json
from pathlib import Path

out = str(Path.home() / "Downloads" / "verify_counts.json")
result = {}
try:
    import element_controller as ec
    try:
        ids = list(ec.get_all_identifiable_element_ids())
        result["all_count"] = len(ids)
        result["all_ids_sample"] = ids[:50]
    except Exception as e:
        result["all_error"] = repr(e)
    try:
        result["visible_count"] = len(ec.get_visible_identifiable_element_ids())
    except Exception as e:
        result["visible_error"] = repr(e)
    try:
        result["active_count"] = len(ec.get_active_identifiable_element_ids())
    except Exception as e:
        result["active_error"] = repr(e)
except Exception as e:
    result["fatal_error"] = repr(e)

with open(out, "w") as f:
    json.dump(result, f, indent=2)
