#!/usr/bin/env python3

import argparse
from pathlib import Path

from PIL import Image


def convert_bitmaps(folder: Path, overwrite: bool) -> int:
	count = 0
	for bmp_path in folder.glob("*.bmp"):
		png_path = bmp_path.with_suffix(".png")
		if png_path.exists() and not overwrite:
			continue
		with Image.open(bmp_path) as image:
			image.save(png_path, format="PNG")
		count += 1
	return count


def main() -> None:
	parser = argparse.ArgumentParser(
		description="Convert all .bmp images in a folder to .png."
	)
	parser.add_argument(
		"folder",
		nargs="?",
		default=".",
		help="Folder containing .bmp files (default: current folder).",
	)
	parser.add_argument(
		"--overwrite",
		action="store_true",
		help="Overwrite existing .png files.",
	)
	args = parser.parse_args()

	folder = Path(args.folder).resolve()
	if not folder.is_dir():
		raise SystemExit(f"Not a folder: {folder}")

	converted = convert_bitmaps(folder, args.overwrite)
	print(f"Converted {converted} file(s).")


if __name__ == "__main__":
	main()
