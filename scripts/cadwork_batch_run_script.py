import cadwork
import utility_controller as uc

print(f"Build number: {uc.get_3d_build()}")
with open(f"{uc.get_3d_file_name()}_log.txt", "w") as f:
    f.write(f"Build number: {uc.get_3d_build()}\n")
    f.write(f"File name: {uc.get_3d_file_name()}\n")
    f.write(f"File path: {uc.get_3d_file_path()}\n")