import cadwork
import element_controller as ec
import attribute_controller as ac

elements = ec.get_all_identifiable_element_ids()

ATTRIBUTE_IDX = 11

ac.set_user_attribute_name(ATTRIBUTE_IDX, "cadwork GUID")

get_guid = ec.get_element_cadwork_guid
set_attribute = ac.set_user_attribute

[set_attribute([element], ATTRIBUTE_IDX, get_guid(element)) for element in elements]
