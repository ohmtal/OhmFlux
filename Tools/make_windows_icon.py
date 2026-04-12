# require : pip install Pillow
# arch: sudo pacman -S python-pillow
# usage example: python make_windows_icon.py mein_logo.png app_icon.ico

import os
import sys
from PIL import Image

def create_windows_ico(input_path, output_path):
    # Standard Windows icon sizes (px)
    icon_sizes = [16, 24, 32, 48, 64, 128, 256]

    try:
        if not os.path.exists(input_path):
            print(f"Error: File '{input_path}' not found.")
            return

        with Image.open(input_path) as img:
            # Ensure RGBA for high-quality transparency
            if img.mode != 'RGBA':
                img = img.convert('RGBA')

            # Create a list of separate image objects for each size
            icon_layers = []
            for size in icon_sizes:
                # Using LANCZOS for the best downscaling result
                resized_img = img.resize((size, size), Image.Resampling.LANCZOS)
                icon_layers.append(resized_img)

            # Sort descending: GIMP and Windows prefer the largest icon as the primary layer
            icon_layers.sort(key=lambda x: x.width, reverse=True)

            # The first image is our base, the others are appended as frames
            main_img = icon_layers[0]
            other_layers = icon_layers[1:]

            # SAVE with PNG compression for the 256px layer (supported since Windows Vista)
            main_img.save(
                output_path,
                format='ICO',
                append_images=other_layers,
                sizes=[(img.width, img.height) for img in icon_layers],
                bitmap_format='png' # This enables compression within the ICO container
            )

            print(f"✅ Optimized Multi-resolution Icon created: {output_path}")
            print(f"Layers included: {[img.width for img in icon_layers]} px (with PNG compression)")

    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python make_windows_icon.py <input.png> <output.ico>")
    else:
        # sys.argv[1] is input, sys.argv[2] is output
        create_windows_ico(sys.argv[1], sys.argv[2])

# import os
# import sys
# from PIL import Image
#
# def create_windows_ico(input_path, output_path):
#     # Standard Windows icon sizes (px)
#     icon_sizes = [16, 32, 48, 64, 128, 256]
#
#     try:
#         if not os.path.exists(input_path):
#             print(f"Error: File '{input_path}' not found.")
#             return
#
#         with Image.open(input_path) as img:
#             # Ensure RGBA for transparency
#             if img.mode != 'RGBA':
#                 img = img.convert('RGBA')
#
#             # Create a list of separate image objects for each size
#             # This is crucial for GIMP to recognize them as layers
#             icon_layers = []
#             for size in icon_sizes:
#                 resized_img = img.resize((size, size), Image.Resampling.LANCZOS)
#                 icon_layers.append(resized_img)
#
#             # We take the first image (usually the largest for best quality)
#             # and append the others as additional frames.
#             # We sort them descending so the largest is the 'main' image.
#             icon_layers.sort(key=lambda x: x.width, reverse=True)
#
#             main_img = icon_layers[0]
#             other_layers = icon_layers[1:]
#
#             main_img.save(
#                 output_path,
#                 format='ICO',
#                 append_images=other_layers,
#                 sizes=[(img.width, img.height) for img in icon_layers]
#             )
#
#             print(f"✅ Multi-resolution Icon created: {output_path}")
#             print(f"Layers included: {[img.width for img in icon_layers]} px")
#
#     except Exception as e:
#         print(f"An error occurred: {e}")
#
# if __name__ == "__main__":
#     if len(sys.argv) != 3:
#         print("Usage: python make_windows_icon.py <input.png> <output.ico>")
#     else:
#         create_windows_ico(sys.argv[1], sys.argv[2])
