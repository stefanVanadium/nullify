# PLAN.md — NULLIFY Sprint 3
> Status: **ÎN EXECUȚIE**
> Bază: Sprint 2 complet (v0.2) · GDD v0.1 · Luna 3 — Visual Polish

---

## OBIECTIVE SPRINT 3

Sprint 3 livrează feedback vizual real de combat + fundația hack-ului:
- **ParticleSystem** — scântei la impact glonț, sânge la moarte inamic
- **neon_glow.frag** — bloom pe pixelii luminoși via RenderTexture
- **chromatic_aberration** — integrat în neon_glow shader (uniform `caIntensity`)
- **glitch.frag** — overlay hack mode cu distorsionare și tint violet
- **HackSystem** — press E lângă terminal → 3s progres → HackSuccessEvent
- **Enemy projectile bullets** — SCOUT trage gloanțe reale (nu instant damage)
- **HUD hack overlay** — bara de progres + text `[HACKING...]`

---

## WAVE 1 — ENG + REN (paralel)

### ENG:03 — ParticleSystem
**Fișiere noi:** `src/rendering/ParticleSystem.cpp/.h`
- Pool 4096 `Particle` structs (fixed array, zero heap)
- `spawnBulletImpact(x,y)` → 6 scântei cyan (#00FFEE), viteză random outward, 0.4s viață
- `spawnBlood(x,y)` → 5 particule #00FFEE/#FF0038 alternant, 0.5s viață, bias downward
- `update(float dt)` → avansează poziție, decay life, fade alpha
- `batchDraw(SpriteBatch&)` → 1 quad per particulă activă
- Subscrie la `BulletHitEvent` + `EnemyDiedEvent` în constructor; unsubscribe în destructor

### REN:02 — Bloom + CA + Glitch via RenderTexture
**Fișiere noi:** `assets/shaders/neon_glow.frag`, `assets/shaders/glitch.frag`
**Fișiere modificate:** `src/rendering/ShaderManager.h/.cpp`, `src/rendering/Renderer.h/.cpp`

**Pipeline nou:**
```
Scene → m_sceneRT (RenderTexture)
m_sceneRT → neon_glow.frag (bloom + CA) → window
Scanlines + Vignette + Glitch → window overlays
```

- `neon_glow.frag`: 12-tap bloom pe pixeli cu brightness > 0.55 + chromatic aberration via uniform `caIntensity`
- `glitch.frag`: row displacement noise + violet tint, uniform `intensity` + `time`
- `Renderer` adaugă `sf::RenderTexture m_sceneRT`, `sf::Sprite m_sceneSprite`, `SpriteBatch m_particleBatch`
- `struct RenderEffects { float caIntensity; float hackIntensity; float gameTime; }`
- `ShaderType`: NeonGlow=2, Glitch=3, COUNT=4
- Draw call budget: parallax(3) + tiles(1) + entities+bullets+particles(1) + crosshair(1) = 6 în RT, then RT→window(1) + scanlines(1) + vignette(1) + glitch(max 1) = ≤ 10 total

---

## WAVE 2 — GP + AI (paralel cu WAVE 1)

### GP:03 — HackSystem
**Fișiere noi:** `src/player/HackSystem.cpp/.h`
- Press E în raza `HACK_INTERACT_RANGE=80px` de un `HackableTag` entity nehackit
- `m_hacking=true`, `m_hackTimer=HACK_TIER1_DURATION=3.0s`
- Countdown → emit `HackSuccessEvent`, marchează `hackable.hacked=true`
- Expune `isHacking()`, `hackProgress()` (0→1), `hackIntensity()` (1.0→0.2 în 0.3s)

### GP:04 — Enemy Projectile Bullets
**Fișiere modificate:** `src/enemies/AIStateMachine.cpp`, `src/enemies/EnemyManager.h/.cpp`

- `EnemyFireEvent { uint32_t enemyId; float fromX,fromY,dirX,dirY; }` în EventBus.h
- AIStateMachine: în loc de instant damage → emit `EnemyFireEvent` cu direcție spre player
- EnemyManager: pool 256 `EnemyBullet` (fixed array), subscrie `EnemyFireEvent`
- Enemy bullets: roșii (#FF0038), viteza `SCOUT_BULLET_SPEED=300px/s`, maxDist 600px
- Raycast pentru coliziune; hit player → `PlayerDamagedEvent`; hit wall → deactivate
- `batchDrawBullets(SpriteBatch&)` pentru Renderer

---

## WAVE 3 — UI + LVL

### UI:02 — HUD Hack Overlay
**Fișiere modificate:** `src/ui/HUD.h/.cpp`
- `renderHackOverlay(window, progress)`: text `[HACKING...]` centrat, bară progres violet (#AA00FF)
- Apelat din Game.cpp când `m_hackSystem.isHacking()`

### LVL:02 — Terminal hackabil în 1-1.json
**Fișiere modificate:** `src/world/LevelLoader.h/.cpp`, `assets/levels/1-1.json`
- Parsează `"hackables"` array din JSON → entități cu `HackableTag` + `Transform` + `Renderable`
- Terminale = pătrate violet (#AA00FF), 16×24px, layer 8
- Adaugă un terminal în 1-1.json la tile (8, 19) (lângă spawn player)

---

## WAVE 4 — BLD + QA

### BLD:03
**Fișiere modificate:** `CMakeLists.txt`
- Adaugă `src/rendering/ParticleSystem.cpp`, `src/player/HackSystem.cpp`

### QA:03 — Checklist Sprint 3

| ID | Test |
|---|---|
| QA:S3:01 | Build zero warnings/errors `-Wall -Wextra -Wpedantic` |
| QA:S3:02 | Bullet impact: scântei cyan vizibile pe orice suprafață |
| QA:S3:03 | Enemy death: blood particles vizibile |
| QA:S3:04 | Bloom: entitățile bright (ZERO, enemies) au aură de glow |
| QA:S3:05 | CA: RGB split vizibil 0.8s la damage de la inamic |
| QA:S3:06 | Enemy fires real bullet (pătrat roșu vizibil traversând ecranul) |
| QA:S3:07 | Press E lângă terminal: glitch activ, overlay `[HACKING...]`, progres 3s, success |
| QA:S3:08 | Draw calls ≤ 10 (confirmat în titlu ferestrei) |
| QA:S3:09 | Zero heap allocs în game loop |
| QA:S3:10 | FPS ≥ 120 @ 1080p cu 3 SCOUT + particles active |

---

## FIȘIERE CREATE / MODIFICATE

| Fișier | Acțiune |
|---|---|
| `src/rendering/ParticleSystem.cpp/.h` | **NOU** |
| `src/player/HackSystem.cpp/.h` | **NOU** |
| `assets/shaders/neon_glow.frag` | **NOU** |
| `assets/shaders/glitch.frag` | **NOU** |
| `src/ecs/Components.h` | MODIFICAT — HackableTag |
| `src/ecs/World.h` | MODIFICAT — HackableTag component |
| `src/core/EventBus.h` | MODIFICAT — EnemyFireEvent, HackActivatedEvent |
| `src/rendering/ShaderManager.h/.cpp` | MODIFICAT — NeonGlow, Glitch |
| `src/rendering/Renderer.h/.cpp` | MODIFICAT — RenderTexture, particles, effects |
| `src/enemies/AIStateMachine.cpp` | MODIFICAT — emit EnemyFireEvent |
| `src/enemies/EnemyManager.h/.cpp` | MODIFICAT — enemy bullet pool |
| `src/enemies/EnemyConfig.h` | MODIFICAT — bullet constexpr values |
| `src/ui/HUD.h/.cpp` | MODIFICAT — renderHackOverlay |
| `src/world/LevelLoader.h/.cpp` | MODIFICAT — parse hackables |
| `assets/levels/1-1.json` | MODIFICAT — hackable terminal |
| `src/core/Game.cpp` | MODIFICAT — HackSystem, ParticleSystem, CA timer, effects |
| `CMakeLists.txt` | MODIFICAT — 2 surse noi |

---

**Approval Gate: EXECUTAT**
