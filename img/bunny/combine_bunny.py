#!/usr/bin/env python3

import argparse
from pathlib import Path

from PIL import Image

DEFAULT_ORDER = [
"BunnyWireframe.png",
"BunnyBVH.png",
]


def load_images(folder: Path, order: list[str]) -> list[Image.Image]:
    images: list[Image.Image] = []
    for name in order:
        path = folder / name
        if not path.is_file():
            raise SystemExit(f"Missing file: {path}")
        images.append(Image.open(path))
    return images


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Combine 2 Bunny images into a 1x2 grid."
    )
    parser.add_argument(
        "folder",
        nargs="?",
        default=".",
        help="Folder containing Bunny*.png files (default: current folder).",
    )
    parser.add_argument(
        "--output",
        default="BunnyGrid.png",
        help="Output image name (default: BunnyGrid.png).",
    )
    args = parser.parse_args()

    folder = Path(args.folder).resolve()
    if not folder.is_dir():
        raise SystemExit(f"Not a folder: {folder}")

    images = load_images(folder, DEFAULT_ORDER)
    if len(images) != 2:
        raise SystemExit("Expected 2 images in the order list.")

    widths = {img.width for img in images}
    heights = {img.height for img in images}
    if len(widths) != 1 or len(heights) != 1:
        raise SystemExit("All images must have the same dimensions.")

    img_w = images[0].width
    img_h = images[0].height
    cols = 2
    rows = 1
    canvas = Image.new("RGBA", (cols * img_w, rows * img_h))

    for idx, img in enumerate(images):
        row = idx // cols
        col = idx % cols
        x = col * img_w
        y = row * img_h
        canvas.paste(img, (x, y))

    output_path = folder / args.output
    canvas.save(output_path)
    print(f"Wrote {output_path}")

    for img in images:
        img.close()


if __name__ == "__main__":
    main()
