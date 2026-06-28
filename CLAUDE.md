# CLAUDE.md — NULLIFY
> Vanadium Systems / Stefan — 2D Cyberpunk Side-Scroller Shooter
> Read this file at the start of every session. Do not ask for context already covered here.

---

## WHAT THIS IS

A 2D side-scroller shooter with hacking mechanics, stealth, and ragdoll physics.
Written in C++20 from scratch — no engine, no framework beyond SFML + Box2D.
Platform: PC (Windows + Linux). Target: 144fps @ 1080p, 60fps @ 4K.
Status: **Pre-production**.

**Dev:** Stefan / Vanadium Systems
**Pitch:** HACK · SHOOT · VANISH
**GDD:** `NULLIFY_GDD.md` — authoritative game design spec, read it before any feature work.

---

## STACK

| Layer | Technology |
|---|---|
| Language | C++20 |
| Rendering | SFML 2.6 (OpenGL backend) |
| Physics | Box2D 2.4 |
| Shaders | GLSL 3.3 |
| Level format | JSON — nlohmann/json 3.11 |
| Build | CMake 3.20+ + Ninja |
| Architecture | ECS (Entity Component System), data-oriented design |
| Platform | Linux (dev), Windows (distribution) |

---

## DOMAIN GLOSSARY

| Term | Meaning |
|---|---|
| `entity` | An integer ID (uint32_t) in the ECS world — not a class |
| `component` | A POD struct attached to an entity — no methods, no logic |
| `system` | A class that owns behavior and iterates over component arrays |
| `world` | The ECS registry — holds all component arrays |
| `pool` | Pre-allocated fixed-size allocator — bullets, particles, enemies |
| `hot path` | Code that runs every frame — zero allocation, zero virtual dispatch here |
| `fixed step` | Physics and AI update at exactly 60 Hz regardless of render rate |
| `interpolation alpha` | Fraction of fixed step elapsed, used by renderer to smooth motion |
| `hackable` | Any in-world object the player can hack (terminal, camera, door) |
| `alert level` | Global tension state 0–3, affects all enemy AI |
| `cover point` | A registered position behind an obstacle where enemies take shelter |
| `navmesh` | Passable node graph derived from tilemap, used by A* pathfinding |
| `stem` | One audio track layer in the adaptive music system |
| `tile` | A 32×32px cell in the tilemap |
| `trigger zone` | An invisible rect in a level that fires an event when player enters |
| `ZERO` | The player character |
| `VEKTOR` | The antagonist corporation |
| `NEXUS-7` | The game world city, 2087 |

---

## ARCHITECTURE RULES — NON-NEGOTIABLE

1. **ECS is canonical** — all game objects are entities with components. No inheritance hierarchies for game objects. No "PlayerClass" with methods.
2. **Components are POD** — structs only: no constructors, no methods, no virtual. If you add behavior to a component, move it to a system.
3. **Systems own all logic** — systems iterate over component arrays. A system may not hold a reference to another system.
4. **EventBus for cross-system communication** — no direct system-to-system calls ever. Emit events, subscribe in constructors.
5. **Zero heap allocation in the game loop** — no `new`, `delete`, `make_shared`, `make_unique`, `vector::push_back` (if it reallocates) inside `update()` or `render()`. Use pool allocators.
6. **No virtual dispatch in hot path** — no `virtual` on anything called per frame.
7. **Physics isolation** — `b2World` lives only in `PhysicsSystem`. No other code accesses it directly.
8. **Fixed timestep** — physics and AI update at 60 Hz. Renderer interpolates. Delta time clamped to 250ms.
9. **InputMap is the only input source** — no `sf::Keyboard::isKeyPressed()` in gameplay code.
10. **Pool exhaustion handled gracefully** — never crash when a pool is full; reuse oldest entry.

---

## FOLDER STRUCTURE

```
nullify/
├── src/
│   ├── core/
│   │   ├── Game.cpp/.h           # main loop, clock, state dispatch
│   │   ├── GameState.cpp/.h      # state machine (MENU/PLAY/HACK/PAUSE/GAMEOVER)
│   │   └── EventBus.cpp/.h       # typed event bus
│   ├── ecs/
│   │   ├── World.cpp/.h          # entity registry, component arrays (SoA)
│   │   ├── Components.h          # ALL POD component definitions in one place
│   │   └── Systems/              # PhysicsSystem, RenderSystem, etc.
│   ├── player/
│   │   ├── Player.cpp/.h
│   │   ├── PlayerStateMachine.cpp/.h
│   │   ├── PlayerConfig.h        # all constexpr tuning values
│   │   ├── WeaponSystem.cpp/.h
│   │   └── HackSystem.cpp/.h
│   ├── hacking/
│   │   └── HackMinigame + Tier1/2/3
│   ├── enemies/
│   │   ├── Enemy.cpp/.h
│   │   ├── EnemyConfig.h         # all constexpr enemy values
│   │   ├── AIStateMachine.cpp/.h
│   │   ├── Pathfinding.cpp/.h    # A* on NavMesh
│   │   ├── NavMesh.cpp/.h
│   │   ├── AlertSystem.cpp/.h
│   │   └── EnemyManager.cpp/.h
│   ├── world/
│   │   ├── TileMap.cpp/.h
│   │   ├── LevelLoader.cpp/.h    # JSON → entities, Box2D bodies, NavMesh
│   │   ├── Camera.cpp/.h
│   │   └── TriggerSystem.cpp/.h
│   ├── rendering/
│   │   ├── Renderer.cpp/.h
│   │   ├── SpriteBatch.cpp/.h
│   │   ├── ShaderManager.cpp/.h
│   │   ├── ParticleSystem.cpp/.h
│   │   └── ParallaxSystem.cpp/.h
│   ├── audio/
│   │   ├── AudioManager.cpp/.h
│   │   ├── AdaptiveMusicSystem.cpp/.h
│   │   └── FootstepSystem.cpp/.h
│   ├── ui/
│   │   ├── UIManager.cpp/.h
│   │   ├── HUD.cpp/.h
│   │   ├── MainMenu.cpp/.h
│   │   ├── PauseMenu.cpp/.h
│   │   └── HackingUI.cpp/.h
│   └── utils/
│       ├── PoolAllocator.h       # template<T, N> — zero heap
│       └── MathUtils.h           # Vec2, AABB, lerp, clamp
├── assets/
│   ├── sprites/                  # TexturePacker atlases
│   ├── shaders/                  # .frag GLSL files
│   ├── audio/music/stems/        # stems for adaptive music
│   ├── audio/sfx/                # weapon, footstep, impact, ui
│   ├── audio/ambient/            # looping ambient tracks
│   ├── levels/                   # JSON level files (1-1.json, etc.)
│   └── fonts/                    # ShareTechMono, Orbitron
├── docs/
│   ├── ARCHITECTURE.md
│   ├── API.md                    # EventBus events + component catalogue
│   └── LEVEL_DESIGN.md
├── .github/agents/               # agent system — see below
├── NULLIFY_GDD.md                # authoritative game design document
├── CHANGELOG.md
└── CMakeLists.txt
```

---

## NAMING CONVENTIONS

- **Files:** `PascalCase.cpp/.h` for classes, `camelCase.h` for header-only utilities
- **Types / Classes:** `PascalCase` — `PhysicsSystem`, `EnemyConfig`, `PoolAllocator`
- **Functions / variables:** `camelCase` — `updatePhysics()`, `alertLevel`
- **Member variables:** `m_camelCase` — `m_position`, `m_currentState`
- **Constants:** `SCREAMING_SNAKE_CASE` — `MAX_ENEMIES`, `FIXED_DT`, `JUMP_IMPULSE`
- **Events:** `PascalCase` + `Event` suffix — `EnemyDiedEvent`, `HackSuccessEvent`
- **Components:** `PascalCase` no suffix — `Transform`, `Health`, `Renderable`
- **All code:** English only. Comments only when WHY is non-obvious.
- **No magic numbers** — every numeric constant goes in `*Config.h` as `constexpr`

---

## PERFORMANCE BUDGETS — HARD LIMITS

| Metric | Target | Blocking threshold |
|---|---|---|
| FPS @ 1080p | 144 | < 120 |
| FPS @ 4K | 60 | < 50 |
| Draw calls / frame | ≤ 10 | > 15 |
| Heap allocs in game loop | 0 | > 0 |
| Max enemies active | 32 | — |
| Bullet pool | 1024 slots | exhaustion = reuse oldest |
| Particle pool | 4096 slots | exhaustion = reuse oldest |
| Physics step | fixed 60 Hz | — |
| A* recalculations / frame | ≤ 4 | — |
| LOS raycasts / frame | ≤ 8 | — |
| Shader uniforms | set once/frame per shader | not per entity |

Any feature that risks these budgets must include a mitigation plan before implementation.

---

## VISUAL IDENTITY

Dark cyberpunk neon-noir. These rules are always active:

**Color palette — only these 10:**
| Role | Hex |
|---|---|
| Background void | `#050810` |
| Neon cyan (primary) | `#00FFEE` |
| Neon magenta | `#FF006B` |
| Neon yellow | `#FFE600` |
| Blood / danger | `#FF0038` |
| Hack violet | `#AA00FF` |
| UI base | `#0A1020` |
| UI border | `#1A2840` |
| Text primary | `#E0EEF8` |
| Text dim | `#6080A0` |

**Fonts:** `ShareTechMono-Regular.ttf` for HUD/data, `Orbitron-Regular.ttf` for titles/narrative.
**No gradients** except HP bar fill (cyan→magenta at low HP).
**No drop shadows** — use border offset.
**Animations:** < 300ms, ease-out, start from scale(0.95) or opacity 0.
**Min HUD font size:** 16px. **Min menu touch target:** 44×44px.

---

## SHADERS (GLSL 3.3)

| File | Trigger |
|---|---|
| `neon_glow.frag` | Always on — bloom on bright pixels |
| `chromatic_aberration.frag` | On player damage — 0.8s ease-out decay |
| `scanlines.frag` | Always on — subtle CRT feel |
| `glitch.frag` | Hacking mode — full intensity on enter, 0.2 during, 0.0 on exit |
| `rain.frag` | Gameplay layers — parallax rain |
| `vignette.frag` | Always on — intensifies at low HP |

Shaders compiled at startup only. Uniforms set once per frame per shader.

---

## AUDIO RULES

- Adaptive music: 5 stems, volume crossfaded per state (STEALTH / COMBAT / HACKING / BOSS)
- SFX pool: 32 concurrent `sf::Sound` slots — steal oldest when full, never block
- Weapon SFX: 3–5 variants per weapon, random selection, never same variant twice in a row
- Footsteps: material-aware (metal / concrete / water / wood), interval varies by speed
- Spatial audio: `sf::Sound::setRelativeToListener(false)`, falloff beyond ~600px
- Pitch variation: ±5% on weapons, ±10% on footsteps, none on UI sounds
- All music authored as stems that loop at bar boundaries — no cross-fade glitches

---

## LEVEL FORMAT

Every level is `assets/levels/<act>-<num>.json`. Key sections:
```json
{ "meta": { "id", "name", "act", "width", "height", "tileSize", "tileset" },
  "layers": { "decorative", "gameplay", "collision", "material" },
  "spawns": { "player", "enemies": [...], "items": [...] },
  "triggers": [...],
  "hackables": [...],
  "coverObjects": [...],
  "cameraRegions": [...],
  "ambientConfig": { "loops": [...], "parallaxTheme": "..." } }
```
Full schema in `docs/LEVEL_DESIGN.md`. Tile IDs 0–10 defined, 11–15 reserved.
Box2D bodies built from collision layer as **edge chains per row**, not one body per tile.

---

## AGENT SYSTEM

All agents live in `.github/agents/`. Read the relevant agent file before implementing in that domain.

| Agent file | Domain | Tag in PLAN.md |
|---|---|---|
| `planner.agent.md` | Architect — plans only, no code | — |
| `engine.agent.md` | Core, ECS, game loop, physics, memory | `ENG:` |
| `gameplay.agent.md` | Player movement, weapons, hacking, stealth | `GP:` |
| `ai.agent.md` | Enemy AI, pathfinding, perception | `AI:` |
| `rendering.agent.md` | SFML pipeline, shaders, particles, parallax | `REN:` |
| `audio.agent.md` | Music system, SFX, spatial audio | `AUD:` |
| `level.agent.md` | Level JSON, tilemap loader, camera, triggers | `LVL:` |
| `ui.agent.md` | HUD, menus, hacking minigame UI | `UI:` |
| `build.agent.md` | CMake, dependencies, packaging | `BLD:` |
| `qa.agent.md` | Build validation, test matrix, profiling | `QA:` |
| `codereview.agent.md` | C++20 quality, ECS discipline, perf review | `CR:` |
| `docs.agent.md` | GDD sync, CHANGELOG, architecture docs | `DOC:` |

**Planning (default on any non-trivial feature):** Read PLAN.md → read relevant GDD section → write plan in PLAN.md → finish with `Approval Gate: PENDING`. Do not implement until `aprobat`.

**Execution wave order:** ENG → GP/AI/REN (parallel) → LVL/UI/AUD → BLD/QA → CR/DOC.
Mark `[x]` on each completed task. On full completion: append checkpoint to MISSION_CONTROL.md, clear PLAN.md.
If MISSION_CONTROL.md > 200 lines → move oldest checkpoint to `ARCHIVE/legacy.md`.

---

## SPRINT PROTOCOL

| Keyword | Meaning |
|---|---|
| `aprobat` | Start execution |
| `modificat: <text>` | Revise plan, reset gate |
| `anulat` | Cancel, clear PLAN.md |
| `task terminat` | Sprint complete |
| `task in asteptare: astept <domain>` | Blocked |

---

## CODING PATTERNS

### ECS Component Definition
```cpp
// Components.h — ALL components here, POD only
struct Transform  { float x, y, rotation; };
struct Velocity   { float vx, vy; };
struct Health     { int current, max; };
struct Renderable { sf::Sprite sprite; int layer; };
struct Bullet     { float travelDist; int penetration; bool ricochet; };
```

### System Update — Cache-Friendly Iteration
```cpp
void PhysicsSystem::update(float dt) {
    auto& transforms = m_world.getComponents<Transform>();
    auto& velocities  = m_world.getComponents<Velocity>();
    for (size_t i = 0; i < count; ++i) {
        transforms[i].x += velocities[i].vx * dt;
        transforms[i].y += velocities[i].vy * dt;
    }
}
```

### Pool Allocator — Zero Heap in Game Loop
```cpp
PoolAllocator<Bullet,   1024> bulletPool;
PoolAllocator<Particle, 4096> particlePool;

// Allocate O(1), no malloc
Bullet* b = bulletPool.allocate();
// Free O(1)
bulletPool.free(b);
```

### EventBus — Cross-System Communication
```cpp
// Emit (in AIStateMachine)
EventBus::emit(EnemyDiedEvent{ .entityId = id, .position = pos });

// Subscribe (in ParticleSystem constructor)
EventBus::on<EnemyDiedEvent>([this](const EnemyDiedEvent& e) {
    spawnBloodEffect(e.position);
});
```

### Game Loop — Fixed Timestep
```cpp
const float FIXED_DT = 1.0f / 60.0f;
float accumulator = 0.0f;
while (running) {
    float dt = std::min(clock.restart().asSeconds(), 0.25f);
    accumulator += dt;
    while (accumulator >= FIXED_DT) {
        physicsSystem.update(FIXED_DT);
        aiSystem.update(FIXED_DT);
        accumulator -= FIXED_DT;
    }
    renderSystem.render(accumulator / FIXED_DT);  // interpolation alpha
}
```

### Config Constants — Never Magic Numbers
```cpp
// PlayerConfig.h
namespace PlayerConfig {
    constexpr float MOVE_SPEED    = 220.0f;
    constexpr float JUMP_IMPULSE  = 520.0f;
    constexpr float DASH_SPEED    = 600.0f;
    constexpr float DASH_DURATION = 0.18f;
    constexpr float COYOTE_TIME   = 0.12f;
}
```

---

## WHAT NOT TO DO

- NEVER put behavior (methods) in a component struct
- NEVER call one system directly from another — use EventBus
- NEVER access `b2World` outside `PhysicsSystem`
- NEVER allocate on the heap inside `update()` or `render()`
- NEVER use `virtual` on anything in the hot path
- NEVER hardcode a numeric constant in game logic — use `*Config.h`
- NEVER read `sf::Keyboard` directly in gameplay code — use `InputMap`
- NEVER add a draw call that pushes total above 10/frame without Planner sign-off
- NEVER compile shaders at runtime — only at startup in `ShaderManager::loadAll()`
- NEVER use colors outside the 10-color palette in UI or HUD
- NEVER commit `build/` or `build_rel/` directories

---

*© 2026 Vanadium Systems / Stefan. All rights reserved.*
