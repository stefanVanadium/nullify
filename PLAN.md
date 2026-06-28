# PLAN.md — NULLIFY Sprint 1: V0.1 Foundation
> Creat: 2026-06-28 | Status: **Approval Gate: PENDING**

---

## OBIECTIV

O fereastră se deschide. ZERO aleargă, sare, cade fizic. Un nivel simplu JSON se încarcă. Camera urmărește jucătorul cu lerp. Coliziunile funcționează. **Nimic mai mult, nimic mai puțin.**

Acest sprint implementează exact **Luna 1** din GDD roadmap. Fără shadere, fără inamici, fără audio.

---

## DELIVERABLES V0.1

- [ ] Fereastră 1280×720, 144fps target, titlu "NULLIFY v0.1"
- [ ] Game loop cu fixed timestep 60Hz physics + render interpolation alpha
- [ ] ZERO (placeholder rect cyan) se mișcă: stânga/dreapta, sare, cade cu gravitație
- [ ] Coliziuni Box2D cu platfomele din tilemap
- [ ] Nivel `assets/levels/1-1.json` minim: podea + câteva platforme
- [ ] Camera smooth follow (lerp) cu ZERO centrat
- [ ] Build reproductibil: `cmake -B build && cmake --build build`

---

## WAVE ORDER

```
BLD (0) → ENG (1) → ENG Physics + REN Basic (2, paralel) → GP Player + LVL Loader (3, paralel) → QA (4)
```

---

## TASKS

### WAVE 0 — BLD: Infrastructură Build
**Tag: `BLD:`**

- [ ] `BLD:01` — `CMakeLists.txt` complet: SFML 2.6 + Box2D 2.4 + C++20 + Ninja + copy assets post-build
- [ ] `BLD:02` — Structura de directoare: `src/`, `assets/levels/`, `assets/sprites/`, `assets/shaders/`, `assets/fonts/`
- [ ] `BLD:03` — `src/utils/PoolAllocator.h` — `template<typename T, size_t N>`, allocate O(1), free O(1), oldest-reuse la exhaustion
- [ ] `BLD:04` — `src/utils/MathUtils.h` — `Vec2`, `AABB`, `lerp()`, `clamp()`, `lerpAngle()`
- [ ] `BLD:05` — Verificare build curat (zero warnings cu `-Wall -Wextra`)

---

### WAVE 1 — ENG: Core Engine + ECS
**Tag: `ENG:`**

- [ ] `ENG:01` — `src/ecs/Components.h` — **TOATE** componentele POD în un singur fișier:
  - `Transform { float x, y, rotation; }`
  - `Velocity { float vx, vy; }`
  - `Health { int current, max; }`
  - `Renderable { sf::Sprite sprite; int layer; bool visible; }`
  - `Collidable { b2Body* body; float w, h; }`
  - `PlayerTag {}` — marker component

- [ ] `ENG:02` — `src/ecs/World.h/.cpp` — ECS registry cu Struct-of-Arrays:
  - `createEntity() → uint32_t`
  - `destroyEntity(uint32_t)`
  - `addComponent<T>(entity, T&&)`
  - `getComponents<T>() → span<T>` (cache-friendly iteration)
  - `hasComponent<T>(entity) → bool`
  - Max 1024 entități active (fixat la compilare)

- [ ] `ENG:03` — `src/core/EventBus.h/.cpp` — typed event bus:
  - `emit<EventT>(EventT&&)`
  - `on<EventT>(callback)` — returns subscription handle
  - `clear()` — la schimbare de stare
  - Zero heap în emit() — callbacks în vector pre-alocat

- [ ] `ENG:04` — `src/core/InputMap.h/.cpp` — abstracție input:
  - Acțiuni enumerate: `Action::MoveLeft`, `MoveRight`, `Jump`, `Crouch`, `Dash`, `Hack`, `Fire`
  - `isHeld(Action)`, `isPressed(Action)`, `isReleased(Action)`
  - `processEvent(sf::Event&)` — singura conexiune la SFML events
  - Bindings hardcodate în V0.1 (WASD + Mouse)

- [ ] `ENG:05` — `src/core/GameState.h/.cpp` — state machine:
  - Stări: `MENU`, `PLAY`, `HACK`, `PAUSE`, `GAMEOVER`
  - `pushState()`, `popState()`, `replaceState()`
  - Fiecare stare: `enter()`, `update(float dt)`, `render()`, `exit()`

- [ ] `ENG:06` — `src/core/Game.h/.cpp` — main loop:
  ```cpp
  const float FIXED_DT = 1.0f / 60.0f;
  float accumulator = 0.0f;
  while (running) {
      float dt = std::min(clock.restart().asSeconds(), 0.25f);
      accumulator += dt;
      while (accumulator >= FIXED_DT) {
          // physics + AI @ 60Hz
          accumulator -= FIXED_DT;
      }
      float alpha = accumulator / FIXED_DT; // interpolation
      render(alpha);
  }
  ```
  - `sf::RenderWindow` 1280×720, VSync off, framerate limit 144
  - `sf::Event` pump → `InputMap::processEvent()`

---

### WAVE 2 (paralel) — ENG Physics + REN Basic

#### ENG: PhysicsSystem
**Tag: `ENG:`**

- [ ] `ENG:07` — `src/ecs/Systems/PhysicsSystem.h/.cpp`:
  - Deține singurul `b2World` din toată aplicația
  - `update(float fixedDt)` — un singur `b2World::Step()` per apel
  - Sync `b2Body` → `Transform` component după step
  - Gravitație: `b2Vec2(0.0f, -20.0f)` (în sus = y pozitiv în Box2D)
  - Nu expune `b2World*` în afară — nicio altă clasă nu îl accesează
  - Creare body: `createStaticBody(float x, float y, float w, float h)`
  - Creare body: `createDynamicBody(float x, float y, float w, float h) → b2Body*`

#### REN: RenderSystem Basic
**Tag: `REN:`**

- [ ] `REN:01` — `src/rendering/Renderer.h/.cpp`:
  - Primește `sf::RenderWindow&` și `interpolation alpha`
  - Iterează `Renderable` + `Transform` sortat pe `layer`
  - Interpolare vizuală: `drawPos = prevPos + alpha * (currPos - prevPos)`
  - Clear cu culoarea `#050810` (background void din paleta GDD)
  - **Fără SpriteBatch în V0.1** — draw simplu per entitate (optimizare Wave 3+)

---

### WAVE 3 (paralel) — GP Player + LVL Loader

#### GP: Player
**Tag: `GP:`**

- [ ] `GP:01` — `src/player/PlayerConfig.h` — **toate** valorile ca `constexpr`:
  ```cpp
  namespace PlayerConfig {
      constexpr float MOVE_SPEED      = 220.0f;
      constexpr float JUMP_IMPULSE    = 520.0f;
      constexpr float MAX_FALL_SPEED  = -800.0f;
      constexpr float COYOTE_TIME     = 0.12f;
      constexpr float JUMP_BUFFER     = 0.10f;
      constexpr float WIDTH           = 24.0f;
      constexpr float HEIGHT          = 48.0f;
  }
  ```

- [ ] `GP:02` — `src/player/PlayerStateMachine.h/.cpp`:
  - Stări: `IDLE`, `RUN`, `JUMP`, `FALL`, `CROUCH`
  - Tranziții: `InputMap` + contact cu solul din PhysicsSystem
  - Coyote time + jump buffering implementate
  - Emite `PlayerStateChangedEvent` pe tranziție

- [ ] `GP:03` — `src/player/Player.h/.cpp`:
  - Creează entitatea ZERO în `World`
  - Adaugă: `Transform`, `Velocity`, `Health{100,100}`, `Renderable`, `Collidable`, `PlayerTag`
  - `update(float dt)` — citește `InputMap`, aplică forțe pe `b2Body`
  - Placeholder vizual: `sf::RectangleShape` cyan `#00FFEE`, 24×48px

#### LVL: TileMap + Loader + Camera
**Tag: `LVL:`**

- [ ] `LVL:01` — `assets/levels/1-1.json` — nivel minim valid:
  ```json
  {
    "meta": { "id": "1-1", "name": "Streets — Sector Zero", "act": 1,
              "width": 60, "height": 20, "tileSize": 32,
              "tileset": "tileset_01" },
    "layers": {
      "collision": [
        [1,1,1,1,...],  // rândul de jos = podea solidă
        ...             // câteva platforme suspendate
      ]
    },
    "spawns": { "player": { "x": 3, "y": 15 } }
  }
  ```
  - 60×20 tiles @ 32px = 1920×640px nivel
  - Podea completă + 4 platforme suspendate la înălțimi diferite

- [ ] `LVL:02` — `src/world/TileMap.h/.cpp`:
  - Parsare JSON cu nlohmann/json
  - Tile ID 0 = gol, 1 = solid
  - Construiește Box2D edge chains **per rând** (nu per tile individual)
  - Randare: `sf::RectangleShape` gri `#1A2840` per tile solid vizibil

- [ ] `LVL:03` — `src/world/LevelLoader.h/.cpp`:
  - `load(const std::string& path, World&, PhysicsSystem&) → bool`
  - Parsează JSON → creează entități tile + bodies Box2D
  - Returnează `PlayerSpawn` struct cu poziția de start
  - Erori fatale loggate pe stderr, return false

- [ ] `LVL:04` — `src/world/Camera.h/.cpp`:
  - Urmărește entitatea cu `PlayerTag`
  - Lerp: `camPos += (targetPos - camPos) * CAMERA_LERP * dt`
  - `constexpr float CAMERA_LERP = 5.0f`
  - Clamp la limitele nivelului
  - Aplică `sf::View` pe `RenderWindow` înainte de render

---

### WAVE 4 — QA: Validare
**Tag: `QA:`**

- [ ] `QA:01` — Build curat: `cmake -B build -G Ninja && cmake --build build -- -j$(nproc)` fără erori
- [ ] `QA:02` — ZERO apare pe ecran la poziția de spawn din JSON
- [ ] `QA:03` — Mișcare stânga/dreapta funcționează (A/D sau ←/→)
- [ ] `QA:04` — Săritură funcționează (W sau SPACE), coyote time testabil
- [ ] `QA:05` — ZERO cade pe podea și nu cade prin ea
- [ ] `QA:06` — ZERO poate sări pe platformele suspendate și stă pe ele
- [ ] `QA:07` — Camera urmărește ZERO smooth, nu iese din limitele nivelului
- [ ] `QA:08` — FPS counter în titlul ferestrei: `NULLIFY v0.1 | 144 FPS`
- [ ] `QA:09` — Zero heap allocations în game loop (verificat manual cu valgrind --tool=massif sau AddressSanitizer)
- [ ] `QA:10` — Procesul se închide clean la Alt+F4 / ESC

---

## CE NU E ÎN SPRINT 1

Explicit excluse pentru a menține scope-ul clar:

| Feature | Sprint target |
|---|---|
| Shadere GLSL | Sprint 3 |
| Inamici / AI | Sprint 2 |
| Arme / Bullets | Sprint 2 |
| Hacking minigame | Sprint 4 |
| Audio | Sprint 5 |
| HUD (HP bar, etc.) | Sprint 2 |
| Particule | Sprint 3 |
| Parallax | Sprint 3 |
| Animații sprite | Sprint 2 |
| Save system | Sprint 5 |

---

## STRUCTURA FIȘIERELOR CREATĂ ÎN SPRINT 1

```
nullify/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── Game.h/.cpp
│   │   ├── GameState.h/.cpp
│   │   ├── EventBus.h/.cpp
│   │   └── InputMap.h/.cpp
│   ├── ecs/
│   │   ├── World.h/.cpp
│   │   ├── Components.h
│   │   └── Systems/
│   │       └── PhysicsSystem.h/.cpp
│   ├── player/
│   │   ├── Player.h/.cpp
│   │   ├── PlayerConfig.h
│   │   └── PlayerStateMachine.h/.cpp
│   ├── world/
│   │   ├── TileMap.h/.cpp
│   │   ├── LevelLoader.h/.cpp
│   │   └── Camera.h/.cpp
│   ├── rendering/
│   │   └── Renderer.h/.cpp
│   └── utils/
│       ├── PoolAllocator.h
│       └── MathUtils.h
└── assets/
    └── levels/
        └── 1-1.json
```

**Total fișiere noi: ~25**

---

## CRITERII DE SUCCES

Sprintul e considerat terminat când toate task-urile QA:01–10 sunt bifate și se poate face o sesiune de joc de 5 minute fără crash.

---

**Approval Gate: PENDING**
> Scrie `aprobat` pentru a începe execuția.
