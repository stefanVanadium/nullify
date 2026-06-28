# PLAN.md — NULLIFY Sprint 2
> Status: **EXECUTAT — Build curat, QA în așteptare**
> Bază: Sprint 1 complet (commit d79ca2b) · GDD v0.1

---

## CONTEXT & PROBLEME MOȘTENITE

### Blocker structural: Draw Call Budget
**Situație:** Fiecare tile din TileMap este desenat ca un `sf::Sprite` separat → N tiles vizibile = N draw calls. Cu un nivel de 60×20 tiles și o fereastră de ~30×20 tiles vizibile, sunt ~600 draw calls per frame. Budget: **≤ 10 draw calls/frame** (hard limit CLAUDE.md).

**Cauza:** `SpriteBatch` figurează în structura proiectului (CLAUDE.md) dar **nu există** în `src/` și `Renderer.cpp` nu are nicio referință la el. Tiles sunt randate individual.

**Impact:** Bottleneck structural care va agrava orice adăugare de enemies, particles, shaders. Trebuie rezolvat **înaintea** oricărei noi funcționalități de rendering.

**Soluție:** Implementare `SpriteBatch` (vertex array batching pe texture atlas) + integrare în `Renderer` + `TileMap`.

---

## OBIECTIVE SPRINT 2

Sprint 2 livrează primul loop de gameplay funcțional:
- ZERO poate **trage** cu PHANTOM-9
- Există cel puțin un **SCOUT** inamic cu AI funcțional (PATROL → ALERT → COMBAT)
- Rendering-ul **nu depășește bugetul** de 10 draw calls
- Există un **HUD minimal** (HP + ammo)
- Există cel puțin **2 shadere** funcționale (scanlines permanent + vignette)

---

## WAVE 1 — ENG (prerequisit pentru tot)

### ENG:01 — SpriteBatch + Texture Atlas
**Fișiere noi:** `src/rendering/SpriteBatch.cpp/.h`
**Fișiere modificate:** `src/rendering/Renderer.cpp/.h`, `src/world/TileMap.cpp/.h`

- `SpriteBatch` acumulează quad-uri (sf::Vertex[4]) într-un `sf::VertexArray` (sf::Quads)
- Un singur `window.draw(vertexArray, &texture)` per batch = 1 draw call per texture atlas
- API: `begin()` / `draw(texRect, worldPos, color)` / `end()` → flush
- TileMap: înlocuiește `vector<sf::Sprite>` cu apeluri `SpriteBatch::draw()`
- Renderer: tiles → 1 draw call, entities → 1 draw call (dacă pe același atlas)
- Zero heap alloc în game loop: VertexArray pre-alocat la `MAX_QUADS = 8192`
- **Target:** ≤ 3 draw calls pentru tiles (1 atlas tiles + 1 entities + 1 UI)

### ENG:02 — ShaderManager
**Fișiere noi:** `src/rendering/ShaderManager.cpp/.h`, `assets/shaders/scanlines.frag`, `assets/shaders/vignette.frag`

- Compilare **doar la startup** — `loadAll()` apelat o singură dată în `Game::init()`
- `getShader(ShaderType)` returnează referință const — zero alloc
- Shadere pornire Sprint 2: `scanlines.frag` (overlay permanent), `vignette.frag` (intensitate bazată pe HP)
- Uniform-uri setate **o dată per frame per shader** — nu per entitate
- `ShaderType` enum în `ShaderManager.h`

---

## WAVE 2 — GP + REN (paralel după ENG)

### GP:01 — WeaponSystem + PHANTOM-9
**Fișiere noi:** `src/player/WeaponSystem.cpp/.h`, `src/player/WeaponConfig.h`
**Fișiere modificate:** `src/ecs/Components.h`, `src/player/Player.cpp`

- `BulletComponent` POD: `{ float travelDist; float maxDist; int penetration; bool active; }`
- `PoolAllocator<BulletComponent, 1024> bulletPool` — în WeaponSystem, zero heap
- Mouse click stâng → spawn bullet în direcția crosshair (mouse world pos via Camera::screenToWorld)
- PHANTOM-9: viteză 800px/s, damage 25, maxDist 600px, penetrare 0
- Bullet update în `WeaponSystem::update()`: avansează `travelDist`, dezactivează la maxDist sau coliziune
- Coliziune bullet↔tile: raycast Box2D în PhysicsSystem, emit `BulletHitEvent`
- **WeaponConfig.h**: toate valorile constexpr, niciun magic number
- Crosshair: `sf::CircleShape` 4px, culoare `#00FFEE`, draw în REN layer

### GP:02 — PlayerStateMachine extins: Dash + Slide
**Fișiere modificate:** `src/player/PlayerStateMachine.cpp/.h`, `src/player/PlayerConfig.h`

- Stări noi: `DASH`, `SLIDE` (în plus față de IDLE/RUN/JUMP/FALL/CROUCH)
- **DASH:** LShift → impulse directional `DASH_SPEED=600`, durată `DASH_DURATION=0.18s`, cooldown `DASH_COOLDOWN=0.8s`, iframe `DASH_IFRAME=0.12s`
- **SLIDE:** crouch în timp ce alergi → viteză × `SLIDE_SPEED_MULT=1.6`, durată `SLIDE_DURATION=0.35s`, hitbox redus (câmpia coliziunii scade cu 40%)
- GDD: "Dash directional cu iframe scurt (evitare gloanțe)"

### REN:01 — Parallax Background System
**Fișiere noi:** `src/rendering/ParallaxSystem.cpp/.h`
**Fișiere modificate:** `src/rendering/Renderer.cpp`

- 3 layere inițiale (fără assets reale — placeholder colored rects):
  - Layer 0 (far): speed factor 0.1 — skyline
  - Layer 1: speed factor 0.3 — clădiri mediu
  - Layer 2: speed factor 0.6 — prim plan
- Camera x offset → deplasare proporțională pe fiecare layer
- Draw: câte 1 draw call per layer = max 3 draw calls suplimentare
- Pregătire pentru assets reale în Sprint 3

---

## WAVE 2 — AI (paralel cu GP + REN)

### AI:01 — NavMesh basic + A*
**Fișiere noi:** `src/enemies/NavMesh.cpp/.h`, `src/enemies/Pathfinding.cpp/.h`
**Fișiere modificate:** `src/world/LevelLoader.cpp`

- NavMesh generat din collision layer JSON la load — noduri centrate pe tiles walkable
- Conectivitate: 4 direcții + diagonal; sare conexiunile prin tiles solide
- A* standard, heuristic Manhattan; max 4 recalculări/frame (budget CLAUDE.md)
- Path cache per enemy — recalculat doar la schimbare de stare sau blocare

### AI:02 — SCOUT Enemy + AIStateMachine
**Fișiere noi:** `src/enemies/Enemy.cpp/.h`, `src/enemies/EnemyConfig.h`, `src/enemies/AIStateMachine.cpp/.h`, `src/enemies/EnemyManager.cpp/.h`
**Fișiere modificate:** `src/ecs/Components.h`, `src/world/LevelLoader.cpp`

- Componente noi POD: `EnemyTag { EnemyType type; }`, `AIState { AIStateEnum current; float timer; }`
- Stări: PATROL → ALERT → COMBAT → SEARCH (GDD spec)
- **PATROL:** mișcare pe waypoints din JSON; viteză `SCOUT_PATROL_SPEED=80px/s`
- **ALERT:** s-a auzit ceva (bullet impact/pași) → investigare punct sunet, max 5s
- **COMBAT:** LOS pe ZERO (raycast, max 8/frame budget) → trage la intervale
- **SEARCH:** pierdut ZERO → caută 15s, revine la PATROL
- LOS: sf::Vector2f raycast prin PhysicsSystem (nu accesare directă b2World)
- `EnemyConfig.h`: HP=50, detect_range=400px, attack_range=250px, attack_interval=1.2s
- `EnemyManager`: spawn din LevelLoader, max 32 activi (pool)
- Comunicare cross-system: `EnemyDiedEvent`, `EnemyAlertedEvent` pe EventBus

---

## WAVE 3 — UI + LVL

### UI:01 — HUD minimal
**Fișiere noi:** `src/ui/HUD.cpp/.h`
**Fișiere modificate:** `src/rendering/Renderer.cpp`

- HP bar: rect stânga-jos, fill cyan→magenta gradient la HP < 30% (singura excepție gradient — GDD)
- Ammo counter: `ShareTechMono 16px`, `#E0EEF8`, colț dreapta-jos
- Alert level indicator: 0–3, culoare: verde/galben/portocaliu/roșu (reutilizând paleta — `#00FFEE`/`#FFE600`/`#FF006B`/`#FF0038`)
- 1 draw call suplimentar (UI batch separat)

### LVL:01 — Level 1-1 extins cu enemies + waypoints
**Fișiere modificate:** `assets/levels/1-1.json`

- Adaugă 3 SCOUT enemies cu waypoints de patrol
- Adaugă cover objects (static bodies)
- Adaugă trigger zone la finalul nivelului

---

## WAVE 4 — BLD + QA

### BLD:01 — CMake update
**Fișiere modificate:** `CMakeLists.txt`

- Adaugă toate fișierele noi la `target_sources`
- Verifică că shaderele sunt copiate în build dir (configure_file sau file COPY)

### QA:01 — Checklist Sprint 2

| ID | Test |
|---|---|
| QA:S2:01 | Build zero warnings/errors pe `-Wall -Wextra` |
| QA:S2:02 | Draw calls per frame ≤ 10 (counter afișat în titlu în debug build) |
| QA:S2:03 | PHANTOM-9: bullet spawn la click, lovetură confirmată vizual |
| QA:S2:04 | SCOUT patrol pe waypoints, tranzit PATROL→COMBAT la LOS |
| QA:S2:05 | Scanlines visible + vignette intensificat la HP < 30% |
| QA:S2:06 | Dash LShift funcțional, cooldown respectat |
| QA:S2:07 | HUD: HP + ammo vizibil, actualizat corect |
| QA:S2:08 | Zero heap allocs în game loop (LD_PRELOAD shim din Sprint 1) |
| QA:S2:09 | ≤ 4 A* recalculări/frame cu 3 SCOUT activi |
| QA:S2:10 | FPS ≥ 120 @ 1080p cu 3 SCOUT + bullets active |

---

## ORDINE EXECUȚIE

```
ENG:01 (SpriteBatch)   ←── prerequisit pentru toate REN tasks
ENG:02 (ShaderManager) ←── prerequisit pentru REN:01 shadere
       ↓
GP:01  GP:02  REN:01  AI:01  AI:02   ← paralel
       ↓
UI:01  LVL:01                        ← după GP + AI
       ↓
BLD:01 → QA:01
```

---

## FIȘIERE CREATE / MODIFICATE

| Fișier | Acțiune |
|---|---|
| `src/rendering/SpriteBatch.cpp/.h` | **NOU** |
| `src/rendering/ShaderManager.cpp/.h` | **NOU** |
| `src/rendering/ParallaxSystem.cpp/.h` | **NOU** |
| `src/rendering/Renderer.cpp/.h` | MODIFICAT — integrare SpriteBatch + Shadere |
| `src/player/WeaponSystem.cpp/.h` | **NOU** |
| `src/player/WeaponConfig.h` | **NOU** |
| `src/player/PlayerStateMachine.cpp/.h` | MODIFICAT — DASH + SLIDE |
| `src/player/PlayerConfig.h` | MODIFICAT — constante noi |
| `src/enemies/NavMesh.cpp/.h` | **NOU** |
| `src/enemies/Pathfinding.cpp/.h` | **NOU** |
| `src/enemies/Enemy.cpp/.h` | **NOU** |
| `src/enemies/EnemyConfig.h` | **NOU** |
| `src/enemies/AIStateMachine.cpp/.h` | **NOU** |
| `src/enemies/EnemyManager.cpp/.h` | **NOU** |
| `src/ui/HUD.cpp/.h` | **NOU** |
| `src/ecs/Components.h` | MODIFICAT — BulletComponent, EnemyTag, AIState |
| `src/world/TileMap.cpp/.h` | MODIFICAT — SpriteBatch |
| `src/world/LevelLoader.cpp` | MODIFICAT — spawn enemies, waypoints, cover |
| `assets/levels/1-1.json` | MODIFICAT — enemies + waypoints |
| `assets/shaders/scanlines.frag` | **NOU** |
| `assets/shaders/vignette.frag` | **NOU** |
| `CMakeLists.txt` | MODIFICAT — surse noi + shader copy |

---

**Approval Gate: EXECUTAT** — Build debug + release curat, zero warnings.
