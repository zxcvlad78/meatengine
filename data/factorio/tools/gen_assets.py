import math
import os
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFilter

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from prerender import (Camera, Mesh, Scene, add_bolts, add_box, add_cylinder,
                       add_gear, add_grille, add_prism, add_ring, add_stripes, norm,
                       render)

ROOT = r"Z:\dev\cpp\meatengine"
OUT = os.path.join(ROOT, "data", "factorio", "res", "textures")
RNG = np.random.default_rng(20261113)

PAL = {
    "rust": (188, 96, 38),
    "rust_dark": (140, 66, 28),
    "rust_light": (214, 124, 58),
    "steel": (124, 130, 138),
    "steel_light": (168, 174, 182),
    "steel_dark": (74, 80, 88),
    "iron": (96, 100, 108),
    "black": (34, 36, 40),
    "coal": (28, 30, 34),
    "brass": (198, 154, 62),
    "brass_dark": (150, 112, 40),
    "hazard": (232, 196, 52),
    "red": (196, 56, 44),
    "red_glow": (255, 96, 52),
    "glass": (126, 190, 214),
    "blue": (86, 164, 206),
    "cyan": (110, 216, 226),
    "wood": (128, 88, 54),
    "wood_dark": (92, 62, 38),
    "stone": (150, 148, 142),
    "stone_dark": (104, 102, 98),
    "stone_light": (178, 176, 168),
    "grass": (92, 122, 60),
    "grass_dark": (64, 92, 44),
    "grass_light": (124, 154, 82),
    "dirt": (118, 90, 62),
    "sand": (198, 178, 124),
    "water": (46, 92, 108),
}

LIGHT_CAM = norm((-0.52, -0.55, 0.655))


def light_world(camera):
    return norm(camera.right * LIGHT_CAM[0] + camera.up * LIGHT_CAM[1] + camera.eye_dir * LIGHT_CAM[2])


def render_sprite(mesh, size, elev=62.0, azimuth=-35.0, scale=1.0, center=(0.0, 0.0, 0.0),
                  ground=None, ground_color=None, shadow=0.30, blur=1.8):
    cam = Camera(elev_deg=elev, azimuth_deg=azimuth, scale=scale, center=center)
    scene = Scene(size[0], size[1], cam)
    scene.add(mesh)
    img = render(scene, light_dir=light_world(cam), shadow=shadow > 0,
                 shadow_strength=shadow, ground_color=ground_color)
    return img


def save(img, name):
    path = os.path.join(OUT, name)
    img.save(path)
    return path


FONT = None


def fbm(n, seed, octaves=((4, 1.0), (8, 0.55), (16, 0.3), (32, 0.15))):
    rng = np.random.default_rng(seed)
    white = rng.standard_normal((n, n))
    freq = np.fft.fft2(white)
    fy = np.fft.fftfreq(n)[:, None]
    fx = np.fft.fftfreq(n)[None, :]
    rad = np.sqrt(fx * fx + fy * fy)
    out = np.zeros((n, n), dtype=np.float64)
    for edge, weight in octaves:
        sigma = 0.5 / edge
        filt = np.exp(-(rad / sigma) ** 2)
        band = np.real(np.fft.ifft2(freq * filt))
        std = band.std()
        if std > 1e-9:
            band /= std
        out += band * weight
    std = out.std()
    if std > 1e-9:
        out /= std
    return out


def height_shade(height, light, strength=1.0, base=0.92):
    gy, gx = np.gradient(height)
    nx = -gx * strength
    ny = -gy * strength
    nz = np.ones_like(height)
    ln = np.sqrt(nx * nx + ny * ny + nz * nz)
    lx, ly, lz = light
    lam = (nx * lx + ny * ly + nz * lz) / ln
    return np.clip(base + lam * 0.9, 0.0, 1.9)


def colorize(albedo, shade):
    out = albedo * shade[..., None]
    return out


def put(arr, x, y, color, alpha=1.0):
    h, w = arr.shape[:2]
    yy = np.arange(y, y + color.shape[0]) % h
    xx = np.arange(x, x + color.shape[1]) % w
    c = color
    a = alpha
    if c.ndim == 2:
        c = np.repeat(c[..., None], arr.shape[2], axis=2)
    for i in range(color.shape[0]):
        for j in range(color.shape[1]):
            arr[yy[i], xx[j], :3] = arr[yy[i], xx[j], :3] * (1 - a) + c[i, j, :3] * a


def nugget(size, base_color, seed, radius=3.2, count=None):
    rng = np.random.default_rng(seed)
    img = np.zeros((size, size, 3), dtype=np.float64)
    alpha = np.zeros((size, size), dtype=np.float64)
    if count is None:
        count = 4
    base = np.array(base_color, dtype=np.float64) / 255.0
    light = np.array(norm((-0.55, -0.5, 0.7)))
    for _ in range(count):
        cx = rng.uniform(3, size - 3)
        cy = rng.uniform(3, size - 3)
        r = radius * rng.uniform(0.65, 1.25)
        half = int(math.ceil(r + 2))
        for y in range(-half, half + 1):
            for x in range(-half, half + 1):
                px = cx + x
                py = cy + y
                if not (0 <= px < size and 0 <= py < size):
                    continue
                dx = (px - cx) / r
                dy = (py - cy) / r
                d2 = dx * dx + dy * dy
                if d2 > 1.0:
                    continue
                dz = math.sqrt(max(1.0 - d2, 0.0))
                nx, ny, nz = dx, dy, dz
                lam = max(nx * light[0] + ny * light[1] + nz * light[2], 0.0)
                spec = max(nz ** 18, 0.0) * 0.7
                tint = 0.45 + 0.75 * lam + spec
                col = np.clip(base * tint + spec * 0.35, 0.0, 1.0)
                img[int(py), int(px)] = col
                alpha[int(py), int(px)] = 1.0
        shadow_r = r * 1.05
        for y in range(-half - 1, half + 2):
            for x in range(-half - 1, half + 2):
                px = int(cx + x + r * 0.35)
                py = int(cy + y + r * 0.45)
                if not (0 <= px < size and 0 <= py < size):
                    continue
                d2 = (x / shadow_r) ** 2 + (y / shadow_r) ** 2
                if d2 <= 1.0 and alpha[py, px] < 0.5:
                    img[py, px] *= 0.82
    return img, alpha


def make_ground_tile(size, kind, seed):
    rng = np.random.default_rng(seed)
    n = size
    height = fbm(n, seed + 1, octaves=((2, 0.6), (4, 1.0), (8, 0.6), (16, 0.35), (32, 0.18)))
    detail = fbm(n, seed + 2, octaves=((4, 0.5), (8, 0.9), (16, 0.7), (32, 0.5)))
    if kind in ("grass", "grass2"):
        g = np.array(PAL["grass"], dtype=np.float64) / 255.0
        gd = np.array(PAL["grass_dark"], dtype=np.float64) / 255.0
        gl = np.array(PAL["grass_light"], dtype=np.float64) / 255.0
        t = np.clip(0.5 + detail * 0.11, 0, 1)
        albedo = g[None, None, :] * (1 - t[..., None]) + gl[None, None, :] * t[..., None]
        dry = np.clip(0.5 + fbm(n, seed + 7, octaves=((2, 1.0), (4, 0.5))) * 0.3, 0, 1)
        dry_c = np.array((138, 136, 74), dtype=np.float64) / 255.0
        albedo = albedo * (1 - dry[..., None] * 0.25) + dry_c[None, None, :] * dry[..., None] * 0.25
        shade = height_shade(height, norm((-0.4, -0.4, 0.82)), strength=1.4, base=0.95)
        img = colorize(albedo, shade)
        if kind == "grass2":
            img *= 0.95
        pil = Image.fromarray((np.clip(img, 0, 1) * 255).astype(np.uint8)).convert("RGBA")
        draw = ImageDraw.Draw(pil)
        for _ in range(14 if kind == "grass" else 10):
            x = rng.uniform(1, n - 1)
            y = rng.uniform(1, n - 1)
            ln = rng.uniform(1.2, 2.6)
            col = (int(150 + rng.uniform(-30, 30)), int(180 + rng.uniform(-30, 30)), int(96 + rng.uniform(-20, 20)), 190)
            draw.line([(x, y), (x + rng.uniform(-0.8, 0.8), y - ln)], fill=col, width=1)
        if kind == "grass2":
            for _ in range(4):
                x = rng.uniform(3, n - 3)
                y = rng.uniform(3, n - 3)
                c = (int(214 + rng.uniform(-20, 20)), int(196 + rng.uniform(-20, 20)), int(96), 220)
                draw.ellipse([x - 1.2, y - 1.2, x + 1.2, y + 1.2], fill=c)
                draw.ellipse([x - 0.7, y - 0.7, x + 0.7, y + 0.7], fill=(240, 226, 130, 230))
        return pil
    if kind == "dirt":
        d = np.array(PAL["dirt"], dtype=np.float64) / 255.0
        dl = np.array((142, 112, 78), dtype=np.float64) / 255.0
        t = np.clip(0.5 + detail * 0.3, 0, 1)
        albedo = d[None, None, :] * (1 - t[..., None]) + dl[None, None, :] * t[..., None]
        shade = height_shade(height, norm((-0.4, -0.4, 0.82)), strength=1.8, base=0.9)
        img = colorize(albedo, shade)
        pil = Image.fromarray((np.clip(img, 0, 1) * 255).astype(np.uint8)).convert("RGBA")
        draw = ImageDraw.Draw(pil)
        for _ in range(14):
            x = rng.uniform(2, n - 2)
            y = rng.uniform(2, n - 2)
            r = rng.uniform(0.8, 1.8)
            draw.ellipse([x - r, y - r, x + r, y + r], fill=(84, 66, 48, 190))
            draw.ellipse([x - r * 0.6, y - r * 0.8, x + r * 0.2, y], fill=(126, 104, 78, 150))
        return pil
    if kind == "sand":
        s = np.array(PAL["sand"], dtype=np.float64) / 255.0
        sl = np.array((216, 200, 152), dtype=np.float64) / 255.0
        t = np.clip(0.5 + detail * 0.35, 0, 1)
        albedo = s[None, None, :] * (1 - t[..., None]) + sl[None, None, :] * t[..., None]
        ripple = np.sin((np.arange(n)[None, :] / n * 2 * math.pi * 2) + fbm(n, seed + 5) * 2.2) * 0.5 + 0.5
        h = height + ripple * 0.35
        shade = height_shade(h, norm((-0.4, -0.4, 0.82)), strength=1.2, base=0.97)
        img = colorize(albedo, shade)
        return Image.fromarray((np.clip(img, 0, 1) * 255).astype(np.uint8)).convert("RGBA")
    if kind == "stone":
        base = np.array(PAL["stone"], dtype=np.float64) / 255.0
        dark = np.array((116, 112, 108), dtype=np.float64) / 255.0
        t = np.clip(0.5 + detail * 0.3, 0, 1)
        albedo = base[None, None, :] * (1 - t[..., None]) + dark[None, None, :] * t[..., None]
        shade = height_shade(height, norm((-0.4, -0.4, 0.82)), strength=1.0, base=0.95)
        img = colorize(albedo, shade)
        pil = Image.fromarray((np.clip(img, 0, 1) * 255).astype(np.uint8)).convert("RGBA")
        draw = ImageDraw.Draw(pil)
        grout = (74, 72, 70, 210)
        cx = n / 2
        cy = n / 2
        rects = [(-1, -1), (0, -1), (1, -1), (-1, 0), (0, 0), (1, 0), (-1, 1), (0, 1), (1, 1)]
        for i, (gx, gy) in enumerate(rects):
            if i % 2 == 0:
                continue
            x0 = gx * n / 3 + 2
            y0 = gy * n / 3 + 2
            x1 = x0 + n / 3 - 4
            y1 = y0 + n / 3 - 4
            jitter = 1.0
            draw.rounded_rectangle([x0 + rng.uniform(-jitter, jitter), y0 + rng.uniform(-jitter, jitter),
                                    x1 + rng.uniform(-jitter, jitter), y1 + rng.uniform(-jitter, jitter)],
                                   radius=2, outline=grout, width=1)
            draw.line([(x0 + 2, y0 + 1), (x1 - 2, y0 + 1)], fill=(178, 176, 170, 120), width=1)
        for _ in range(10):
            x = rng.uniform(2, n - 2)
            y = rng.uniform(2, n - 2)
            draw.point((x, y), fill=(96, 94, 90, 200))
        return pil
    if kind == "water":
        h = fbm(n, seed + 11, octaves=((2, 0.7), (4, 1.0), (8, 0.6), (16, 0.4)))
        gy, gx = np.gradient(h)
        nx = -gx * 3.0
        ny = -gy * 3.0
        nz = np.ones_like(h)
        ln = np.sqrt(nx * nx + ny * ny + nz * nz)
        light = np.array(norm((-0.35, -0.3, 0.9)))
        lam = (nx * light[0] + ny * light[1] + nz * light[2]) / ln
        spec = np.clip(lam, 0, 1) ** 22
        deep = np.array((34, 78, 94), dtype=np.float64) / 255.0
        shallow = np.array((58, 116, 128), dtype=np.float64) / 255.0
        t = np.clip(0.5 + detail * 0.4, 0, 1)
        albedo = deep[None, None, :] * (1 - t[..., None]) + shallow[None, None, :] * t[..., None]
        img = albedo * (0.88 + 0.18 * np.clip(lam, 0, 1))[..., None] + spec[..., None] * np.array([0.55, 0.8, 0.85])
        return Image.fromarray((np.clip(img, 0, 1) * 255).astype(np.uint8)).convert("RGBA")
    raise ValueError(kind)


def ore_tile(size, ore_kind, seed):
    if ore_kind in ("iron", "copper"):
        base = make_ground_tile(size, "dirt" if ore_kind == "copper" else "grass2", seed)
        base = base.filter(ImageFilter.GaussianBlur(0.3))
        arr = np.asarray(base.convert("RGBA"), dtype=np.float64) / 255.0
        pal = (196, 108, 58) if ore_kind == "copper" else (150, 118, 92)
        img, alpha = nugget(size, pal, seed + 21, radius=3.6, count=5)
        for _ in range(3):
            img2, a2 = nugget(size, pal, seed + 30 + _, radius=2.0, count=3)
            mask = a2 > 0.5
            img[mask] = img2[mask]
            alpha = np.maximum(alpha, a2)
        rgb = arr[..., :3]
        rgb = rgb * (1 - alpha[..., None]) + img * alpha[..., None]
        arr[..., :3] = rgb
        return Image.fromarray((np.clip(arr, 0, 1) * 255).astype(np.uint8), "RGBA")
    if ore_kind == "coal":
        base = make_ground_tile(size, "dirt", seed)
        arr = np.asarray(base.convert("RGBA"), dtype=np.float64) / 255.0
        img, alpha = nugget(size, (40, 42, 46), seed + 41, radius=4.0, count=5)
        img *= 1.15
        arr[..., :3] = arr[..., :3] * (1 - alpha[..., None]) + img * alpha[..., None]
        return Image.fromarray((np.clip(arr, 0, 1) * 255).astype(np.uint8), "RGBA")
    if ore_kind == "rock":
        base = make_ground_tile(size, "grass", seed)
        arr = np.asarray(base.convert("RGBA"), dtype=np.float64) / 255.0
        img, alpha = nugget(size, (146, 144, 138), seed + 51, radius=5.5, count=3)
        arr[..., :3] = arr[..., :3] * (1 - alpha[..., None]) + img * alpha[..., None]
        return Image.fromarray((np.clip(arr, 0, 1) * 255).astype(np.uint8), "RGBA")
    if ore_kind == "rocks":
        base = make_ground_tile(size, "grass", seed + 5)
        arr = np.asarray(base.convert("RGBA"), dtype=np.float64) / 255.0
        for i in range(4):
            img, alpha = nugget(size, (140, 138, 132), seed + 60 + i, radius=2.6, count=1)
            arr[..., :3] = arr[..., :3] * (1 - alpha[..., None]) + img * alpha[..., None]
        return Image.fromarray((np.clip(arr, 0, 1) * 255).astype(np.uint8), "RGBA")
    if ore_kind == "depleted":
        base = make_ground_tile(size, "dirt", seed + 3)
        arr = np.asarray(base.convert("RGBA"), dtype=np.float64) / 255.0
        arr[..., :3] *= 0.82
        for i in range(12):
            img, alpha = nugget(size, (110, 106, 100), seed + 70 + i, radius=1.4, count=1)
            arr[..., :3] = arr[..., :3] * (1 - alpha[..., None]) + img * alpha[..., None]
        return Image.fromarray((np.clip(arr, 0, 1) * 255).astype(np.uint8), "RGBA")
    raise ValueError(ore_kind)


TILE_PLAN = [
    ("grass", 100),
    ("grass2", 101),
    ("dirt", 102),
    ("sand", 103),
    ("stone", 104),
    ("water", 105),
    ("iron", 106),
    ("copper", 107),
    ("coal", 108),
    ("rock", 109),
    ("depleted", 110),
    ("rocks", 111),
]


def gen_terrain(tile=32):
    sheet = Image.new("RGBA", (tile * 4, tile * 4), (0, 0, 0, 0))
    for idx, (kind, seed) in enumerate(TILE_PLAN):
        if kind in ("grass", "grass2", "dirt", "sand", "stone", "water"):
            img = make_ground_tile(tile, kind, seed)
        else:
            img = ore_tile(tile, kind, seed)
        sheet.alpha_composite(img, ((idx % 4) * tile, (idx // 4) * tile))
    save(sheet, "terrain.png")
    return sheet


def main():
    os.makedirs(OUT, exist_ok=True)
    args = sys.argv[1:] or ["all"]
    if "terrain" in args or "all" in args:
        gen_terrain()


if __name__ == "__main__":
    main()
