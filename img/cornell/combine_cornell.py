#!/usr/bin/env python3

import argparse
from pathlib import Path

from PIL import Image

DEFAULT_ORDER = [
    "CornellApple.png",
    "CornellChicken1.png",
    "CornellChicken2.png",
    "CornellCream.png",
    "CornellKetchup.png",
    "CornellMarble.png",
    "CornellPotato.png",
    "CornellSkimmilk.png",
    "CornellSkin1.png",
    "CornellSkin2.png",
    "CornellSpectralon.png",
    "CornellWholemilk.png",
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
        description="Combine 12 Cornell images into a 2x6 grid."
    )
    parser.add_argument(
        "folder",
        nargs="?",
        default=".",
        help="Folder containing Cornell*.png files (default: current folder).",
    )
    parser.add_argument(
        "--output",
        default="CornellGrid.png",
        help="Output image name (default: CornellGrid.png).",
    )
    args = parser.parse_args()

    folder = Path(args.folder).resolve()
    if not folder.is_dir():
        raise SystemExit(f"Not a folder: {folder}")

    images = load_images(folder, DEFAULT_ORDER)
    if len(images) != 12:
        raise SystemExit("Expected 12 images in the order list.")

    widths = {img.width for img in images}
    heights = {img.height for img in images}
    if len(widths) != 1 or len(heights) != 1:
        raise SystemExit("All images must have the same dimensions.")

    img_w = images[0].width
    img_h = images[0].height
    cols = 6
    rows = 2
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
