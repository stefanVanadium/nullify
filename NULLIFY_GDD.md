# NULLIFY
## Game Design Document v0.1
> "The system is the enemy. You are the virus."

| | |
|---|---|
| **Engine** | Custom C++20 + SFML 2.6 + Box2D 2.4 |
| **Genre** | 2D Cyberpunk Side-Scroller Shooter |
| **Platform** | PC (Windows / Linux) |
| **Target FPS** | 144fps @ 1080p / 60fps @ 4K |
| **Status** | Pre-production |
| **Version** | GDD v0.1 — June 2026 |

---

## 01. OVERVIEW

### Concept
NULLIFY este un 2D side-scroller shooter cyberpunk cu mecanici de hacking, stealth opțional, și fizică ragdoll. Jucătorul controlează un hacker renegat într-un oraș corporatist supravegheat total, vânat de agenți de elită. Obiectivul final: penetrarea CORE-ului sistemului central și ștergerea lui.

### Pitch în 3 cuvinte
**HACK · SHOOT · VANISH**

### Inspirații

| Joc | Ce împrumutăm |
|---|---|
| Plasma Burst 2 | Gameplay bază — shooting fluid, ragdoll, level design |
| Katana Zero | Vibe noir, execuții cinematice, storytelling |
| Deus Ex | Hacking mechanic, libertate de abordare |
| Mirror's Edge | Mișcare fluidă, momentum |
| Cyberpunk 2077 | Estetică vizuală, lore corporatist |

---

## 02. STORY & LORE

### Setting
Orașul **NEXUS-7, 2087**. O megacorporație numită **VEKTOR** controlează toată infrastructura urbană: rețeaua electrică, transportul, comunicațiile, și sistemul judiciar. Fiecare cetățean e monitorizat 24/7 prin **NeuralTag** — un implant obligatoriu introdus la naștere.

### Protagonistul — ZERO

| | |
|---|---|
| **Nume** | ZERO (identitate reală necunoscută) |
| **Ocupație** | Ghost hacker — fost angajat VEKTOR, acum fugitiv |
| **Abilitate** | Neural Override — poate hackui orice sistem conectat |
| **Motivație** | A descoperit că VEKTOR planifică un reset total al populației |
| **Weakness** | Implantul său NeuralTag e parțial activ — VEKTOR îl poate localiza |

### Structura narativă — 5 Acte

| Act | Eveniment |
|---|---|
| **ACT I** | ZERO fuge din sediul VEKTOR după ce descoperă Project NULLIFY |
| **ACT II** | Infiltrare în subteranul orașului — contact cu rezistența |
| **ACT III** | Jaf la centrul de date — furt cheie de decriptare |
| **ACT IV** | Trădare internă — unul din rezistență e agent dublu |
| **ACT V** | Penetrarea CORE-ului și execuția protocolului NULLIFY |

---

## 03. GAMEPLAY SYSTEMS

### Movement
- Alergat, mers ghemuit, alunecare (slide)
- Wallrun pe suprafețe verticale max 2 secunde
- Double jump cu jetpack (durată limitată, se reîncarcă)
- Dash directional cu iframe scurt (evitare gloanțe)
- Cățărare pe margini automat (like Celeste)

### Combat
- Vizare liberă cu mouse — crosshair cu recoil vizibil
- Bullet physics: viteza, penetrarea materialelor, ricoșeu
- Cover system — stai după obstacole, poți trage din cover
- Kill cam la headshot / kill creativ (0.3s slow-mo)
- Ragdoll complet la moartea inamicilor (Box2D joints)
- Blood decals în hex cyan/roșu pentru vibe cyberpunk

### Weapon System

| Armă | Descriere |
|---|---|
| **PHANTOM-9** | Pistol silențios. Rapid, precis. Mod: burst / single |
| **STATIC SMG** | SMG electroplasmatic. Recoil mare, rate of fire ridicat |
| **RAILGUN MK2** | Trage prin pereți dacă ai Hack Charge activ. Lent |
| **VOID SHOTGUN** | Spread mare, distanță scurtă. Knockback pe inamici |
| **EMP GRENADES** | Disable electronică 5 sec — camere, drones, inamici augmentați |
| **NEURAL SPIKE** | Aruncă remote — hackuiești inamicul și îl controlezi 3 sec |

### Hacking System
Când ZERO ajunge la un terminal sau e în raza unui sistem hackuibil, apasă `[E]` pentru a intra în modul **HACK**.

Jocul intră în **FREEZE vizual** — timp oprit, scanlines intense pe ecran. Apare un minigame în stilul unui circuit breaker:

| Tier | Minigame | Dificultate |
|---|---|---|
| **TIER 1** | Sequence match (4 simboluri, 3 secunde) | Ușor |
| **TIER 2** | Circuit routing (conectează noduri fără să te intersectezi) | Mediu |
| **TIER 3** | ICE breaker (sparge layere de criptare sub timer agresiv) | Greu |

- **Succes** → uși deschise, camere oprite, inamici dezorientați 8 sec
- **Eșec** → alertă +1, spawn rapid de reinforcements

### Stealth Layer (opțional)
- Cone of vision vizibil pentru fiecare inamic
- Hearing radius — inamicii aud pași, împușcături, geamuri sparte
- Corpuri găsite → alertă ridicată pentru toată zona
- Hackuiești camere → le oprești sau le folosești ca ochii tăi
- Silent takedown din spate (animație rapidă, instant kill)

---

## 04. ENEMY DESIGN

### Tipuri de Agenți VEKTOR

| Tip | Descriere |
|---|---|
| **SCOUT** | Patrulează, alertează alții. Armă ușoară, HP mic. AI: patrol → alert → call |
| **ENFORCER** | Standard grunt. Pistol + vest. Intră în cover, flanchează |
| **SHIELD** | Scut balistic în față. Trebuie flancat sau EMP-at |
| **SNIPER** | Stă pe poziții înalte. Laser sight vizibil roșu. One-shot periculos |
| **HACKER** | Încearcă să îți blocheze Neural Override. Kill priority high |
| **HEAVY** | Minigun, slow, mult HP. Trebuie kited. Strică cover-ul cu ușurință |
| **DRONE** | Patrol aerian. EMP instant kill. Alertează dacă te vede |
| **CYBORG ELITE** | Boss-tier normal. Augmentări multiple, faze diferite de luptă |

### AI Architecture
Fiecare inamic rulează o **state machine cu 5 stări**:

```
PATROL  → mișcare pe waypoints predefinite
ALERT   → a auzit ceva, investighează
COMBAT  → te-a văzut, intră în luptă activă
SEARCH  → te-a pierdut, caută 15 secunde
CALL    → trimite semnal de reinforcement
```

Tranziții bazate pe: LOS (line of sight), hearing radius, bullet impact

### Pathfinding
- A* pe navmesh generat din tilemap
- Inamicii evită obstacole dinamice (corpuri, explozii)
- Flanking logic — 2+ inamici încearcă să te prindă în clește
- Cover selection — aleg cel mai apropiat cover față de tine

---

## 05. VISUAL DESIGN

### Art Direction
Dark cyberpunk neon-noir. Culori dominante: **negru profund, cyan electric, magenta, galben neon**. Contrastul e violent și intenționat. Arhitectura: brutalism + holografic + reclame corporatiste omniprezente.

### Shader Effects (GLSL via SFML)

| Shader | Efect |
|---|---|
| `neon_glow.frag` | Bloom pe pixelii luminoși. Fiecare sursă de lumină neon pulsează |
| `chromatic_aberration.frag` | Activated la damage. RGB channels se separă vizual |
| `scanlines.frag` | Overlay subtil permanent. Dă feel de monitor vechi |
| `glitch.frag` | Activated în hacking mode. Distorsionare, noise, pixel shift |
| `rain.frag` | Parallax rain particles pe layerul de background |
| `vignette.frag` | Vignetă pe margini. Se intensifică la low HP |

### Parallax Layers

| Layer | Conținut |
|---|---|
| **Layer 0** (far) | Skyline neon, clădiri în ceață, lună roșie |
| **Layer 1** | Reclame holografice animate, elicoptere în fundal |
| **Layer 2** | Exterioruri clădiri, fire escapes, conducte |
| **Layer 3** (near) | Prim-plan — platforme, cover, gameplay activ |
| **Layer 4** (fg) | Particule ploaie, fum, scântei — în fața camerei |

### Particle Systems
- Bullet impact → scântei electrice + gaură de glonț
- Enemy death → blood hex cyan + ragdoll
- Explosion → shockwave + debris + fum persistent
- Hack activation → glitch particles violet în jurul terminalului
- Jetpack → glow albastru + fum termic
- Rain → 500–2000 particule, coliziune cu suprafețe

### Sprite Production — PB2 Style

**Stil:** Plasma Burst 2 — clean outlines (1-2px), flat shading cu maxim 4 culori per zonă, fără textură organică, fără brush strokes. Vector-like illustration exportat ca raster PNG cu transparență.

**Workflow:**
```
Concept (Gemini / manual) → curățare în Affinity Designer / Photoshop
→ export PNG transparent → TexturePacker atlas → SFML setSmooth(true)
```

**Canvas de lucru:** 2048×2048 per personaj (precizie la desenat, export la dimensiunea finală).

**Dimensiuni export (runtime PNG):**

| Personaj | Export (native) | On-screen @ 1080p |
|---|---|---|
| ZERO | 512×512 | ~160px |
| SCOUT | 384×384 | ~110px |
| ENFORCER | 384×384 | ~120px |
| SHIELD | 512×512 | ~130px |
| SNIPER | 384×384 | ~125px |
| HACKER | 256×256 | ~100px |
| HEAVY | 512×512 | ~170px |
| DRONE | 384×256 (landscape) | ~90px |
| CYBORG ELITE | 512×512 | ~200px |
| Arme (separate) | 256×128 | ~80px |

**Paleta per personaj** (din cele 10 culori canonice):

| Personaj | Bază armură | Accent neon | Detaliu |
|---|---|---|---|
| ZERO | `#0A1020` + `#1A2840` | `#00FFEE` trim | glugă `#050810` |
| SCOUT | `#1A2840` | `#FF006B` trim | vizieră `#00FFEE` |
| ENFORCER | `#1A2840` vest | `#FF006B` | `#6080A0` detalii |
| SHIELD | `#050810` scut | `#FFE600` avertizare | `#1A2840` corp |
| SNIPER | `#050810` | `#FF0038` laser dot | scope `#6080A0` |
| HACKER | `#0A1020` civilian | `#AA00FF` device | `#6080A0` |
| HEAVY | `#1A2840` gros | `#FF0038` + `#FFE600` | minigun `#6080A0` |
| DRONE | `#050810` | `#00FFEE` rotoare | `#FF006B` sensor |
| CYBORG ELITE | `#0A1020` + `#6080A0` metal | `#AA00FF` augmentări | `#FF0038` eye |

**Silhouette rules:** Fiecare personaj recognoscibil ca silhouette solid negru. Element definitoriu per tip: ZERO=glugă forward-lean, SHIELD=scut rectangular, HEAVY=minigun pe umăr, DRONE=formă hexagon cu rotoare, CYBORG ELITE=braț mecanic proeminent.

**SFML runtime:** `texture.setSmooth(true)`, scale calculat la spawn: `scale = targetHeight / nativeHeight`, setat o dată — zero cost în hot path.

---

## 06. TECHNICAL ARCHITECTURE

### Tech Stack

| | |
|---|---|
| **Limbaj** | C++20 |
| **Rendering** | SFML 2.6 (OpenGL backend) |
| **Physics** | Box2D 2.4 |
| **Shaders** | GLSL 3.3 |
| **Level format** | JSON (nlohmann/json) |
| **Build system** | CMake 3.20+ |
| **Asset pipeline** | TexturePacker + custom tool |

### Structura Proiectului

```
nullify/
├── src/
│   ├── core/
│   │   ├── Game.cpp/h          # main loop, delta time, states
│   │   ├── GameState.cpp/h     # state machine (menu/play/hack/pause)
│   │   └── EventBus.cpp/h      # decoupled event system
│   ├── ecs/
│   │   ├── World.cpp/h         # entity registry
│   │   ├── Components.h        # POD structs (Transform, Health, etc)
│   │   └── Systems/            # PhysicsSystem, RenderSystem, AISystem...
│   ├── player/
│   │   ├── Player.cpp/h
│   │   ├── WeaponSystem.cpp/h
│   │   └── HackSystem.cpp/h
│   ├── enemies/
│   │   ├── Enemy.cpp/h
│   │   ├── AIStateMachine.cpp/h
│   │   └── Pathfinding.cpp/h
│   ├── world/
│   │   ├── TileMap.cpp/h
│   │   ├── LevelLoader.cpp/h
│   │   └── Camera.cpp/h
│   ├── rendering/
│   │   ├── Renderer.cpp/h
│   │   ├── ShaderManager.cpp/h
│   │   └── ParticleSystem.cpp/h
│   ├── ui/
│   │   └── HUD.cpp/h
│   └── utils/
│       ├── PoolAllocator.h     # zero heap alloc în game loop
│       └── MathUtils.h
├── assets/
│   ├── sprites/
│   ├── shaders/
│   ├── audio/
│   └── levels/
└── CMakeLists.txt
```

### Performance Architecture

#### Entity Component System (ECS)
Data-oriented design — struct of arrays pentru cache locality maximă. Zero virtual dispatch în hot path. Toate componentele sunt POD structs.

```cpp
// Components — POD structs, cache-friendly
struct Transform  { float x, y, rotation; };
struct Velocity   { float vx, vy; };
struct Health     { int current, max; };
struct Renderable { sf::Sprite sprite; int layer; };

// Systems iterează linear pe arrays
void PhysicsSystem::update(float dt) {
    auto& transforms = world.getComponents<Transform>();
    auto& velocities  = world.getComponents<Velocity>();
    for (size_t i = 0; i < count; ++i) {  // SIMD-friendly loop
        transforms[i].x += velocities[i].vx * dt;
        transforms[i].y += velocities[i].vy * dt;
    }
}
```

#### Pool Allocator pentru Bullets & Particles
```cpp
template<typename T, size_t POOL_SIZE>
class PoolAllocator {
    T pool[POOL_SIZE];          // pre-allocated, zero heap
    bool active[POOL_SIZE]{};
    size_t nextFree = 0;
public:
    T* allocate() {             // O(1), no malloc()
        return &pool[nextFree];
    }
    void free(T* ptr) { active[ptr - pool] = false; }
};

PoolAllocator<Bullet,   1024> bulletPool;
PoolAllocator<Particle, 4096> particlePool;
```

#### Batch Rendering
Toate sprite-urile cu același texture atlas se desenează într-un singur draw call. SpriteBatch sort by texture + layer → minimum OpenGL state changes.

```
Target: < 10 draw calls per frame pentru tot gameplay-ul
  Layer 0-4 parallax:   5 draw calls
  Gameplay sprites:     1-2 draw calls (atlas)
  Particles:            1 draw call (instanced)
  UI/HUD:               1 draw call
  Post-process shaders: 1 pass
```

---

## 07. LEVEL DESIGN

### Structura unui Level

| Layer | Conținut |
|---|---|
| **Layer Decorativ** | Reclame neon, graffiti, țevi, aer condiționat |
| **Layer Gameplay** | Platforme, cover objects, terminale hackuibile |
| **Layer Coliziune** | Tilemap coliziune Box2D |
| **Spawn Points** | Player spawn, enemy waypoints, item drops |
| **Trigger Zones** | Cutscene triggers, level transitions, alert zones |

### Level Progression — Actele Jocului

| Levels | Zonă | Focus |
|---|---|---|
| 1-1 → 1-3 | NEXUS-7 Streets | Introducere mecanici, inamici basic |
| 2-1 → 2-4 | Underground | Rezistența, hacking introdus, sniperi |
| 3-1 → 3-3 | VEKTOR Data Center | Stealth heavy, drone patruls |
| 4-1 → 4-2 | Inner Sanctum | Trădare, luptă în timp ce totul se prăbușește |
| 5-1 (FINAL) | CORE Access | Boss fight + protocol NULLIFY |

### Verticality & Platforming
- Fiecare level are minim 3 rute alternative (sus / mijloc / jos)
- Rooftops accesibile prin wallrun / jetpack pentru sniper advantage
- Subteran cu vizibilitate redusă — avantaj stealth
- Reclame holografice = platforme temporare (hackuibile)

---

## 08. AUDIO DESIGN

### Soundtrack
Dark synthwave + industrial electronic. Tempo se adaptează la starea jocului:

| Stare | Muzică |
|---|---|
| **Stealth / Exploration** | Ambient lent, bass profund, synth melancolic |
| **Combat** | BPM crește, kick industrial, lead synth agresiv |
| **Hacking minigame** | Glitch music, arpegii distorsionate, no drums |
| **Boss fight** | Full orchestral electronic, tension maximă |

### Sound Design

| Categorie | Detalii |
|---|---|
| **Arme** | Fiecare armă are 3-5 variante audio, random selection per shot |
| **Footsteps** | Material detection: metal / beton / apă / lemn |
| **UI** | Beep-uri CRT vintage pentru menus, glitch sounds pentru hack |
| **Environment** | Ploaie procedurală, trafic, drone hum, neon electric buzz |
| **Impact** | Bullet hit: metal / carne / sticlă / electronic — 4 categorii |

---

## 09. DEVELOPMENT ROADMAP

### Luna 1 — Foundation ✅ Sprint 1 (v0.1)
- [x] Window + game loop + delta time fix
- [x] CMakeLists.txt cu SFML + Box2D
- [x] Player movement basic (alergat, sărit, căzut)
- [x] Tilemap loader din JSON
- [x] Camera urmărire player cu lerp

### Luna 2 — Combat Core ✅ Sprint 2 (v0.2) — ragdoll pendent
- [x] Weapon system — pistol funcțional (PHANTOM-9, pool 1024 bullets)
- [x] Bullet physics cu Box2D (raycast collision, BulletHitEvent)
- [x] Enemy basic (SCOUT) cu state machine (PATROL→ALERT→COMBAT→SEARCH + A*)
- [ ] Ragdoll la moarte — enemies dispar instant; Box2D joints ragdoll neimplementat
- [x] HUD basic: HP bar, ammo counter (+ alert level indicator)

### Luna 3 — Visual Polish ✅ Sprint 3 (v0.3)
- [x] Shader manager + neon glow (bloom 12-tap via RenderTexture + ShaderManager)
- [x] Particle system — bullet impact (cyan sparks) + blood (cyan/red), pool 4096
- [x] Parallax background system (3 layere, speed factors 0.1/0.3/0.6)
- [x] Chromatic aberration la damage (0.8s decay, integrat în neon_glow.frag)
- [x] Scanlines overlay (permanent, GLSL)

### Luna 4 — Gameplay Depth
- [ ] Hacking minigame complet
- [ ] Toate tipurile de inamici
- [ ] Stealth system (cone of vision, hearing)
- [ ] Toate armele implementate
- [ ] Level 1-1 → 1-3 complet

### Luna 5 — Content & Polish
- [ ] Audio complet (SFML Audio / OpenAL)
- [ ] Toate levelele Act I + II
- [ ] Main menu + pause menu
- [ ] Save system
- [ ] Performance profiling + optimizare finală

### Post-launch
- [ ] Act III → V (restul jocului)
- [ ] Controller support
- [ ] Level editor intern
- [ ] Steam release

---

## 10. SETUP INIȚIAL

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(NULLIFY VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto")
endif()

find_package(SFML 2.6 COMPONENTS graphics window audio system REQUIRED)
find_package(box2d REQUIRED)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(nullify ${SOURCES})

target_link_libraries(nullify PRIVATE
    sfml-graphics sfml-window sfml-audio sfml-system
    box2d
)

target_include_directories(nullify PRIVATE src/)

add_custom_command(TARGET nullify POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:nullify>/assets
)
```

### Instalare dependințe — Arch Linux

```bash
# SFML
sudo pacman -S sfml

# Box2D
sudo pacman -S box2d

# CMake + build tools
sudo pacman -S cmake ninja base-devel

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/nullify
```

---

*[ END OF DOCUMENT — NULLIFY GDD v0.1 ]*

**hack the planet.**
