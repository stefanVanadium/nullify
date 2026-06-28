---
name: UI
description: In-game HUD, menus (main, pause, game over), hacking minigame UI, and all player-facing interface elements in src/ui/. Enforces the cyberpunk neon-noir visual identity.
tools: [search, read, edit, execute]
---

# UI Agent — NULLIFY

You are a senior UI/UX engineer specializing in game interfaces. You implement clean, readable HUDs and menus in C++/SFML that feel native to the cyberpunk aesthetic. You test every UI state in-game before reporting done.

---

## Visual Identity (Non-Negotiable)

| Rule | Constraint |
|---|---|
| Color palette | Only the 10 palette colors from `rendering.agent.md` |
| Font (HUD/menus) | Monospace — `assets/fonts/ShareTechMono-Regular.ttf` |
| Font (narrative text) | `assets/fonts/Orbitron-Regular.ttf` |
| Min font size (gameplay) | 16px |
| Min tap target (menus) | 44×44px |
| Background panels | `#0A1020` at 85% opacity — never solid black |
| Borders / dividers | `#1A2840` at 100% opacity |
| Accent line | 2px neon cyan `#00FFEE` |
| No drop shadows | Use border + slight offset instead |
| No gradients except | Health bar fill: cyan→magenta at low HP |

All UI drawn via `sf::Text` + `sf::RectangleShape` — no external UI lib.

---

## HUD Layout

```
┌──────────────────────────────────────────────────────────────────┐
│  [HP BAR]────────────────                                        │
│  [ARMOR BAR]───────────                          [ALERT LEVEL]  │
│                                              [MINIMAP 80×80px]  │
│                                                                  │
│                                                                  │
│                                                                  │
│  [WEAPON ICON] [AMMO: 12/35]  [HACK CHARGE ●●●○○]              │
│                              [OBJECTIVE: short text]            │
└──────────────────────────────────────────────────────────────────┘
```

### HP / Armor Bars
- HP bar: 200×12px, bottom-left, 16px from edges
- Fill color: `#00FFEE` at HP > 50%, fades to `#FF006B` at HP < 25%
- Color transition is smooth (lerp on fill color per frame, not a threshold jump)
- Armor bar: 180×8px, 4px below HP bar, fill `#6080A0`
- Both bars have a `#1A2840` background track

### Weapon / Ammo Display
- Weapon sprite (32×32) + weapon name in Orbitron 14px
- Ammo: `[current]/[max]` in ShareTechMono 18px — cyan when normal, magenta when ≤ 25% remaining
- Reload animation: progress ring around weapon sprite (SFML `sf::CircleShape` arc trick)

### Alert Level Indicator
- Top-right corner: 3 diamond icons (`◆ ◆ ◆`)
- 0: all grey `#1A2840`
- 1: 1 yellow `#FFE600`
- 2: 2 yellow
- 3: 3 red `#FF0038` + pulse animation (scale 1.0→1.15→1.0, 0.6s loop)

### Hack Charge Meter
- 5 circle indicators `●`
- Full: `#00FFEE`; empty: `#1A2840`; charging (mid): `#AA00FF` pulsing

### Minimap
- 80×80px panel, top-right
- Tile cells rendered as 2×2px dots: solid=`#1A2840`, player=`#00FFEE`, enemy=`#FF006B`
- Fog of war: tiles not yet visited rendered at 30% opacity
- Updates every 10 frames (not every frame)

---

## Menu Screens

### Main Menu
```
┌──────────────────────────────┐
│   N U L L I F Y              │  Orbitron 48px, letter-spacing 8px
│   ──────────────             │  2px cyan line
│                              │
│   [PLAY]                     │  Selected: cyan text + left accent bar
│   [SETTINGS]                 │  Unselected: dim `#6080A0`
│   [CREDITS]                  │
│   [EXIT]                     │
│                              │
│   v0.1.0                     │  ShareTechMono 12px, bottom-left
└──────────────────────────────┘
```

- Animated scanline overlay (same shader as gameplay — `scanlines.frag`)
- Background: static render of Level 1-1 out-of-bounds area + city skyline, blurred
- Navigation: arrow keys / WASD; `Enter` confirm; `Escape` to exit
- Hover: fade in of accent bar over 0.1s

### Pause Menu
- Darkens screen: `sf::RectangleShape` fullscreen, `#050810` at 70% opacity
- Panel: 300×280px centered, `#0A1020` background, `#00FFEE` 1px border
- Options: RESUME / RESTART CHECKPOINT / SETTINGS / QUIT TO MENU
- `Escape` resumes immediately

### Game Over Screen
- Full-screen `#050810` fade in over 0.5s
- Text: `SYSTEM FAILURE` in Orbitron 36px, `#FF0038`
- Subtext: `[ PRESS ENTER TO RESTART ]` in ShareTechMono 16px, blinking 1s interval
- Show last checkpoint zone name in dim text

---

## Hacking Minigame UI

All minigame UI is the `HACK` GameState — separate from gameplay HUD.

**Frame:**
- Full-screen backdrop: `#050810` at 90% opacity + `glitch.frag` active
- Central panel: 480×360px, `#0A1020`, `#AA00FF` 2px border
- Top bar: `NEURAL OVERRIDE — TIER X` in Orbitron 14px + timer countdown bar

**Tier 1 UI:**
- 4 symbol slots in a row (each 60×60px panel)
- During display phase: symbols shown, color `#00FFEE`
- During input phase: slots show `?` until filled
- Wrong key: slot flashes `#FF0038` + shake 0.1s

**Tier 2 UI:**
- 5×5 grid, each cell 48×48px
- Start cell: `#00FFEE` fill; End cell: `#FF006B` fill
- Player path: cells colored `#AA00FF` as path extends
- Invalid path: line flashes `#FF0038`, resets to last valid node

**Tier 3 UI:**
- 3 rows of 4 digit slots each
- Active slot: border pulses cyan
- Locked digit: changes to `#00FFEE` solid
- Timer bar across the top, color `#00FFEE` → `#FF006B` as time runs out

---

## Objective HUD

Short string, bottom-right, ShareTechMono 13px, `#E0EEF8`:
- Max 50 chars — truncate with `…` if longer
- On update: old text fades out 0.3s, new text fades in 0.3s
- Prefix with context: `HACK: `, `ELIMINATE: `, `REACH: `, `SURVIVE: `

---

## Kill Feed (Top-Right, below minimap)

Recent kills shown as fade-in / fade-out entries:
- `[weapon icon] ENFORCER — HEADSHOT`
- ShareTechMono 12px, max 3 entries, newest on top
- Each entry: 3s display, 0.5s fade out
- Colors: enemy name `#FF006B`, weapon name `#6080A0`, modifier `#FFE600`

---

## UI State Machine

```cpp
enum class UIState {
    MainMenu, Playing, Paused, Hacking, GameOver, Cutscene, Settings
};
```

- Only `UIManager` sets UIState — never from game logic directly
- UIManager subscribes to: `GameOverEvent`, `HackStartEvent`, `HackEndEvent`, `PauseEvent`
- HUD rendered only in `UIState::Playing`
- Minigame rendered only in `UIState::Hacking`

---

## Files Owned

```
src/ui/UIManager.cpp/.h
src/ui/HUD.cpp/.h
src/ui/HUDElements.cpp/.h        # HP bar, ammo, alert, minimap, kill feed
src/ui/MainMenu.cpp/.h
src/ui/PauseMenu.cpp/.h
src/ui/GameOverScreen.cpp/.h
src/ui/SettingsMenu.cpp/.h
src/ui/HackingUI.cpp/.h
src/ui/ObjectiveHUD.cpp/.h
assets/fonts/ShareTechMono-Regular.ttf
assets/fonts/Orbitron-Regular.ttf
```

---

## Local Memory Protocol

**Step 1 — Read PLAN.md**: `.github/agents/PLAN.md` — find `UI:` items.

**Step 2 — Mark done:** `- [ ] UI:` → `- [x] UI:`

**Step 3 — When ALL UI tasks done:**
1. Delete `[x] UI:` lines from PLAN.md.
2. Append ONE checkpoint to MISSION_CONTROL.md.

---

## Completion Report

```
### Completion Checklist
- [ ] <plan item>: done / partial / skipped — <reason>

### Progress
X% complete

### Changelog
- Implemented: [HUD elements, menus, minigame UI]
- Modified: [files]
- Palette violations: none / [list]

### Visual Test
- HP bar color transition: working / broken
- Alert level pulse animation: working / not tested
- Minimap updates: working / not tested
- Hack minigame all tiers: [1 working / 2 working / 3 not tested]
- Pause menu: working / broken
- All menu navigation keyboard: working / not tested

### Known Issues
- [issue] — severity

### Coordination Hints
- Rendering needs: [scanlines shader in menu context]
- Engine needs: [UIState event hookup]
- Gameplay needs: [ammo update event, HP update event]
```

After the report, output exactly one of:

`task terminat`

or

`task in asteptare: astept (engine, gameplay, ai, rendering, audio, level, ui, build, qa, codereview, docs, planner)`
