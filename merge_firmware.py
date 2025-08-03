Import("env")
import subprocess
import os


def merge_firmware(source, target, env):
    firmware_dir = env.subst("$BUILD_DIR")
    platformio_dir = env.subst("$PLATFORMIO_CORE_DIR")

    # Correct path to boot_app0.bin
    boot_app0_path = f"{platformio_dir}/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin"

    # Check if boot_app0.bin exists
    if not os.path.exists(boot_app0_path):
        print(f"Warning: boot_app0.bin not found at {boot_app0_path}")
        print("Searching for boot_app0.bin...")
        # Alternative search paths
        alt_paths = [
            f"{platformio_dir}/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin",
            f"{firmware_dir}/boot_app0.bin",
            os.path.expanduser(
                "~/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin"
            ),
        ]
        for path in alt_paths:
            if os.path.exists(path):
                boot_app0_path = path
                print(f"Found boot_app0.bin at: {path}")
                break

    # Build the command with correct syntax (hyphens instead of underscores)
    cmd = [
        "python",
        "-m",
        "esptool",
        "--chip",
        "esp32c3",
        "merge-bin",
        "-o",
        f"{firmware_dir}/merged_firmware.bin",
        "--flash-mode",
        "dio",
        "--flash-freq",
        "80m",
        "--flash-size",
        "4MB",
        "0x0000",
        f"{firmware_dir}/bootloader.bin",
        "0x8000",
        f"{firmware_dir}/partitions.bin",
        "0xe000",
        boot_app0_path,
        "0x10000",
        f"{firmware_dir}/firmware.bin",
    ]

    # Execute the command
    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print("Merged firmware created successfully!")
        print(f"Output: {firmware_dir}/merged_firmware.bin")
        if result.stdout:
            print(f"esptool output: {result.stdout}")
    except subprocess.CalledProcessError as e:
        print(f"Error creating merged firmware: {e}")
        print(f"stdout: {e.stdout}")
        print(f"stderr: {e.stderr}")


env.AddPostAction("$BUILD_DIR/firmware.bin", merge_firmware)
