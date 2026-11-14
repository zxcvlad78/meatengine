import math

import numpy as np
from PIL import Image, ImageFilter

SS = 4

UP = np.array([0.0, 0.0, 1.0])


def norm(v):
    v = np.asarray(v, dtype=np.float64)
    n = np.linalg.norm(v)
    if n < 1e-12:
        return v
    return v / n


class Camera:
    def __init__(self, elev_deg=62.0, azimuth_deg=-35.0, scale=1.0, center=(0.0, 0.0, 0.0)):
        elev = math.radians(elev_deg)
        az = math.radians(azimuth_deg)
        dir_flat = np.array([math.cos(az), math.sin(az), 0.0])
        self.eye_dir = norm(dir_flat * math.cos(elev) + UP * math.sin(elev))
        self.forward = -self.eye_dir
        self.right = norm(np.cross(self.forward, UP))
        self.up = norm(np.cross(self.right, self.forward))
        self.scale = scale
        self.center = np.asarray(center, dtype=np.float64)

    def project(self, pts):
        rel = pts - self.center
        x = rel @ self.right
        y = rel @ self.up
        z = rel @ self.forward
        return np.stack([x, y, z], axis=-1) * self.scale

    def to_screen(self, cam_pts, width, height):
        sx = cam_pts[..., 0] + width * 0.5
        sy = height * 0.5 - cam_pts[..., 1]
        return np.stack([sx, sy, cam_pts[..., 2]], axis=-1)


class Mesh:
    def __init__(self):
        self.verts = []
        self.faces = []
        self.mats = []

    def mat(self, color, spec=0.25, shin=24.0, emissive=0.0, alpha=None):
        self.mats.append({
            "color": np.array(color, dtype=np.float64) / 255.0 if max(color) > 1.0 else np.array(color, dtype=np.float64),
            "spec": spec,
            "shin": shin,
            "emissive": emissive,
            "alpha": alpha,
        })
        return len(self.mats) - 1

    def add_face(self, idxs, mat):
        self.faces.append((tuple(idxs), mat))

    def add_quad(self, a, b, c, d, mat):
        base = len(self.verts)
        self.verts.extend([a, b, c, d])
        self.faces.append(((base, base + 1, base + 2), mat))
        self.faces.append(((base, base + 2, base + 3), mat))

    def add_tri(self, a, b, c, mat):
        base = len(self.verts)
        self.verts.extend([a, b, c])
        self.faces.append(((base, base + 1, base + 2), mat))

    def add_poly(self, pts, mat, double=False):
        base = len(self.verts)
        self.verts.extend(pts)
        for i in range(1, len(pts) - 1):
            self.faces.append(((base, base + i, base + i + 1), mat))
            if double:
                self.faces.append(((base, base + i + 1, base + i), mat))


def _quad(m, p0, p1, p2, p3, mat):
    m.add_quad(p0, p1, p2, p3, mat)


def octagon(cx, cy, r, rot=math.pi / 8):
    return [(cx + r * math.cos(rot + i * math.pi / 4), cy + r * math.sin(rot + i * math.pi / 4)) for i in range(8)]


def add_prism(m, cx, cy, z0, z1, radius, mat, sides=8, rot=math.pi / 8, top_mat=None, bottom=False):
    pts = [(cx + radius * math.cos(rot + i * 2 * math.pi / sides),
            cy + radius * math.sin(rot + i * 2 * math.pi / sides)) for i in range(sides)]
    bottom_pts = [(x, y, z0) for (x, y) in pts]
    top_pts = [(x, y, z1) for (x, y) in pts]
    for i in range(sides):
        j = (i + 1) % sides
        _quad(m, bottom_pts[i], bottom_pts[j], top_pts[j], top_pts[i], mat)
    m.add_poly(top_pts, mat if top_mat is None else top_mat)
    if bottom:
        m.add_poly(list(reversed(bottom_pts)), mat)


def add_box(m, cx, cy, z0, z1, sx, sy, mat, chamfer=0.0, top_mat=None, bottom=False):
    if chamfer <= 0:
        x0, x1 = cx - sx / 2, cx + sx / 2
        y0, y1 = cy - sy / 2, cy + sy / 2
        low = [(x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0)]
        high = [(x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)]
        for i in range(4):
            j = (i + 1) % 4
            _quad(m, low[i], low[j], high[j], high[i], mat)
        m.add_quad(high[0], high[1], high[2], high[3], mat if top_mat is None else top_mat)
        if bottom:
            m.add_quad(low[3], low[2], low[1], low[0], mat)
        return
    c = chamfer
    x0, x1 = cx - sx / 2, cx + sx / 2
    y0, y1 = cy - sy / 2, cy + sy / 2
    low = [(x0 + c, y0, z0), (x1 - c, y0, z0), (x1, y0 + c, z0), (x1, y1 - c, z0),
           (x1 - c, y1, z0), (x0 + c, y1, z0), (x0, y1 - c, z0), (x0, y0 + c, z0)]
    high = [(x0 + c, y0, z1), (x1 - c, y0, z1), (x1, y0 + c, z1), (x1, y1 - c, z1),
            (x1 - c, y1, z1), (x0 + c, y1, z1), (x0, y1 - c, z1), (x0, y0 + c, z1)]
    for i in range(8):
        j = (i + 1) % 8
        _quad(m, low[i], low[j], high[j], high[i], mat)
    m.add_poly(high, mat if top_mat is None else top_mat)
    if bottom:
        m.add_poly(list(reversed(low)), mat)


def add_cylinder(m, cx, cy, z0, z1, radius, mat, sides=16):
    pts = [(cx + radius * math.cos(i * 2 * math.pi / sides),
            cy + radius * math.sin(i * 2 * math.pi / sides)) for i in range(sides)]
    low = [(x, y, z0) for (x, y) in pts]
    high = [(x, y, z1) for (x, y) in pts]
    for i in range(sides):
        j = (i + 1) % sides
        _quad(m, low[i], low[j], high[j], high[i], mat)
    m.add_poly(high, mat)
    m.add_poly(list(reversed(low)), mat)


def add_gear(m, cx, cy, z0, z1, radius, teeth, mat, body_mat=None, hole=0.35, tooth_frac=0.55):
    total = teeth * 4
    outer = []
    for i in range(total):
        k = i % 4
        r = radius if k < 2 else radius * (1.0 - 0.22 * tooth_frac)
        if k == 0 or k == 3:
            r = radius * 0.78
        ang = i * 2 * math.pi / total
        outer.append((cx + r * math.cos(ang), cy + r * math.sin(ang)))
    low = [(x, y, z0) for (x, y) in outer]
    high = [(x, y, z1) for (x, y) in outer]
    for i in range(total):
        j = (i + 1) % total
        _quad(m, low[i], low[j], high[j], high[i], mat)
    m.add_poly(high, mat if body_mat is None else body_mat)
    m.add_poly(list(reversed(low)), mat)
    if hole > 0:
        add_cylinder(m, cx, cy, z1 * 0.35, z1 + 0.01, radius * hole, mat, sides=12)


def add_ring(m, cx, cy, z0, z1, r_out, r_in, mat, sides=20):
    o = [(cx + r_out * math.cos(i * 2 * math.pi / sides), cy + r_out * math.sin(i * 2 * math.pi / sides)) for i in range(sides)]
    inn = [(cx + r_in * math.cos(i * 2 * math.pi / sides), cy + r_in * math.sin(i * 2 * math.pi / sides)) for i in range(sides)]
    for i in range(sides):
        j = (i + 1) % sides
        _quad(m, (o[i][0], o[i][1], z1), (o[j][0], o[j][1], z1), (o[j][0], o[j][1], z0), (o[i][0], o[i][1], z0), mat)
        _quad(m, (inn[i][0], inn[i][1], z1), (inn[j][0], inn[j][1], z0), (inn[j][0], inn[j][1], z1), (inn[i][0], inn[i][1], z0), mat)
    top = []
    for i in range(sides):
        j = (i + 1) % sides
        top.extend([(o[i][0], o[i][1], z1), (o[j][0], o[j][1], z1), (inn[j][0], inn[j][1], z1), (inn[i][0], inn[i][1], z1)])
    m.add_poly([(o[i][0], o[i][1], z1) for i in range(sides)], mat)


def add_stripes(m, x0, y0, x1, y1, z, width, mat_a, mat_b, horizontal=True):
    if horizontal:
        yy = y0
        k = 0
        while yy < y1:
            yy2 = min(yy + width, y1)
            mat = mat_a if k % 2 == 0 else mat_b
            m.add_quad((x0, yy, z), (x1, yy, z), (x1, yy2, z), (x0, yy2, z), mat)
            yy = yy2
            k += 1
    else:
        xx = x0
        k = 0
        while xx < x1:
            xx2 = min(xx + width, x1)
            mat = mat_a if k % 2 == 0 else mat_b
            m.add_quad((xx, y0, z), (xx2, y0, z), (xx2, y1, z), (xx, y1, z), mat)
            xx = xx2
            k += 1


def add_bolts(m, points, z, r, mat, sides=6):
    for (x, y) in points:
        add_cylinder(m, x, y, z, z + r * 1.4, r, mat, sides=sides)


def add_grille(m, cx, cy, z, sx, sy, slots, mat):
    step = sy / slots
    for i in range(slots):
        y = cy - sy / 2 + step * (i + 0.5)
        add_box(m, cx, y, z, z + 0.4, sx, step * 0.45, mat)


class GroundPlane:
    def __init__(self, size, color, elev=90.0):
        self.size = size
        self.color = np.array(color, dtype=np.float64) / 255.0
        self.elev = elev


class Scene:
    def __init__(self, width, height, camera):
        self.width = width
        self.height = height
        self.camera = camera
        self.objects = []

    def add(self, mesh):
        self.objects.append(mesh)

    def _collect(self):
        verts = []
        faces = []
        mats = []
        offset = 0
        for mesh in self.objects:
            arr = np.asarray(mesh.verts, dtype=np.float64)
            verts.append(arr)
            for (idxs, mat) in mesh.faces:
                faces.append(((idxs[0] + offset, idxs[1] + offset, idxs[2] + offset), mat))
            mats.extend(mesh.mats)
            offset += len(mesh.verts)
        if not verts:
            return np.zeros((0, 3)), [], mats
        return np.concatenate(verts, axis=0), faces, mats


def _rasterize(screen, zvals, colors, width, height, zbuf, cbuf, cov):
    n = len(screen)
    for i in range(n):
        tri = screen[i]
        x0 = max(int(math.floor(tri[:, 0].min())), 0)
        x1 = min(int(math.ceil(tri[:, 0].max())) + 1, width)
        y0 = max(int(math.floor(tri[:, 1].min())), 0)
        y1 = min(int(math.ceil(tri[:, 1].max())) + 1, height)
        if x1 <= x0 or y1 <= y0:
            continue
        xs = np.arange(x0, x1, dtype=np.float64) + 0.5
        ys = np.arange(y0, y1, dtype=np.float64) + 0.5
        gx, gy = np.meshgrid(xs, ys)
        ax, ay = tri[0, 0], tri[0, 1]
        bx, by = tri[1, 0], tri[1, 1]
        cx, cy = tri[2, 0], tri[2, 1]
        denom = (by - cy) * (ax - cx) + (cx - bx) * (ay - cy)
        if abs(denom) < 1e-9:
            continue
        w0 = ((by - cy) * (gx - cx) + (cx - bx) * (gy - cy)) / denom
        w1 = ((cy - ay) * (gx - cx) + (ax - cx) * (gy - cy)) / denom
        w2 = 1.0 - w0 - w1
        inside = (w0 >= -1e-6) & (w1 >= -1e-6) & (w2 >= -1e-6)
        if not inside.any():
            continue
        z = w0 * tri[0, 2] + w1 * tri[1, 2] + w2 * tri[2, 2]
        sub_z = zbuf[y0:y1, x0:x1]
        sub_c = cbuf[y0:y1, x0:x1]
        sub_cov = cov[y0:y1, x0:x1]
        mask = inside & (z < sub_z)
        if not mask.any():
            continue
        sub_z[mask] = z[mask]
        sub_c[mask] = colors[i]
        sub_cov[mask] = True


def _shade(normal, mats, mat_id, light, ambient=0.42):
    mat = mats[mat_id]
    base = mat["color"]
    ndl = max(float(np.dot(normal, light)), 0.0)
    fill = max(float(np.dot(normal, -light)), 0.0)
    diff = ambient + 0.72 * ndl + 0.14 * fill
    h = norm(light + np.array([0.0, 0.0, 1.0]))
    spec = mat["spec"] * (max(float(np.dot(normal, h)), 0.0) ** mat["shin"])
    tint = np.array([1.03, 1.0, 0.96])
    col = base * diff * tint + spec * np.array([1.0, 0.99, 0.95])
    if mat["emissive"] > 0:
        col = col * (1.0 - mat["emissive"]) + base * mat["emissive"] * 1.25
    return np.clip(col, 0.0, 1.0)


def render(scene, light_dir=(-0.45, -0.55, 0.75), shadow=True, shadow_strength=0.38,
           ground=None, ground_color=None, edge_boost=0.0, ambient=0.32):
    W = scene.width * SS
    H = scene.height * SS

    cbuf = np.zeros((H, W, 3), dtype=np.float64)
    zbuf = np.full((H, W), 1e9, dtype=np.float64)
    cov = np.zeros((H, W), dtype=bool)

    verts, faces, mats = scene._collect()
    cam = scene.camera
    cam_pts = cam.project(verts)
    scr = cam.to_screen(cam_pts, W, H)

    light = norm(light_dir)

    if ground_color is not None:
        cbuf[:, :] = np.array(ground_color, dtype=np.float64) / 255.0
        cov[:, :] = True

    if shadow and len(verts):
        sh = []
        for (idxs, mat_id) in faces:
            tri = verts[list(idxs)]
            proj = []
            for p in tri:
                proj.append((p[0], p[1], 0.0))
            sh.append(np.asarray(proj))
        shadow_screen = cam.to_screen(cam.project(np.asarray(sh).reshape(-1, 3)), W, H).reshape(-1, 3, 3)
        sw = W
        shbuf = np.zeros((H, sw, 4), dtype=np.float64)
        scov = np.zeros((H, sw), dtype=bool)
        for tri in shadow_screen:
            x0 = max(int(math.floor(tri[:, 0].min())), 0)
            x1 = min(int(math.ceil(tri[:, 0].max())) + 1, sw)
            y0 = max(int(math.floor(tri[:, 1].min())), 0)
            y1 = min(int(math.ceil(tri[:, 1].max())) + 1, H)
            if x1 <= x0 or y1 <= y0:
                continue
            xs = np.arange(x0, x1, dtype=np.float64) + 0.5
            ys = np.arange(y0, y1, dtype=np.float64) + 0.5
            gx, gy = np.meshgrid(xs, ys)
            ax, ay = tri[0, 0], tri[0, 1]
            bx, by = tri[1, 0], tri[1, 1]
            cx, cy = tri[2, 0], tri[2, 1]
            denom = (by - cy) * (ax - cx) + (cx - bx) * (ay - cy)
            if abs(denom) < 1e-9:
                continue
            w0 = ((by - cy) * (gx - cx) + (cx - bx) * (gy - cy)) / denom
            w1 = ((cy - ay) * (gx - cx) + (ax - cx) * (gy - cy)) / denom
            w2 = 1.0 - w0 - w1
            inside = (w0 >= -1e-6) & (w1 >= -1e-6) & (w2 >= -1e-6)
            scov[y0:y1, x0:x1] |= inside
        scr_shadow = np.zeros((H, sw, 4), dtype=np.float64)
        scr_shadow[..., 3] = scov.astype(np.float64) * shadow_strength
        scr_shadow[..., 0:3] = 0.04
        shadow_img = Image.fromarray((scr_shadow * 255.0).astype(np.uint8), "RGBA").filter(ImageFilter.GaussianBlur(1.6 * SS))
        shadow_arr = np.asarray(shadow_img, dtype=np.float64) / 255.0
        alpha = shadow_arr[..., 3:4]
        cbuf = cbuf * (1.0 - alpha) + shadow_arr[..., 0:3] * alpha
        cov[:, :] |= (alpha[..., 0] > 0.01)

    for face_index, (idxs, mat_id) in enumerate(faces):
        tri2d = scr[list(idxs)]
        n = np.cross(verts[idxs[1]] - verts[idxs[0]], verts[idxs[2]] - verts[idxs[0]])
        ln = np.linalg.norm(n)
        if ln < 1e-9:
            continue
        n = n / ln
        if n[2] < 0:
            n = -n
        col = _shade(n, mats, mat_id, light, ambient)
        variation = 1.0 + 0.04 * ((((face_index * 2654435761) % 1000) / 1000.0) - 0.5)
        col = np.clip(col * variation, 0.0, 1.0)
        _rasterize([tri2d], [None], [col], W, H, zbuf, cbuf, cov)

    rgba = np.zeros((H, W, 4), dtype=np.float64)
    rgba[..., 0:3] = cbuf
    rgba[..., 3] = cov.astype(np.float64)
    if ground_color is None:
        rgba[..., 0:3] = cbuf
    img = Image.fromarray((np.clip(rgba, 0, 1) * 255.0).astype(np.uint8), "RGBA")
    img = img.resize((scene.width, scene.height), Image.LANCZOS)
    return img


def flatten(mesh, color=(10, 12, 16)):
    out = Mesh()
    mat = out.mat(color, spec=0.0, emissive=0.0)
    for v in mesh.verts:
        out.verts.append((v[0], v[1], 0.0))
    for (idxs, _m) in mesh.faces:
        out.faces.append((tuple(idxs), mat))
    out.mats = [out.mats[0]]
    return out


def outline(img, color=(14, 14, 18, 255)):
    alpha = img.getchannel("A")
    dilated = alpha.filter(ImageFilter.MaxFilter(3))
    layer = Image.new("RGBA", img.size, color[:3] + (0,))
    layer.putalpha(dilated)
    layer.alpha_composite(img)
    merged = np.maximum(np.asarray(alpha), np.asarray(dilated))
    layer.putalpha(Image.fromarray(merged, "L"))
    return layer


def compose(dst, src, x, y):
    dst.alpha_composite(src, (x, y))
    return dst


def save(img, path):
    img.save(path)
