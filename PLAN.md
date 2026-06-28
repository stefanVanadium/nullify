# PLAN.md — NULLIFY Sprint 4
> Status: **ÎN EXECUȚIE**
> Bază: Sprint 3 complet (v0.3) · GDD v0.1 · Luna 4 — Gameplay Depth

---

## OBIECTIVE SPRINT 4

Luna 4 transformă demo-ul tehnic într-un joc real cu depth mecanic complet:

1. **Hacking minigame complet** — Tier 1/2/3 interactive (GDD §03)
2. **Toate tipurile de inamici** — 7 tipuri noi pe lângă SCOUT (GDD §04)
3. **Stealth system** — cone of vision, hearing, corpuri, takedown silențios
4. **Toate armele** — 5 arme noi pe lângă PHANTOM-9 (GDD §03)
5. **Level 1-1 → 1-3 complet** — 1-2 și 1-3 JSON cu design conform GDD

**Starea inițială:**
- SCOUT enemy ✅, HackSystem fundație ✅, PHANTOM-9 ✅, 1-1.json ✅
- Ragdoll la moarte ❌ (pendent din Luna 2 — blocat de joints Box2D)
- Hacking minigame ❌, stealth ❌, inamici multipli ❌, arme 5/6 ❌, 1-2/1-3 ❌

---

## WAVE 1 — ENG (fundație pentru sisteme noi) ✅

### ENG:04 — Ragdoll System (datorie tehnică din Sprint 2)
**Fișiere noi:** `src/rendering/RagdollSystem.cpp/.h`
**Fișiere modificate:** `src/ecs/Components.h`, `src/enemies/EnemyManager.cpp`

- `struct Ragdoll { b2Body* bodies[6]; b2Joint* joints[5]; bool active; float lifetime; }` în Components.h
- La `EnemyDiedEvent`: creează 6 corpuri Box2D (torso, cap, 2x braț, 2x picior) cu joints revolute limitate
- Aplică impulse de la direcția glonțului (din event)
- Lifetime 4.0s → fade out alpha → deactivate bodies → remove entity
- `RagdollSystem::update(float dt)` iterează ragdoll-uri active, avansează timer, fade
- `RagdollSystem::render(SpriteBatch&)` desenează 6 dreptunghiuri per ragdoll (proxy vizual, fără sprite final)
- Subscrie `EnemyDiedEvent` în constructor

### ENG:05 — Stealth Perception Components (arhitectură)
**Fișiere modificate:** `src/ecs/Components.h`, `src/core/EventBus.h`

Componente noi în Components.h:
```cpp
struct ConeOfVision  { float range; float halfAngle; bool playerVisible; };
struct HearingRadius { float range; bool alertedBySound; };
struct StealthBody   { bool isCorpse; bool corpseFound; float corpseTimer; };
struct SilentTakedown{ bool vulnerable; };
```

Evenimente noi în EventBus.h:
```cpp
struct CorpseFoundEvent   { uint32_t enemyId; float x, y; };
struct SoundEmittedEvent  { float x, y; float radius; bool isShot; };
struct TakedownEvent      { uint32_t targetId; };
struct EnemyHackedEvent   { uint32_t targetId; float duration; };
struct WeaponSwitchedEvent{ WeaponType newWeapon; };
struct HackMinigameStartEvent { int tier; uint32_t hackableId; };
struct HackBlockedEvent   { uint32_t blockerId; };
struct EMPDetonatedEvent  { float x, y, radius; };
struct CoverDestroyedEvent{ uint32_t coverId; };
```

---

## WAVE 2 — GP + AI + REN (paralel) ✅ (REN:03 pending)

### GP:05 — Toate Armele (5 noi + rework PHANTOM-9)
**Fișiere modificate:** `src/player/WeaponConfig.h`, `src/player/WeaponSystem.h/.cpp`

`WeaponConfig.h` — adaugă constexpr pentru fiecare armă:
```
PHANTOM-9    : BURST_MODE toggle (Q), silenced (SoundEmitted radius=0)
STATIC SMG   : 600rpm, spread 8°, pool 1024, SoundEmitted radius 400px
RAILGUN MK2  : 1 glonț, penetrare infinită pereți dacă hackChargeActive, cooldown 2s
VOID SHOTGUN : 8 pellets, spread 30°, knockback 400px/s pe inamici, range 250px
EMP GRENADE  : proiectil arcuit (parabolă), raza 180px, disable drone/cyborg 5s
NEURAL SPIKE : proiectil slow, la hit → EnemyHackedEvent (ZERO controlează 3s)
```

`WeaponSystem`:
- `enum class WeaponType { PHANTOM9, STATIC_SMG, RAILGUN, VOID_SHOTGUN, EMP_GRENADE, NEURAL_SPIKE }`
- `struct WeaponSlot { WeaponType type; int ammo; float cooldownLeft; bool burstMode; }`
- Player are slot activ, ciclat cu scroll wheel / taste 1-6
- EMP: spawn proiectil cu viteză+gravitate → la impact emit `EMPDetonatedEvent { x, y, radius }`
- Neural Spike: spawn proiectil lent → hit → emit `EnemyHackedEvent`, ZERO intră în control mode 3s
- Railgun: raycast extins (ignoră `isSolid=false` tiles dacă hackChargeActive) → damage toate entitățile pe traiectorie
- Emit `SoundEmittedEvent` la fiecare împușcătură (raze diferite per armă, 0 pentru PHANTOM-9 silenced)

### AI:01 — Toate Tipurile de Inamici (7 noi)
**Fișiere noi:** `src/enemies/EnemyTypes.h`
**Fișiere modificate:** `src/enemies/EnemyConfig.h`, `src/enemies/AIStateMachine.h/.cpp`, `src/enemies/EnemyManager.h/.cpp`

`EnemyTypes.h`:
```cpp
enum class EnemyType {
    SCOUT, ENFORCER, SHIELD, SNIPER, HACKER, HEAVY, DRONE, CYBORG_ELITE
};
```

`EnemyConfig.h` — constexpr per tip:
```
ENFORCER  : HP 80, armor 0.5x dmg frontal, cover-seeker, flanking
SHIELD    : HP 100, scut față (0 dmg față, full dmg flanc/spate), slow
SNIPER    : HP 40, laser sight, 1-shot 60dmg, re-aim 2.5s, pozitii înalte
HACKER    : HP 50, blochează Neural Override în raza 200px, nu atacă
HEAVY     : HP 200, minigun 800rpm spread 15°, viteză 60%, distruge cover
DRONE     : HP 20, aerian (fără NavMesh), EMP = instant kill, patrol 360°
CYBORG_ELITE: HP 350, 3 faze — stub în Sprint 4, complet în Sprint 5
```

`AIStateMachine` — extensii per tip:
- **ENFORCER**: COMBAT → caută nearest cover point, trage din cover, flanchează dacă 2+ COMBAT
- **SHIELD**: COMBAT → avansează spre player, scut activ față; rotire la atacuri laterale
- **SNIPER**: laser sight activ dacă LOS; trage după 1.5s aim-time; stă pe waypoints înalte
- **HACKER**: rămâne în spate, emit `HackBlockedEvent` dacă ZERO HACK state în raza 200px
- **HEAVY**: spray continuu în COMBAT, emit `BulletHitCoverEvent` când lovește cover
- **DRONE**: waypoints aeriene directe (Velocity-based), detectare 360°, nu are `NavMeshAgent`
- **CYBORG_ELITE**: stub COMBAT — trage ca ENFORCER, HP 350; faze în Sprint 5

`EnemyManager` — `spawnEnemy(EnemyType, x, y)` factory; atribuie componente corespunzătoare

### GP:06 — Stealth System
**Fișiere noi:** `src/player/StealthSystem.cpp/.h`
**Fișiere modificate:** `src/enemies/AIStateMachine.cpp`, `src/rendering/Renderer.cpp`

- **Cone of vision**: fiecare frame → max 8 LOS raycasts distribuite round-robin (`enemyIdx % 8`)
- Check: `dot(toPlayer, facingDir) > cos(halfAngle)` AND `dist < range` AND raycast passes → `PlayerSpottedEvent`
- **Hearing**: la `SoundEmittedEvent` → toți inamicii în raza → `alertedBySound=true` → ALERT, mers la sursă
- **Corpuri**: la `EnemyDiedEvent` → `StealthBody { isCorpse=true }`. Inamic cu LOS la corp → `CorpseFoundEvent` → AlertSystem +1
- **Silent takedown [F]**: `dot(playerFacing, enemyFacing) > 0.7` AND `dist < 48px` → `TakedownEvent` → instant kill, fără sunet
- `renderCones(SpriteBatch&)`: triangles semi-transparente cyan (#00FFEE40), roșii în ALERT/COMBAT

### REN:03 — Laser Sight + Visual Stealth Indicators
**Fișiere modificate:** `src/rendering/Renderer.cpp/.h`

- Laser sight SNIPER: linie roșie (#FF003880) de la enemy la primul hit point; pulsează alpha 0.4→0.9 la 2Hz
- Cone of vision overlay: triangles per inamic în același batch
- Weapon slot HUD: icoane 1-6 stânga-jos, activă luminoasă cyan, inactive dim (#6080A0)
- Hack blocked indicator: `[OVERRIDE BLOCKED]` roșu pulsând dacă HACKER în raza

---

## WAVE 3 — UI (Hacking Minigame) + LVL (1-2, 1-3)

### UI:03 — Hacking Minigame Complet (Tier 1/2/3)
**Fișiere noi:** `src/hacking/HackMinigame.cpp/.h`, `src/hacking/Tier1Sequence.cpp/.h`, `src/hacking/Tier2Circuit.cpp/.h`, `src/hacking/Tier3ICE.cpp/.h`
**Fișiere modificate:** `src/ui/HackingUI.cpp/.h`, `src/core/GameState.h/.cpp`, `src/core/Game.cpp`

**Arhitectură:**
```
HackSystem → detectează tier → emit HackMinigameStartEvent { tier }
GameState → HACK state (freeze physics + AI)
HackMinigame::start(tier) → activează tier-ul corect
HackMinigame::update(dt) → deleghează la tier activ
HackingUI::render(window) → UI per tier
```

**TIER 1 — Sequence Match:**
- 4 simboluri random din `{ ▲ ■ ● ◆ }` generate la start
- Afișate 1.5s, then hidden; player tastează secvența (WASD mapate la 4 simboluri)
- Timer 3.0s vizibil; greșeală = eșec imediat
- UI: 4 celule violet (#AA00FF), ShareTechMono 32px, timer bar sus

**TIER 2 — Circuit Routing:**
- Grid 5×5 noduri; start = (0,0), end = (4,4); 3–5 noduri blocate random
- Player navighează cu arrow keys, nu poate intersecta propriul path, nu poate înapoi
- Timer 8.0s; UI: grid cyan, path trasat în timp real, noduri blocate roșii

**TIER 3 — ICE Breaker:**
- 3 layere de criptare, fiecare cu HP (40/60/80)
- `Space` rapid-tap = brute force (rapid dar alertă +1 per layer eșuat)
- `S hold` = scan mode (lent, safe, garantat succes dacă timer OK)
- Timer 12.0s total; UI: 3 bare stacked, glitch overlay intens, countdown

**Integrare GameState:**
- `HACK` state: physicsSystem.update() și aiSystem.update() NU se apelează
- `HackSuccessEvent` → exit HACK state, efecte hackable (uși, camere, etc.)
- `HackFailEvent` → AlertSystem::addAlert(+1), spawn reinforcements

### LVL:03 — Level 1-2 și 1-3
**Fișiere noi:** `assets/levels/1-2.json`, `assets/levels/1-3.json`
**Fișiere modificate:** `src/world/LevelLoader.cpp/.h`, `src/world/TriggerSystem.cpp/.h`

**Level 1-2 — "THE UNDERCROFT":**
- 80×25 tiles (2560×800px), interior abandonat, 3 rute verticale
- Inamici: 4× ENFORCER, 2× SCOUT, 1× SNIPER (pe platformă înaltă)
- 2 terminale (Tier 1 și Tier 2); Tier 2 deschide ușa finală
- 6 cover objects (30 HP), trigger tranziție → 1-3 la tile (78, 12)
- Ambient: electric hum, picurare apă, fără ploaie

**Level 1-3 — "VEKTOR OUTPOST":**
- 100×30 tiles (3200×960px), outpost militarizat, neon corporatist
- Inamici: 3× ENFORCER, 2× SHIELD, 1× SNIPER, 1× HEAVY, 2× DRONE, 1× CYBORG_ELITE
- 3 terminale (Tier 1/2/3); Tier 3 dezactivează camere + drone de patrulare
- Weapon pickups: STATIC SMG și VOID SHOTGUN la mijlocul levelului
- 8 cover objects (unele destroyable de HEAVY)
- 2 camera regions cu scrolling limitat la boss fight
- Boss trigger la tile (95, 15) → spawn CYBORG_ELITE + AlertSystem full

**LevelLoader extensii:**
- `"coverObjects": [{ "x","y","w","h","hp","destroyable" }]` → entități cu `CoverTag + Health`
- `"weaponPickups": [{ "x","y","weaponType" }]` → entități cu `PickupTag + WeaponType`
- `"cameraRegions": [{ "xMin","xMax","yMin","yMax" }]` → Camera.setBounds()
- `"hackables"` include `"tier"` field (1/2/3) → HackMinigame pornește cu tier corect

---

## WAVE 4 — BLD + QA

### BLD:04
**Fișiere modificate:** `CMakeLists.txt`

Surse noi:
```
src/rendering/RagdollSystem.cpp
src/player/StealthSystem.cpp
src/hacking/HackMinigame.cpp
src/hacking/Tier1Sequence.cpp
src/hacking/Tier2Circuit.cpp
src/hacking/Tier3ICE.cpp
src/ui/HackingUI.cpp
```

### QA:04 — Checklist Sprint 4

| ID | Test | Acceptanță |
|---|---|---|
| QA:S4:01 | Build zero warnings `-Wall -Wextra -Wpedantic` | 0 erori, 0 warnings |
| QA:S4:02 | Ragdoll la moartea oricărui inamic | 6 bodies + joints, 4s lifetime |
| QA:S4:03 | Toate 6 armele funcționale | Fiecare trage cu mecanica corectă |
| QA:S4:04 | Scroll weapon slot 1-6 schimbă arma activă | HUD reflectă arma curentă |
| QA:S4:05 | ENFORCER se ascunde după cover în COMBAT | Caută nearest cover în 0.5s |
| QA:S4:06 | SNIPER laser sight vizibil, one-shot 60 dmg | Laser roșu + damage corect |
| QA:S4:07 | SHIELD — 0 dmg față, dmg normal flanc/spate | Testare cu PHANTOM-9 |
| QA:S4:08 | HEAVY — HP 200, mișcare lentă | Move speed 60% față de SCOUT |
| QA:S4:09 | DRONE — patrol aerian fără NavMesh, EMP kill instant | EMP grenade kills DRONE |
| QA:S4:10 | Cone of vision vizibil, detecție corectă | Intru în con → COMBAT |
| QA:S4:11 | Hearing — împușcătura alertează inamicii în raza | STATIC SMG alertează raza 400px |
| QA:S4:12 | PHANTOM-9 silenced nu alertează | Inamicii nu reacționează la distanță |
| QA:S4:13 | Silent takedown [F] din spate instant kill, fără alertă | Nu emit SoundEmittedEvent |
| QA:S4:14 | Corpse found → AlertSystem +1 | Text `ALERT LVL 2` în HUD |
| QA:S4:15 | Hacking Tier 1 — secvență 4 simboluri, 3s | Succes + eșec funcționează |
| QA:S4:16 | Hacking Tier 2 — grid routing 5×5 | Nu poți intersecta propriul path |
| QA:S4:17 | Hacking Tier 3 — 3 layere ICE, 12s | Brute force = alertă la eșec |
| QA:S4:18 | 1-2.json se încarcă, tranziție → 1-3 funcțională | Level traversal complet |
| QA:S4:19 | 1-3.json — CYBORG_ELITE spawn la boss trigger | Mini-boss apare |
| QA:S4:20 | Cover destroyable de HEAVY | Cover dispare la HP=0 |
| QA:S4:21 | Weapon pickup din nivel funcționează | Player primește arma |
| QA:S4:22 | Draw calls ≤ 10 cu toate sistemele active | Confirmat în titlul ferestrei |
| QA:S4:23 | FPS ≥ 120 @ 1080p cu scenă completă | Profiler confirmat |
| QA:S4:24 | Zero heap allocs în game loop | Counter = 0 |

---

## FIȘIERE CREATE / MODIFICATE

| Fișier | Acțiune |
|---|---|
| `src/rendering/RagdollSystem.cpp/.h` | **NOU** |
| `src/player/StealthSystem.cpp/.h` | **NOU** |
| `src/hacking/HackMinigame.cpp/.h` | **NOU** |
| `src/hacking/Tier1Sequence.cpp/.h` | **NOU** |
| `src/hacking/Tier2Circuit.cpp/.h` | **NOU** |
| `src/hacking/Tier3ICE.cpp/.h` | **NOU** |
| `src/enemies/EnemyTypes.h` | **NOU** |
| `assets/levels/1-2.json` | **NOU** |
| `assets/levels/1-3.json` | **NOU** |
| `src/ecs/Components.h` | MODIFICAT — Ragdoll, ConeOfVision, HearingRadius, StealthBody, SilentTakedown |
| `src/core/EventBus.h` | MODIFICAT — 8 evenimente noi |
| `src/core/GameState.h/.cpp` | MODIFICAT — HACK freeze logic |
| `src/core/Game.cpp` | MODIFICAT — RagdollSystem, StealthSystem, HackMinigame |
| `src/player/WeaponConfig.h` | MODIFICAT — toate 6 arme constexpr |
| `src/player/WeaponSystem.h/.cpp` | MODIFICAT — WeaponType enum, slot system, 5 mecanici noi |
| `src/enemies/EnemyConfig.h` | MODIFICAT — toate 8 tipuri constexpr |
| `src/enemies/EnemyTypes.h` | **NOU** — enum EnemyType |
| `src/enemies/AIStateMachine.h/.cpp` | MODIFICAT — 6 comportamente noi |
| `src/enemies/EnemyManager.h/.cpp` | MODIFICAT — factory spawnEnemy(type) |
| `src/rendering/Renderer.h/.cpp` | MODIFICAT — laser sight, cone overlay, weapon HUD |
| `src/ui/HackingUI.cpp/.h` | MODIFICAT — render Tier 1/2/3 |
| `src/world/LevelLoader.h/.cpp` | MODIFICAT — cover, pickups, camera regions, tier hackable |
| `src/world/TriggerSystem.h/.cpp` | MODIFICAT — boss trigger, level transition |
| `CMakeLists.txt` | MODIFICAT — 7 surse noi |

---

## NOTE TEHNICE CRITICE

**Stealth + LOS budget:** Max 8 LOS raycasts/frame (budget CLAUDE.md). Cu 8+ inamici, distribuim round-robin: `enemyIdx % 8` per frame. ConeOfVision update în `StealthSystem::update()`, nu în AIStateMachine.

**Drone fără NavMesh:** DRONE nu are `NavMeshAgent`. Mișcare directă prin `Velocity` pe waypoints aeriene. Physics body = sensor (fără coliziune cu tiles), colizionează doar cu `PlayerBody`.

**HACKER block:** `HackSystem` verifică fiecare frame dacă există HACKER activ în raza 200px. Dacă da, `m_blocked=true` → minigame nu pornește + overlay "BLOCKED".

**Weapon slots vs pool:** Bullet pool 1024 shared. Enemy bullets 256. EMP și Neural Spike = proiectile în pool-ul de bullets cu `BulletType` flag pentru comportament special.

**GameState HACK freeze:** Când `GameState == HACK`, physicsSystem și aiSystem NU se apelează. Doar `hackMinigame.update()` și `hackingUI.render()`. Camera rămâne fixă.

**Cover HP:** Cover objects au `Health` component existent. La `BulletHitEvent` pe cover entity → `CombatSystem` scade HP. La `Health.current == 0` → `CoverDestroyedEvent` → body Box2D removes, entity deactivate.

---

**Approval Gate: PENDING**
