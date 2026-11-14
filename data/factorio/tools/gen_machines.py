import math
import os
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFilter

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from prerender import (Camera, Mesh, Scene, add_bolts, add_box, add_cylinder,
                       add_gear, add_grille, add_prism, add_ring, add_stripes, flatten,
                       norm, outline, render)

ROOT = r"Z:\dev\cpp\meatengine"
OUT = os.path.join(ROOT, "data", "factorio", "res", "textures")

PAL = {
    "steel": (122, 122, 128),
    "steel_light": (168, 168, 176),
    "steel_dark": (72, 72, 80),
    "iron": (88, 88, 94),
    "black": (30, 30, 34),
    "rubber": (56, 54, 52),
    "yellow": (232, 190, 62),
    "yellow_dark": (168, 134, 34),
    "orange": (216, 120, 40),
    "red": (178, 48, 40),
    "blue": (74, 148, 200),
    "blue_light": (126, 196, 236),
    "green": (96, 186, 96),
    "green_light": (150, 226, 140),
    "stone": (142, 140, 134),
    "stone_dark": (98, 96, 92),
    "stone_light": (172, 170, 162),
    "wood": (134, 92, 54),
    "wood_dark": (92, 62, 36),
    "copper": (198, 112, 58),
    "iron_ore": (156, 122, 92),
    "coal": (28, 28, 32),
    "glass": (128, 192, 214),
    "flame": (255, 168, 48),
}


def light_world(camera):
    lc = norm((-0.55, -0.5, 0.67))
    return norm(camera.right * lc[0] + camera.up * lc[1] + camera.eye_dir * lc[2])


def grain(img, amount=0.05, seed=3):
    arr = np.asarray(img, dtype=np.float64)
    rng = np.random.default_rng(seed)
    h, w = arr.shape[:2]
    noise = rng.standard_normal((h, w, 1))
    noise = np.asarray(Image.fromarray(
        ((noise[..., 0] * 0.5 + 0.5) * 255).astype(np.uint8)).filter(
        ImageFilter.GaussianBlur(0.6)), dtype=np.float64)[..., None] / 255.0
    factor = 1.0 + (noise - 0.5) * 2.0 * amount
    arr[..., :3] *= factor
    return Image.fromarray(np.clip(arr, 0, 255).astype(np.uint8), "RGBA")


def crop_center(img, size, bottom_anchor=0.66, scale=1.0):
    bbox = img.getbbox()
    if bbox is None:
        return img.resize((size, size), Image.LANCZOS)
    cropped = img.crop(bbox)
    w, h = cropped.size
    factor = min(size * 0.99 / max(w, h), size / max(w, h))
    factor = min(factor, (size * 0.98) / max(w, h)) * scale
    nw = max(1, int(round(w * factor)))
    nh = max(1, int(round(h * factor)))
    cropped = cropped.resize((nw, nh), Image.LANCZOS)
    canvas = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    ox = (size - nw) // 2
    oy = int(size * bottom_anchor) - nh
    if oy < 0:
        oy = 0
    if oy + nh > size:
        cropped = cropped.crop((0, 0, nw, max(1, size - oy)))
    canvas.alpha_composite(cropped, (ox, oy))
    return canvas


def sprite(mesh, size=64, elev=52.0, azimuth=-40.0, center=None, shadow=0.30,
           blur=0.0, anchor=0.74, fit=1.0, edge=True, g=0.045, fill=0.96):
    verts = np.asarray(mesh.verts, dtype=np.float64)
    if center is None:
        center = ((verts[:, 0].min() + verts[:, 0].max()) * 0.5,
                  (verts[:, 1].min() + verts[:, 1].max()) * 0.5,
                  (verts[:, 2].min() + verts[:, 2].max()) * 0.5)
    cam = Camera(elev_deg=elev, azimuth_deg=azimuth, scale=1.0, center=center)
    rel = verts - np.asarray(center, dtype=np.float64)
    ex = max(float((rel @ cam.right).max() - (rel @ cam.right).min()), 1e-3)
    ey = max(float((rel @ cam.up).max() - (rel @ cam.up).min()), 1e-3)
    target = size * 2.0 * fill
    cam.scale = min(target / ex, target / ey)

    canvas = size * 2
    light = light_world(cam)

    scene = Scene(canvas, canvas, cam)
    scene.add(mesh)
    body = render(scene, light_dir=light, shadow=False)

    if shadow > 0:
        flat_scene = Scene(canvas, canvas, cam)
        flat_scene.add(flatten(mesh))
        blob = render(flat_scene, light_dir=light, shadow=False)
        blob = blob.filter(ImageFilter.GaussianBlur(size * 0.06))
        arr = np.asarray(blob, dtype=np.float64)
        arr[..., 0:3] = 0.05
        arr[..., 3] *= shadow
        blob = Image.fromarray(np.clip(arr, 0, 255).astype(np.uint8), "RGBA")
        body = Image.alpha_composite(blob, body)

    body = body.filter(ImageFilter.GaussianBlur(blur))
    body = crop_center(body, size, bottom_anchor=anchor, scale=fit)
    if edge:
        body = outline(body)
    if g > 0:
        body = grain(body, amount=g)
    return body


def save(img, name):
    path = os.path.join(OUT, name)
    img.save(path)
    return path


def build_drill():
    m = Mesh()
    steel = m.mat(PAL["steel"], spec=0.34, shin=26)
    steel_dark = m.mat(PAL["steel_dark"], spec=0.26)
    steel_light = m.mat(PAL["steel_light"], spec=0.4, shin=30)
    yellow = m.mat(PAL["yellow"], spec=0.38, shin=32)
    yellow_dark = m.mat(PAL["yellow_dark"], spec=0.3)
    black = m.mat(PAL["black"], spec=0.16)
    green = m.mat(PAL["green_light"], spec=0.5, emissive=0.4)

    add_box(m, 0, 0, 0, 4, 30, 30, steel_dark, chamfer=2)
    add_box(m, 0, 0, 4, 9, 26, 26, yellow, chamfer=2, top_mat=yellow_dark)
    add_box(m, 0, 0, 9, 22, 18, 18, steel, chamfer=2)
    add_box(m, 0, 0, 22, 26, 14, 14, steel_dark, chamfer=1)
    add_cylinder(m, 0, 0, 26, 30, 3.4, steel_light, sides=10)
    add_gear(m, 0, 0, 26, 30, 8.0, 8, steel, body_mat=steel_dark)
    add_box(m, 0, 0, 30, 37, 7, 7, steel_light, chamfer=0.8)
    for x in (-9, 9):
        add_box(m, x, 9, 22, 26, 5, 5, steel_dark, chamfer=0.8)
        add_cylinder(m, x, 9, 26, 28, 1.6, green, sides=8)
    add_grille(m, 0, -9, 22.1, 12, 6, 3, black)
    add_bolts(m, [(-11, -11), (11, -11), (11, 11), (-11, 11)], 9.1, 1.4, steel_dark)
    return m


def build_furnace():
    m = Mesh()
    stone = m.mat(PAL["stone"], spec=0.12)
    stone_dark = m.mat(PAL["stone_dark"], spec=0.1)
    stone_light = m.mat(PAL["stone_light"], spec=0.14)
    black = m.mat(PAL["black"], spec=0.08)
    flame = m.mat(PAL["flame"], spec=0.0, emissive=0.9)
    steel = m.mat(PAL["steel_dark"], spec=0.4, shin=28)

    add_box(m, 0, 0, 0, 3, 28, 28, stone_dark, chamfer=1)
    add_box(m, 0, 0, 3, 22, 24, 24, stone, chamfer=1)
    add_box(m, 0, 0, 22, 25, 25, 25, stone_light, chamfer=1)
    for x in (-8, 0, 8):
        add_box(m, x, -12.2, 4, 20, 6, 1.4, stone_light)
    add_box(m, 0, 8, 25, 34, 10, 10, stone_dark, chamfer=0.8)
    add_box(m, 0, 8, 34, 36, 12, 12, stone_light, chamfer=0.8)
    add_box(m, 0, -6, 4, 16, 12, 12, black, chamfer=1)
    add_box(m, 0, -6, 6, 13, 9, 9, flame, chamfer=1)
    add_box(m, 0, 6, 4, 20, 14, 8, stone_dark, chamfer=1)
    return m


def build_belt():
    m = Mesh()
    frame = m.mat(PAL["steel_dark"], spec=0.36, shin=24)
    belt = m.mat(PAL["rubber"], spec=0.1)
    belt_line = m.mat(PAL["yellow_dark"], spec=0.2)
    roller = m.mat(PAL["steel_light"], spec=0.55, shin=34)
    yellow = m.mat(PAL["yellow"], spec=0.35)

    add_box(m, 0, 0, 0, 2.0, 30, 30, frame, chamfer=1.5)
    add_box(m, 0, 0, 2.0, 3.6, 24, 24, belt)
    add_stripes(m, -15, -15, 15, 15, 3.7, 2.2, belt, belt_line, horizontal=True)
    for x in (-10, 0, 10):
        add_cylinder(m, x, 0, 1.6, 4.4, 2.4, roller, sides=10)
    add_box(m, 0, -15, 0, 3.6, 30, 3, yellow)
    return m


def build_inserter():
    m = Mesh()
    base = m.mat(PAL["steel_dark"], spec=0.4, shin=26)
    base_top = m.mat(PAL["steel"], spec=0.36)
    arm = m.mat(PAL["yellow"], spec=0.42, shin=32)
    arm_dark = m.mat(PAL["yellow_dark"], spec=0.3)
    hub = m.mat(PAL["steel_light"], spec=0.55, shin=38)
    black = m.mat(PAL["black"], spec=0.15)

    add_box(m, 0, 0, 0, 3.5, 24, 24, base, chamfer=2)
    add_cylinder(m, 0, 0, 3.5, 7.0, 7.5, base_top, sides=14)
    add_gear(m, 0, 0, 7.0, 10.0, 8.5, 8, hub, body_mat=base)
    add_box(m, 0, -5, 10, 13, 4.5, 18, arm, chamfer=1)
    add_box(m, 0, -5, 13, 14.2, 3.4, 15, arm_dark)
    add_box(m, 0, -12, 13, 16, 7, 5, hub, chamfer=0.8)
    add_box(m, 0, -12, 16, 18, 4, 3, black, chamfer=0.5)
    add_box(m, 0, 2, 10, 12, 3.2, 8, arm, chamfer=0.8)
    return m


def build_chest():
    m = Mesh()
    wood = m.mat(PAL["wood"], spec=0.12)
    wood_dark = m.mat(PAL["wood_dark"], spec=0.1)
    band = m.mat(PAL["steel_dark"], spec=0.45, shin=30)
    lock = m.mat(PAL["yellow"], spec=0.5, shin=36)

    add_box(m, 0, 0, 0, 3, 28, 24, wood_dark, chamfer=1)
    add_box(m, 0, 0, 3, 15, 26, 22, wood, chamfer=1)
    add_box(m, 0, 0, 15, 21, 27, 23, wood_dark, chamfer=2)
    for y in (-11, 11):
        add_box(m, 0, y, 3, 15, 28, 2.2, band)
    add_box(m, 0, 0, 15, 21.4, 28, 4.5, band)
    add_box(m, 0, -11.6, 8, 14, 5, 2.4, lock)
    for x in (-9, 9):
        add_box(m, x, 0, 0.5, 3.2, 3, 24, band)
    return m


def build_assembler():
    m = Mesh()
    body = m.mat((104, 122, 152), spec=0.34, shin=26)
    body_dark = m.mat((62, 74, 100), spec=0.28)
    steel = m.mat(PAL["steel_light"], spec=0.5, shin=32)
    yellow = m.mat(PAL["yellow"], spec=0.4, shin=34)
    glass = m.mat(PAL["glass"], spec=0.7, shin=48)
    black = m.mat(PAL["black"], spec=0.14)

    add_box(m, 0, 0, 0, 4, 30, 30, body_dark, chamfer=2)
    add_box(m, 0, 0, 4, 16, 26, 26, body, chamfer=2)
    add_box(m, 0, 0, 16, 19, 22, 22, body_dark, chamfer=1)
    add_box(m, 0, 0, 19, 22, 18, 18, yellow, chamfer=1)
    add_gear(m, 0, 0, 22, 26, 9.0, 10, steel, body_mat=body_dark)
    add_cylinder(m, 0, 0, 26, 31, 5.0, glass, sides=12)
    add_cylinder(m, 0, 0, 31, 33, 3.0, steel, sides=12)
    for x in (-11, 11):
        add_box(m, x, -13, 6, 14, 2.6, 2.6, steel)
        add_box(m, x, 13, 6, 14, 2.6, 2.6, steel)
        add_box(m, x, 0, 4, 6, 5, 22, black)
    add_bolts(m, [(-11, -11), (11, -11), (11, 11), (-11, 11)], 16.1, 1.5, steel)
    return m


def build_lab():
    m = Mesh()
    body = m.mat((120, 136, 150), spec=0.34, shin=26)
    body_dark = m.mat((72, 84, 98), spec=0.28)
    steel = m.mat(PAL["steel_light"], spec=0.55, shin=36)
    glass = m.mat(PAL["blue_light"], spec=0.75, shin=52, emissive=0.4)
    green = m.mat(PAL["green_light"], spec=0.4, emissive=0.55)
    black = m.mat(PAL["black"], spec=0.14)

    add_box(m, 0, 0, 0, 4, 28, 28, body_dark, chamfer=2)
    add_box(m, 0, 0, 4, 20, 24, 24, body, chamfer=2)
    add_box(m, 0, 0, 20, 23, 20, 20, body_dark, chamfer=1)
    add_cylinder(m, 0, 0, 23, 32, 7.0, glass, sides=14)
    add_cylinder(m, 0, 0, 32, 34, 5.0, steel, sides=14)
    for x in (-8, 8):
        add_cylinder(m, x, 7, 23, 27, 2.0, green, sides=8)
        add_box(m, x, -9, 6, 16, 3, 3, steel)
    add_grille(m, 0, -11, 20.1, 14, 5, 3, black)
    return m


def build_arrow(direction):
    m = Mesh()
    yellow = m.mat(PAL["yellow"], spec=0.42, shin=34)
    yellow_dark = m.mat(PAL["yellow_dark"], spec=0.3)
    if direction == "up":
        add_box(m, 0, 5, 0, 2.0, 4.0, 12.0, yellow)
        add_box(m, 0, -3, 0, 2.0, 11.0, 8.0, yellow_dark)
    elif direction == "down":
        add_box(m, 0, -5, 0, 2.0, 4.0, 12.0, yellow)
        add_box(m, 0, 3, 0, 2.0, 11.0, 8.0, yellow_dark)
    elif direction == "right":
        add_box(m, 5, 0, 0, 2.0, 12.0, 4.0, yellow)
        add_box(m, -3, 0, 0, 2.0, 8.0, 11.0, yellow_dark)
    else:
        add_box(m, -5, 0, 0, 2.0, 12.0, 4.0, yellow)
        add_box(m, 3, 0, 0, 2.0, 8.0, 11.0, yellow_dark)
    return m


def build_item(kind):
    m = Mesh()
    if kind in ("iron_ore", "copper_ore"):
        col = PAL["iron_ore"] if kind == "iron_ore" else PAL["copper"]
        mat = m.mat(col, spec=0.32, shin=18)
        rng = np.random.default_rng(7)
        for _ in range(4):
            add_prism(m, rng.uniform(-3.0, 3.0), rng.uniform(-3.0, 3.0), 0,
                      rng.uniform(2.0, 3.4), rng.uniform(2.0, 3.0), mat, sides=6,
                      rot=rng.uniform(0, 1.0))
        return m
    if kind == "coal":
        mat = m.mat(PAL["coal"], spec=0.2)
        rng = np.random.default_rng(11)
        for _ in range(5):
            add_prism(m, rng.uniform(-3.0, 3.0), rng.uniform(-3.0, 3.0), 0,
                      rng.uniform(1.8, 2.8), rng.uniform(1.6, 2.4), mat, sides=6,
                      rot=rng.uniform(0, 1.0))
        return m
    if kind == "stone":
        mat = m.mat(PAL["stone"], spec=0.2)
        rng = np.random.default_rng(13)
        for _ in range(3):
            add_prism(m, rng.uniform(-2.8, 2.8), rng.uniform(-2.8, 2.8), 0,
                      rng.uniform(2.6, 3.4), rng.uniform(2.4, 3.2), mat, sides=7,
                      rot=rng.uniform(0, 1.0))
        return m
    if kind == "iron_plate":
        mat = m.mat(PAL["steel_light"], spec=0.55, shin=36)
        add_box(m, 0, 0, 0, 1.4, 11, 7, mat, chamfer=0.5)
        add_box(m, 0, 0, 1.4, 2.2, 9, 5, mat, chamfer=0.4)
        return m
    if kind == "copper_plate":
        mat = m.mat(PAL["copper"], spec=0.5, shin=34)
        add_box(m, 0, 0, 0, 1.4, 11, 7, mat, chamfer=0.5)
        add_box(m, 0, 0, 1.4, 2.2, 9, 5, mat, chamfer=0.4)
        return m
    if kind == "gear":
        mat = m.mat(PAL["steel"], spec=0.6, shin=40)
        add_gear(m, 0, 0, 0, 2.2, 6.0, 8, mat,
                 body_mat=m.mat(PAL["steel_dark"], spec=0.4))
        return m
    if kind == "science_pack":
        shell = m.mat(PAL["blue"], spec=0.4, shin=30)
        glass = m.mat(PAL["blue_light"], spec=0.8, shin=50, emissive=0.55)
        add_box(m, 0, 0, 0, 5.5, 8, 8, shell, chamfer=1.2)
        add_box(m, 0, 0, 5.5, 8.0, 5, 5, glass, chamfer=0.8)
        return m
    if kind == "battery":
        mat = m.mat(PAL["steel_dark"], spec=0.4)
        add_box(m, 0, 0, 0, 7, 7, 7, mat, chamfer=0.8)
        add_box(m, 0, 0, 7, 8.4, 4, 4, m.mat(PAL["yellow"], spec=0.4))
        return m
    if kind == "rocket_part":
        mat = m.mat((198, 200, 210), spec=0.5, shin=34)
        add_prism(m, 0, 0, 0, 9, 4.0, mat, sides=8, rot=math.pi / 8)
        add_box(m, 0, 0, 9, 11, 5, 5, m.mat(PAL["orange"], spec=0.3))
        return m
    raise ValueError(kind)


def build_tree(kind):
    m = Mesh()
    trunk = m.mat(PAL["wood_dark"], spec=0.12)
    if kind == "tree":
        leaf = m.mat((104, 176, 78), spec=0.1)
        leaf_dark = m.mat((74, 140, 62), spec=0.08)
        add_box(m, 0, 0, 0, 10, 4, 4, trunk, chamfer=0.8)
        add_prism(m, 0, 0, 10, 24, 13.0, leaf, sides=7, rot=0.3)
        add_prism(m, 0, 0, 20, 31, 10.0, leaf_dark, sides=7, rot=0.9)
        add_prism(m, 0, 0, 28, 36, 6.0, leaf, sides=7, rot=1.6)
    elif kind == "tree2":
        leaf = m.mat((112, 182, 84), spec=0.12)
        add_box(m, 0, 0, 0, 6, 3.5, 3.5, trunk, chamfer=0.7)
        add_prism(m, 0, 0, 6, 17, 11.0, leaf, sides=8, rot=0.2)
        add_prism(m, 0, 0, 14, 23, 7.5, leaf, sides=8, rot=1.1)
    else:
        leaf = m.mat((104, 172, 76), spec=0.12)
        add_prism(m, 0, 0, 0, 7, 8.0, leaf, sides=8, rot=0.4)
        add_prism(m, 2, 1, 5, 10, 5.0, leaf, sides=7, rot=1.2)
    return m


def build_overlay(kind):
    m = Mesh()
    if kind == "gear":
        mat = m.mat(PAL["steel_light"], spec=0.65, shin=44)
        add_gear(m, 0, 0, 0, 2.0, 6.0, 8, mat)
    elif kind == "gear_gold":
        mat = m.mat(PAL["yellow"], spec=0.7, shin=46)
        add_gear(m, 0, 0, 0, 2.0, 6.0, 8, mat)
    elif kind == "flame":
        mat = m.mat(PAL["flame"], spec=0.0, emissive=0.9)
        add_prism(m, 0, 0, 0, 6.0, 3.0, mat, sides=6, rot=0.2)
        add_prism(m, 0, 0, 5, 10.0, 2.0, mat, sides=6, rot=0.9)
    elif kind == "furnace_glow":
        mat = m.mat(PAL["flame"], spec=0.0, emissive=0.85)
        add_prism(m, 0, 0, 0, 2.5, 5.0, mat, sides=8, rot=0.2)
    elif kind == "science_glow":
        mat = m.mat(PAL["blue_light"], spec=0.0, emissive=0.85)
        add_prism(m, 0, 0, 0, 3.0, 4.0, mat, sides=8, rot=0.4)
    return m


def build_particle(kind):
    m = Mesh()
    if kind == "smoke":
        mat = m.mat((176, 176, 184), spec=0.0, emissive=0.22)
        add_prism(m, 0, 0, 0, 5.0, 3.0, mat, sides=6, rot=0.3)
    elif kind == "spark":
        mat = m.mat((255, 216, 110), spec=0.0, emissive=1.0)
        add_box(m, 0, 0, 0, 3.0, 1.6, 1.6, mat)
    elif kind == "dust":
        mat = m.mat((196, 176, 134), spec=0.0, emissive=0.18)
        add_prism(m, 0, 0, 0, 2.5, 2.2, mat, sides=5, rot=0.5)
    elif kind == "glow_blue":
        mat = m.mat(PAL["blue_light"], spec=0.0, emissive=0.9)
        add_prism(m, 0, 0, 0, 4.0, 3.2, mat, sides=8, rot=0.2)
    elif kind == "glow_yellow":
        mat = m.mat((255, 228, 130), spec=0.0, emissive=0.9)
        add_prism(m, 0, 0, 0, 4.0, 3.2, mat, sides=8, rot=0.2)
    return m


def build_character():
    m = Mesh()
    suit = m.mat((64, 96, 156), spec=0.22)
    suit_dark = m.mat((44, 68, 116), spec=0.18)
    shirt = m.mat((226, 224, 214), spec=0.2)
    skin = m.mat((226, 178, 140), spec=0.16)
    hat = m.mat((232, 196, 60), spec=0.42, shin=32)
    hat_dark = m.mat((186, 152, 40), spec=0.32)
    boot = m.mat((58, 52, 48), spec=0.2)

    add_box(m, -4.5, 0, 0, 6, 4.5, 6, boot, chamfer=0.8)
    add_box(m, 4.5, 0, 0, 6, 4.5, 6, boot, chamfer=0.8)
    add_box(m, 0, 0, 6, 15, 11, 8, suit, chamfer=1.5)
    add_box(m, 0, 0, 15, 19, 10, 7, suit_dark, chamfer=1.2)
    add_box(m, 0, -0.5, 15, 19.5, 5.5, 6, shirt, chamfer=0.8)
    add_box(m, -7, 0, 15, 19, 3.5, 4, suit, chamfer=0.9)
    add_box(m, 7, 0, 15, 19, 3.5, 4, suit, chamfer=0.9)
    add_box(m, 0, 0, 19, 24, 7, 6, skin, chamfer=1.4)
    add_box(m, 0, 0, 24, 25.5, 10, 7, hat, chamfer=1.8)
    add_box(m, 0, -3.6, 24, 25.5, 8, 3.4, hat_dark, chamfer=1.0)
    add_box(m, 0, 0, 25.5, 27.5, 3, 3, hat, chamfer=0.8)
    return m


def build_cursor(ok):
    m = Mesh()
    col = (120, 240, 120) if ok else (250, 110, 110)
    mat = m.mat(col, spec=0.2, emissive=0.55)
    w = 26.0
    t = 2.2
    add_box(m, 0, -w / 2, 0, 1.2, w, t, mat)
    add_box(m, 0, w / 2, 0, 1.2, w, t, mat)
    add_box(m, -w / 2, 0, 0, 1.2, t, w, mat)
    add_box(m, w / 2, 0, 0, 1.2, t, w, mat)
    return m


MACHINE_SPRITES = {
    "drill": build_drill,
    "furnace": build_furnace,
    "belt": build_belt,
    "inserter": build_inserter,
    "chest": build_chest,
    "assembler": build_assembler,
    "lab": build_lab,
}

ITEM_KINDS = ("iron_ore", "copper_ore", "coal", "stone", "iron_plate", "copper_plate",
               "gear", "science_pack", "battery", "rocket_part")


def main():
    os.makedirs(OUT, exist_ok=True)
    args = sys.argv[1:] or ["all"]

    def want(name):
        return "all" in args or name in args

    if want("machines"):
        for name, builder in MACHINE_SPRITES.items():
            img = sprite(builder(), 64, elev=52, azimuth=-40, shadow=0.32, anchor=0.72, fit=0.95)
            save(img, "%s.png" % name)

    if want("arrows"):
        for name in ("up", "right", "down", "left"):
            save(sprite(build_arrow(name), 24, elev=42, azimuth=-40, shadow=0.0, blur=0.7, fit=0.9, g=0.0),
                 "arrow_%s.png" % name)

    if want("items"):
        for kind in ITEM_KINDS:
            save(sprite(build_item(kind), 16, elev=50, azimuth=-40, shadow=0.2, blur=0.7, fit=0.9, g=0.03),
                 "%s.png" % kind)

    if want("decor"):
        save(sprite(build_tree("tree"), 56, elev=50, azimuth=-40, shadow=0.26, fit=0.96), "decor_tree.png")
        save(sprite(build_tree("tree2"), 44, elev=50, azimuth=-40, shadow=0.26, fit=0.95), "decor_tree2.png")
        save(sprite(build_tree("bush"), 32, elev=50, azimuth=-40, shadow=0.24, fit=0.94), "decor_bush.png")

    if want("overlays"):
        save(sprite(build_overlay("gear"), 16, elev=42, azimuth=-40, shadow=0.0, blur=0.6, fit=0.92, g=0.0), "overlay_gear.png")
        save(sprite(build_overlay("gear_gold"), 16, elev=42, azimuth=-40, shadow=0.0, blur=0.6, fit=0.92, g=0.0), "overlay_gear_gold.png")
        save(sprite(build_overlay("flame"), 16, elev=42, azimuth=-40, shadow=0.0, blur=0.8, fit=0.92, g=0.0), "overlay_flame.png")
        save(sprite(build_overlay("furnace_glow"), 16, elev=42, azimuth=-40, shadow=0.0, blur=1.0, fit=0.92, g=0.0), "furnace_glow.png")
        save(sprite(build_overlay("science_glow"), 16, elev=42, azimuth=-40, shadow=0.0, blur=1.0, fit=0.92, g=0.0), "science_glow.png")

    if want("particles"):
        save(sprite(build_particle("smoke"), 12, elev=42, azimuth=-40, shadow=0.0, blur=0.9, fit=0.9, g=0.0), "particle_smoke.png")
        save(sprite(build_particle("spark"), 10, elev=42, azimuth=-40, shadow=0.0, blur=0.6, fit=0.9, g=0.0), "particle_spark.png")
        save(sprite(build_particle("dust"), 10, elev=42, azimuth=-40, shadow=0.0, blur=0.8, fit=0.9, g=0.0), "particle_dust.png")
        save(sprite(build_particle("glow_blue"), 14, elev=42, azimuth=-40, shadow=0.0, blur=1.0, fit=0.9, g=0.0), "particle_glow_blue.png")
        save(sprite(build_particle("glow_yellow"), 14, elev=42, azimuth=-40, shadow=0.0, blur=1.0, fit=0.9, g=0.0), "particle_glow_yellow.png")

    if want("cursors"):
        save(sprite(build_cursor(True), 32, elev=42, azimuth=-40, shadow=0.0, blur=0.6, fit=0.92, g=0.0), "cursor_ok.png")
        save(sprite(build_cursor(False), 32, elev=42, azimuth=-40, shadow=0.0, blur=0.6, fit=0.92, g=0.0), "cursor_bad.png")

    if want("character"):
        save(sprite(build_character(), 32, elev=48, azimuth=-40, shadow=0.26,
                    anchor=0.86, fit=0.94), "character.png")

    if want("preview"):
        sheet = Image.new("RGBA", (640, 400), (30, 32, 38, 255))
        names = ["drill.png", "furnace.png", "belt.png", "inserter.png", "chest.png",
                 "assembler.png", "lab.png", "decor_tree.png", "decor_tree2.png",
                 "decor_bush.png", "arrow_up.png", "arrow_right.png"]
        for i, name in enumerate(names):
            path = os.path.join(OUT, name)
            if not os.path.exists(path):
                continue
            im = Image.open(path).convert("RGBA")
            x = 10 + (i % 6) * 105
            y = 10 + (i // 6) * 190
            sheet.alpha_composite(im, (x, y))
        items = ["iron_ore.png", "copper_ore.png", "coal.png", "stone.png",
                 "iron_plate.png", "copper_plate.png", "gear.png", "science_pack.png"]
        for i, name in enumerate(items):
            path = os.path.join(OUT, name)
            if not os.path.exists(path):
                continue
            im = Image.open(path).convert("RGBA")
            sheet.alpha_composite(im, (10 + i * 40, 380))
        sheet.save(os.path.join(ROOT, "out", "tools", "preview.png"))


if __name__ == "__main__":
    main()
