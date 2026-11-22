# meatengine module: 3d

Полноценный 3D-модуль поверх OpenGL 3.3+ (через glad) в стиле движка:
чистый ECS — компоненты это голые данные, вся логика в свободных функциях
и системах. SFML остаётся оконной системой и 2D-рендером; 3D-проход
встраивается в тот же кадр (до 2D-спрайтов).

## Как это работает

- Окно создаётся с depth-буфером (24 bit) и MSAA 4x — см. `MainLoop::init`.
- Система `engine:3d:transform` (Update, priority 3) пересчитывает
  `GlobalTransform3D` по иерархии `hierarchy::cmp::Parent/Children`.
- Система `engine:3d:render` (Render, priority min+2) каждый кадр:
  собирает свет -> собирает видимые меши -> сортирует (opaque по
  программе/текстуре/мешу, transparent назад-вперёд) -> рисует ->
  восстанавливает GL-состояние (`window.resetGLStates()`).
- Для чисто 2D-проектов модуль бесплатен: без `MeshRenderer` и текущей
  `Camera3D` не делается ни одного GL-вызова.

Ограничения дефолтного шейдера: 4 directional + 16 point + 8 spot
источников, Blinn-Phong, туман, альбедо-текстура, unlit-материал.

## Структура (1 файл = 1 задача, ODR)

| Файл | Ответственность |
|---|---|
| `cmp/*.hpp` | компоненты: только данные |
| `Mesh.hpp` | данные меша (`Mesh`, `MeshVertex`) |
| `MeshOps` | операции над CPU-данными меша |
| `MeshGPU` | upload/draw/free GL-буферов |
| `MeshIO` | загрузка .obj |
| `Primitives` | генераторы примитивов |
| `ShaderProgram.hpp` | данные GL-программы + кэш юниформ |
| `ShaderOps` | компиляция/юниформы |
| `Shader3D` | ресурс для ResourceLoader (vert+frag файлы) |
| `DefaultShader` | встроенный forward-шейдер |
| `Math` | glm-хелперы (look_at, оси, кватернионы) |
| `RenderState` | `Scene3D` — per-frame состояние в registry ctx |
| `CameraOps/RendererOps/MaterialOps` | свободные функции над компонентами |
| `three_d` | инициализация GL-лоадера (init-хук) |
| `systems/transform3d` | иерархия GlobalTransform3D |
| `systems/render3d` | весь render-pass |

## C++

```cpp
#include <meatengine/td/three_d.hpp>
#include <meatengine/td/Primitives.hpp>
#include <meatengine/td/RendererOps.hpp>
#include <meatengine/td/MaterialOps.hpp>
#include <meatengine/td/CameraOps.hpp>
#include <meatengine/td/Math.hpp>
#include <meatengine/td/cmp/...>

auto& reg = me::MainLoop::get_registry();

// объект
auto e = reg.create();
auto& t = reg.emplace<me::td::cmp::Transform3D>(e);
t.position = {-3.f, 0.f, 0.f};

auto& r = reg.emplace<me::td::cmp::MeshRenderer>(e);
me::td::renderer_set_mesh(r, me::td::primitives::cube(glm::vec3(1.5f)));
// или из файла (кэшируется ResourceLoader):
// me::td::renderer_set_mesh_path(r, "res/models/sword.obj");

auto& m = reg.emplace<me::td::cmp::Material3D>(e);
m.albedo = {0.9f, 0.3f, 0.25f};

// камера (первая создаётся уже current)
auto cam = reg.create();
auto& ct = reg.emplace<me::td::cmp::Transform3D>(cam);
ct.position = {0.f, 3.5f, 8.f};
ct.rotation = me::td::look_at_rotation(ct.position, {0.f, 0.f, 0.f});
reg.emplace<me::td::cmp::Camera3D>(cam);

// свет
auto sun = reg.create();
auto& dl = reg.emplace<me::td::cmp::DirectionalLight>(sun);
dl.direction = {-0.5f, -1.f, -0.3f};

// настройки сцены
auto& scene = me::td::ensure_scene3d(reg);
scene.clear_color_value = {0.08f, 0.10f, 0.15f};
scene.fog_enabled = true;
```

## Lua (глобальная таблица `td`)

```lua
local e = reg:create()
reg:add_Transform3D(e)
local t = reg:get_Transform3D(e)
t.position = td.vec3(0, 1, 0)

local r = reg:add_MeshRenderer(e)
td.set_mesh(r, td.sphere(1, 32, 16))        -- или: td.set_mesh(r, "res/models/x.obj")
-- r.draw_mode = DrawMode.LINES             -- для td.grid и т.п.

local m = reg:add_Material3D(e)
m.albedo = td.vec3(1, 0.3, 0.2)
td.set_texture(m, "res/textures/brick.png")
m.emissive = 0.2
m.opacity = 1

-- камера/ориентация
td.look_at(t, td.vec3(0, 0, 0))
td.rotate(t, 45, td.vec3(0, 1, 0))
td.make_current(reg, e)

-- свет
local l = reg:add_PointLight(e)
l.color, l.intensity, l.range = td.vec3(1, 0.5, 0.2), 1.2, 10

-- сцена
td.set_clear_color(0.08, 0.10, 0.15)
td.set_fog(td.vec3(0.1, 0.1, 0.15), 20, 100)
td.set_wireframe(false)

-- примитивы: td.cube td.sphere td.plane td.cylinder td.cone td.capsule
--             td.torus td.quad td.grid
-- векторная математика: td.length td.normalize td.dot td.cross
--                       td.distance td.lerp td.axis_angle td.forward td.up td.right
```

Компоненты доступны через стандартный API реестра: `add_Transform3D`,
`get_Camera3D`, `foreach_MeshRenderer`, `has_PointLight` и т.д.

## Зависимости

`glm`, `glad` (добавлены в `vcpkg.json`). Модуль включён в
`MEATENGINE_MODULES` (`external/meatengine/CMakeLists.txt`).
