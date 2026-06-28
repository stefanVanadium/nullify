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

### QA confirmată
- [x] QA:01 Build curat zero warnings/errors
- [x] QA:02 ZERO apare la spawn
- [x] QA:05 ZERO cade pe podea (Box2D funcționează)
- [x] QA:08 FPS counter în titlu ("NULLIFY v0.1 | 142 FPS" confirmat screenshot)
- [x] QA:10 Procesul se închide clean
- [ ] QA:03/04/06/07 — necesită testare manuală cu input
- [ ] QA:09 — zero heap allocs (necesită valgrind)

### Note tehnice
- FetchContent descarcă deps la prima configurare CMake (~60s)
- XWayland screenshot via spectacle (ffmpeg x11grab nu capturează GL composited)
- Camera x clampată la 640 (level 1920px > window 1280px) — funcționează corect
- Camera y clampată la 360 (level 640px < window 720px — guard implementat)

---
