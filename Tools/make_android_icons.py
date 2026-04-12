# require : pip install Pillow
# arch: sudo pacman -S python-pillow
# example usage: python make_android_icons.py iconbase.png ./
import os
import sys
from PIL import Image

def generate_icons(input_path, output_dir):
    # Standard Launcher Icons (48dp base)
    launcher_sizes = {
        'mipmap-mdpi': 48,
        'mipmap-hdpi': 72,
        'mipmap-xhdpi': 96,
        'mipmap-xxhdpi': 144,
        'mipmap-xxxhdpi': 192
    }

    # Adaptive Icons (108dp base for foreground layer)
    adaptive_sizes = {
        'mipmap-mdpi': 108,
        'mipmap-hdpi': 162,
        'mipmap-xhdpi': 216,
        'mipmap-xxhdpi': 324,
        'mipmap-xxxhdpi': 432
    }

    try:
        if not os.path.exists(input_path):
            print(f"Error: File '{input_path}' not found.")
            return

        with Image.open(input_path) as img:
            # Ensure RGBA for proper transparency in PNGs
            if img.mode != 'RGBA':
                img = img.convert('RGBA')

            for folder, l_size in launcher_sizes.items():
                target_path = os.path.join(output_dir, folder)
                os.makedirs(target_path, exist_ok=True)

                # 1. Classic & Round Icons (PNG only)
                icon = img.resize((l_size, l_size), Image.Resampling.LANCZOS)
                for name in ['ic_launcher', 'ic_launcher_round']:
                    icon.save(os.path.join(target_path, f"{name}.png"))

                # 2. Adaptive Icons (Foreground Layer - PNG only)
                a_size = adaptive_sizes[folder]
                adaptive_icon = img.resize((a_size, a_size), Image.Resampling.LANCZOS)
                adaptive_icon.save(os.path.join(target_path, "ic_launcher_foreground.png"))

            # 3. Play Store High-Res Icon
            playstore_path = os.path.join(output_dir, "playstore_icon.png")
            img.resize((512, 512), Image.Resampling.LANCZOS).save(playstore_path)

            print(f"✅ PNG Icons successfully created in '{output_dir}'.")

    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python make_android_icons.py <input.png> <output_directory>")
    else:
        generate_icons(sys.argv[1], sys.argv[2])
