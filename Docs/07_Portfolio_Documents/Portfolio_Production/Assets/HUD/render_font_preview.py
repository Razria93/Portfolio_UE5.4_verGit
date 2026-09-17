"""Render real font specimens, not a game screenshot or UMG test."""
import argparse
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

parser = argparse.ArgumentParser()
parser.add_argument("font_directory", type=Path)
parser.add_argument("--engine-font-directory", type=Path,
                    default=Path("C:/Program Files/Epic Games/UE_5.4/Engine/Content/Slate/Fonts"))
args = parser.parse_args()
image = Image.new("RGB", (1600, 960), "#12181d")
draw = ImageDraw.Draw(image)
heading = ImageFont.truetype(str(args.engine_font_directory / "Roboto-Regular.ttf"), 22)
draw.text((32, 20), "HUD FONT STUDY / REAL FONT FILES / NOT UMG RENDERING", font=heading, fill="#f1f2ee")
rows = [
    ("Oxanium Regular", args.font_directory / "Oxanium-Regular.ttf", "TARGET   HP  SH  BE  BU   0123456789"),
    ("Oxanium Medium", args.font_directory / "Oxanium-Medium.ttf", "TARGET   HP  SH  BE  BU   0123456789"),
    ("Roboto Regular (engine)", args.engine_font_directory / "Roboto-Regular.ttf", "TARGET   HP  SH  BE  BU   0123456789"),
    ("Noto Sans CJK KR Regular", args.font_directory / "NotoSansCJKkr-Regular.otf", "대상 이름 / 체력 / 미구현   0123456789"),
]
for index, (label, path, text) in enumerate(rows):
    y = 78 + 212 * index
    draw.text((32, y), label, font=heading, fill="#f1f2ee")
    draw.rectangle((810, y+34, 1568, y+196), fill="#d3d3d3")
    for line, size in enumerate((28, 22, 17)):
        font = ImageFont.truetype(str(path), size)
        sample = f"{size}px  {text}"
        draw.text((32, y+44+46*line), sample, font=font, fill="#f1f2ee")
        draw.text((826, y+44+46*line), sample, font=font, fill="#12181d")
image.save(Path(__file__).with_name("font-study.png"))
