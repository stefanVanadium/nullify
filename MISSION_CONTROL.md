# MISSION CONTROL — NULLIFY

---

## CHECKPOINT 001 — Sprint 1 V0.1 Foundation
**Data:** 2026-06-28
**Status:** ✅ Build complet, joc lansabil

### Livrat
- CMakeLists.txt cu FetchContent (SFML 2.6.2, Box2D 2.4.1, nlohmann/json 3.11.3)
- ECS complet: World (SoA, 1024 entități max), Components.h (Transform, Velocity, Health, Renderable, Collidable, PlayerTag, TileTag)
- EventBus typed (emit zero-alloc, subscribe/unsubscribe, clearAll)
- InputMap (WASD+Space+LShift, isHeld/isPressed/isReleased)
- GameState machine (push/pop/replace stack, pending ops)
- Game loop: fixed 60Hz physics + render interpolation alpha
- PhysicsSystem: b2World unic, sync body→Transform, createStaticBody/createDynamicBody, isBodyGrounded (raycast)
- Renderer: sort pe layer, interpolare vizuală prevPos→currPos
- PlayerConfig.h (toate constexpr), Player, PlayerStateMachine (IDLE/RUN/JUMP/FALL/CROUCH + coyote + jump buffer)
- TileMap + LevelLoader (JSON→Box2D bodies + ECS entities)
- Camera (lerp CAMERA_LERP=5.0, clamp nivel, aplică sf::View)
- Level `assets/levels/1-1.json`: 60×20 tiles, podea completă, 4 platforme suspendate

### QA confirmată — TOATE 10
- [x] QA:01 Build curat zero warnings/errors
- [x] QA:02 ZERO apare la spawn
- [x] QA:03 Mișcare A/D funcționează (confirmat vizual + input automation via libXtst)
- [x] QA:04 Săritură Space funcționează, coyote time activ
- [x] QA:05 ZERO cade pe podea, nu cade prin ea
- [x] QA:06 ZERO aterizează pe platforme suspendate (~160px înălțime)
- [x] QA:07 Camera lerp urmărește ZERO, clampată la limitele nivelului
- [x] QA:08 FPS counter în titlu ("NULLIFY v0.1 | 142 FPS" confirmat screenshot)
- [x] QA:09 Zero heap allocs din codul jocului (verificat LD_PRELOAD shim: toate allocs din XCB/Mesa/libX11)
- [x] QA:10 Procesul se închide clean la ESC

### Note tehnice
- FetchContent descarcă deps la prima configurare CMake (~60s)
- XWayland screenshot via spectacle (ffmpeg x11grab nu capturează GL composited)
- Camera x clampată la 640 (level 1920px > window 1280px) — funcționează corect
- Camera y clampată la 360 (level 640px < window 720px — guard implementat)
- Input automation: libXtst XTestFakeKeyEvent via LD_PRELOAD Python shim; Wayland necesită aprobare "Remote Control" la primul run
- setTitle folosește snprintf în loc de std::to_string — elimină 2 temp strings per update FPS
- git: d79ca2b

---

## CHECKPOINT 002 — Sprint 2 V0.2 Gameplay Loop
**Data:** 2026-06-28
**Status:** ✅ Build curat (debug + release), zero warnings/errors `-Wall -Wextra -Wpedantic`

### Livrat

**ENG — Rendering infrastructure:**
- `SpriteBatch` — VertexArray pre-alocat (8192 quads × 4 vertices), zero heap în game loop
- `ShaderManager` — compilare shader doar la startup, `loadAll()` + `get(ShaderType)`
- `scanlines.frag` + `vignette.frag` — overlay screen-space, uniform `intensity` per frame
- `ParallaxSystem` — 3 layere placeholder colored rects, speed factors 0.1/0.3/0.6
- `Renderer` refăcut — SpriteBatch entități + draw calls: tiles(1) + entități(1) + crosshair(1) + parallax(3) + shadere(2) = **≤ 8 draw calls/frame**
- `TileMap` refăcut — nu mai creează entități ECS; construiește sf::VertexArray static; physics bodies via `createStaticBody()` trăiesc în b2World

**ENG — Foundation:**
- `Components.h`: Renderable simplificat (size+color), adăugate EnemyTag/AIState/WaypointPath/Weapon
- `World.h`: 11 tipuri componente (COUNT=11), template specializations complete
- `PhysicsSystem`: adăugat `rayCastClear(from, to)` pentru LOS
- `EventBus`: adăugate BulletHitEvent, EnemyAlertedEvent, PlayerDamagedEvent
- `InputMap`: mouse position tracking (`mouseScreenPos()`)
- `Camera`: `screenToWorld()` + `center()` getter
- `GameState.h`: adăugat `top()` la GameStateManager

**GP — Player systems:**
- `WeaponSystem` — pool 1024 bullet states, zero heap; fire pe mouse click; raycast collision
- `WeaponConfig.h` — toate valorile PHANTOM-9 constexpr
- `PlayerStateMachine` — DASH + SLIDE cu timere, cooldown 0.8s
- `PlayerConfig.h` — constante DASH/SLIDE adăugate
- `Player` — EventBus subscription pentru dash/slide physics; component Weapon la spawn

**AI — Enemy systems:**
- `NavMesh` — grid walkable din collision layer, conexiuni orizontale
- `Pathfinding` — A* zero-heap (fixed arrays), budget extern 4/frame
- `AIStateMachine` — PATROL→ALERT→COMBAT→SEARCH cu LOS raycast + A* path cache
- `EnemyConfig.h` — toate valorile SCOUT constexpr
- `EnemyManager` — spawn/update max 32 enemies, navmesh, alertLevel 0-3

**UI + LVL:**
- `HUD` — HP bar (cyan→magenta la < 30%), ammo counter, alert level; font opțional
- `LevelLoader` — returnat `LevelData`; adăugat enemy spawns + cover objects
- `1-1.json` — 3 SCOUT enemies cu waypoints + 3 cover objects + trigger level_end

**BLD:** `CMakeLists.txt` — 8 surse noi

### QA confirmată
- [x] QA:S2:01 Build zero warnings/errors — debug + release
- [ ] QA:S2:02..10 — runtime QA pending (necesită display)

### Note tehnice
- `sf::Color` nu e literal type în SFML 2.6 — `constexpr sf::Color` → `const sf::Color`
- Enemy bullets = instant-damage în Sprint 2; projectile enemy în Sprint 3
- Slide nu modifică hitbox Box2D în Sprint 2

---

## CHECKPOINT 003 — Sprint 3 V0.3 Visual Combat + Hack Foundation
**Data:** 2026-06-28
**Status:** ✅ Build curat (debug + release), zero warnings/errors `-Wall -Wextra -Wpedantic`

### Livrat

**ENG — ParticleSystem:**
- Pool 4096 particule (fixed array, zero heap în game loop)
- `spawnBulletImpact(x,y)` — 6 scântei cyan (#00FFEE), viteză outward random, 0.4s viață
- `spawnBlood(x,y)` — 5 particule #00FFEE/#FF0038 alternant, bias downward, 0.5s viață
- Subscrie `BulletHitEvent` + `EnemyDiedEvent` în constructor, unsubscribe în destructor
- `batchDraw(SpriteBatch&)` — merge în entity batch, 0 draw calls extra

**REN — Post-process pipeline cu RenderTexture:**
- `neon_glow.frag` — 12-tap bloom pe pixeli luminoși + chromatic aberration uniform `caIntensity`
- `glitch.frag` — row displacement noise + RGB split + violet tint (#AA00FF)
- `ShaderType::NeonGlow=2, Glitch=3, COUNT=4`
- `Renderer` refăcut: scenă → `sf::RenderTexture` → neon_glow → window; glitch condiționat
- `RenderEffects { caIntensity, hackIntensity, gameTime }` — bundle parametri efecte
- **Draw call budget:** 9-10 draw calls ✅

**GP — HackSystem:**
- Press E în 80px de `HackableTag` → 3s countdown, glitch max pe entry → 0.2 pe durata hack
- Success → `HackSuccessEvent`, terminal vizual → dark violet
- `isHacking()`, `hackProgress()`, `hackIntensity()` expuse

**GP — Enemy Projectile Bullets:**
- `EnemyFireEvent` — AIStateMachine emite în loc de instant damage
- Pool 256 `EnemyBullet` (zero heap), bullets roșii (#FF0038), 300px/s, raycast coliziune
- `batchDrawBullets(SpriteBatch&)` — merge în entity batch

**UI + LVL:**
- `HUD::renderHackOverlay()` — panel centrat, `[HACKING...]`, bară violet progres
- `LevelLoader` parsează `"hackables"` → `HackableTag` entities (violet 16×24px, layer 8)
- `1-1.json` — terminal hackabil la tile (8,20) lângă spawn player

**ECS:**
- `HackableTag` component — `ComponentType::HackableTag=11, COUNT=12`
- `EnemyFireEvent`, `HackActivatedEvent`, `HackFailedEvent` în EventBus.h

### QA confirmată
- [x] QA:S3:01 Build zero warnings/errors — debug + release (20/20 obiecte)
- [ ] QA:S3:02..10 — runtime QA pending (necesită display)

### Note tehnice
- `sf::RenderTexture` Y-flip gestionat automat de SFML la draw via `sf::Sprite`
- Vignette quad UV-uri ajustate (Y invertit) pentru orientarea corectă a RT-ului
- ParticleSystem folosește LCG zero-heap pentru random în hot path
- Enemy bullets nu au physics body — coliziune via raycast + AABB player check (d2 < 36²)
- Enemy instant-damage din Sprint 2 înlocuit cu projectile real

---
