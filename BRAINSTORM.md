# Powder, but better: a brainstorm

A native macOS falling-sand sandbox in the spirit of *Powder Game* (Dan-Ball) and *The Powder Toy*,
with real thermodynamics, emergent combustion, and machines you can build.

Working title ideas: **Kiln**, **Flashpoint**, **Pyrolab**, **Ember**, **Combust**.

---

## 1. What "better" has to mean

Powder Game is a cellular automaton: every pixel is a material, and hand-written rules decide what
happens when two materials touch ("fire + wood → fire"). The Powder Toy added air pressure, heat,
and electricity. That's the competition, so "better" needs more than a longer element list:

| Powder Game / TPT | This game |
|---|---|
| Fire is a particle type | Fire is a **process**: fuel + oxygen + heat → products + energy |
| Rules are hard-coded per element pair | Materials have **properties**, and behaviour comes from physics plus a data-driven reaction table |
| Heat is a number per cell | Heat moves by conduction, convection and radiation, and phase changes absorb latent heat |
| Air is a coarse pressure grid | Gas is a real fluid that carries **species** (O₂, fuel vapour, CO₂, H₂O, smoke) |
| Solids are static walls | Solids are **rigid bodies** that hold stress, crack, shatter and move |
| You can't build a working engine | You **can**: combustion pressure pushes pistons, turns cranks, spins turbines |

The core pitch: **build a working engine out of pixels.** A cylinder made of steel, a piston on a
joint, a spark, fuel vapour. Then watch it run, overheat, knock, and blow up.

### Direction so far

- **Art: hybrid lit pixel, "pixels as the canvas, light as the paint".** Solids, powders and
  liquids are crisp pixels with real material texture (wood grain, sand grain mixes, brushed steel).
  Fire, embers and molten metal are see-through and glowing, light the scene and cast shadows.
  Smoke is soft. No flat colour fills, which read as childish.
- **Setting: a clean test lab, not a cave.** Dark lab (charcoal panels) by default, with a white lab
  theme. Walls are panels with a faint measuring grid and ruler ticks, the floor is a steel-edged
  bench. Fire, soot and scorch marks stand out against the clean surfaces.
- **UI: game hotbar (UI 3) as the default, plus a lab mode (split view, probes, graphs).** The play
  screen gets a 1–9 element hotbar, a tool rail, element tooltip cards and challenge cards. Lab
  mode adds a split view (normal / heat / O₂ / pressure / stress), pinned probes and live graphs.
- Mockups: the "Kiln UI & Art Style Mockups" design canvas (UI 3 and UI 4 boards).

---

## 2. Engine architecture: layered hybrid simulation

One grid can't do all of this well, so stack several simulations, each at the resolution it needs,
and couple them every tick.

```
┌───────────────────────────────────────────────────────────────┐
│ L4  Rigid bodies     pixel-shaped bodies, joints, motors,     │
│                      fracture into particles (Noita-style)    │
├───────────────────────────────────────────────────────────────┤
│ L3  Particle grid    1 cell = 1 pixel. Material, temperature, │
│     (full res)       phase, sub-cell velocity, lifetime       │
├───────────────────────────────────────────────────────────────┤
│ L2  Gas/fluid field  ¼ res. Velocity, pressure, temperature,  │
│     (coarse)         species: O₂, fuel, CO₂, H₂O, CO, soot    │
├───────────────────────────────────────────────────────────────┤
│ L1  Networks         electricity (resistance, Joule heating), │
│                      sensors, logic                           │
└───────────────────────────────────────────────────────────────┘
```

### L3: particle grid (the "sand")
- Falling-sand CA for powders, liquids and loose debris.
- Per-material properties instead of per-pair rules: density, friction (sets the angle of repose),
  viscosity, specific heat, thermal conductivity, melting and boiling points, latent heats,
  flammability data, hardness and tensile strength.
- Particles get sub-cell velocity so explosions *throw* sand in arcs instead of teleporting it a cell at a time.
- Liquids layer by density on their own (oil floats on water, mercury sinks).

### L2: gas field (the big upgrade)
- Eulerian "stable fluids" solver (Jos Stam) on a coarse grid: advection, pressure projection, buoyancy.
- Hot gas rises because it's less dense, not because a rule tells fire to move up.
- Each cell carries **species concentrations**, which is what makes combustion emergent.
- Particle cells act as solid boundaries. Gas leaks through porous materials at a rate set per material.

### L4: rigid bodies
- Box2D-style solver, or a custom one, where bodies are built from pixel masks.
- Paint steel and it becomes a body. Overstress it and it breaks into chunks or sand.
- Joints, hinges, sliders, axles and motors give you pistons, crankshafts, flywheels and gears.
- Two-way coupling: gas pressure produces forces on body boundaries, and moving bodies push gas
  and particles out of the way. **This coupling is what lets engines work.**

### L1: electricity
- Conductors have resistance, current heats them (I²R), and a thin tungsten wire glows.
- Batteries, sparks (which ignite gas mixtures), electrolysis (water → H₂ + O₂), and simple logic.

---

## 3. Combustion model (the centerpiece)

"Plausible, not accurate." Real chemistry in spirit, game-tuned in the numbers.

**Fire triangle as data.** Something burns when it has fuel, oxygen and enough heat. Remove any one
and it goes out. That gives you smothering, CO₂ extinguishers and water cooling for free.

**Pyrolysis.** Solids don't burn directly. Heated wood releases flammable gas into the L2 field,
and that gas is what burns. This produces:
- visible flame fronts that lift off the surface,
- **smouldering** (glowing char with low O₂) versus **flaming** (gas phase),
- **flashover**: a room heats until everything pyrolyses at once,
- **backdraft**: an O₂-starved fire fills a room with hot fuel gas, then someone opens a door.

**Reaction rate.** Use Arrhenius-style kinetics, `rate = A · e^(−Ea / T) · [fuel] · [O₂]`, plus
flammability limits (a mixture that's too lean or too rich won't burn). Rich burns make CO and soot,
so you get black smoke. Lean burns are clean and blue.

**Energy bookkeeping.** Each reaction releases a set heat of combustion into the cell's temperature.
Gas expansion follows from the ideal gas law, and that pressure is what drives pistons.

**Deflagration vs detonation.**
- *Deflagration* is a subsonic flame front. Pressure builds gradually, which is what an engine
  cylinder wants.
- *Detonation* is a supersonic shock wave. It shatters rigid bodies, and you get it from high
  explosives or from **engine knock** (too much compression → autoignition → a damaged piston).
- **Dust explosions**: flour, coal dust or sawdust suspended in air, plus one spark.

**Reaction table (data-driven).** For example:
```toml
[[reaction]]
name      = "thermite"
reactants = ["aluminium_powder", "rust"]
requires  = { min_temp = 1500 }
products  = ["molten_iron", "alumina"]
enthalpy  = -850   # strongly exothermic
```
The same system handles sodium + water, acids dissolving metals, slow rusting, catalysts,
gunpowder (oxidiser built in, so it burns underwater), and electrolysis.

**Looks that tell you the physics.**
- Flame colour from temperature (blackbody: dull red → orange → white → blue-white), plus
  emission tints for additives (sodium yellow, copper green, lithium red, potassium lilac).
- Fireworks come from the chemistry, not from a fireworks element.

---

## 4. Thermodynamics

- **Conduction**: heat diffusion that uses each material's conductivity and heat capacity.
  Copper spreads heat, brick insulates.
- **Convection**: free, because the gas field moves the heat.
- **Radiation**: a cheap approximation (blur of T⁴ at coarse resolution) so a furnace warms what
  it can "see".
- **Phase changes with latent heat**: ice sits at 0 °C while it melts, water boils at 100 °C and
  stays there until it's all steam. Steam condenses on cold surfaces.
- **Pressure-dependent boiling** (stretch goal) makes pressure cookers and boiler explosions possible.
- **Thermal stress**: quench hot glass and it cracks.

---

## 5. Structure, collapse and gravity

Things should fall down for real reasons. A burning shed sags and then collapses because its posts
charred through, not because a script says so.

**Structural integrity**
- Every solid has a **strength** (compressive and tensile) and **bonds** to its solid neighbours.
- **Load paths**: weight flows down through bonds to anything anchored (bedrock, pinned walls).
  Long unsupported spans take bending load, so cantilevers and bridges sag and snap at the weak point.
- **Islands**: when bonds break, a flood fill finds pieces no longer connected to an anchor. Those
  pieces become **rigid bodies** that fall, tumble and land. Hard impacts shatter them into debris.
- **Heat weakens things**: charred wood loses most of its strength; steel above about 550 °C loses
  half its yield strength and starts to sag; concrete spalls. A fire can bring a building down.
- **Damage from forces**: explosion shock waves, impacts and pressure break bonds. Brittle
  materials (glass, brick) crack and shatter; ductile ones (steel, copper) bend first.
- **Granular collapse**: sand piles keep their angle of repose until they're undercut, wetted or
  shaken, then they slide. Water erodes sand and dirt.
- **Joints**: rope, chain, hinges and welds are bonds with their own strength, so you can build
  bridges, cranes and towers that fail in believable ways.

**World settings (sliders, in a world panel)**
- **Gravity**: strength (0 → 3 g) and **direction** (rotate it with a trackpad gesture). Also
  per-zone gravity fields and point attractors ("planets").
- **Zero-g is emergent**: with no gravity there's no buoyancy, so hot gas doesn't rise. Flames turn
  into small blue spheres that smother themselves, the way they do on the ISS.
- **Air**: density and pressure (thin air means weaker flames and lower boiling points), oxygen
  level (pure O₂ makes everything violently flammable), ambient temperature, and wind.
- **Time scale**: slow motion for watching collapses and explosions frame by frame.
- **Per-material physics**: friction, restitution (bounciness), density and strength are all
  editable in the material editor.

**Views that make it readable**
- A **stress view** in lab mode colours each solid by load relative to its strength, so you can
  see where a structure will fail before it does.
- Cracks render on the material as it gets close to failing.

---

## 6. Machines and "engines" as gameplay

The rigid-body plus gas-pressure coupling makes these buildable from parts:

- **Steam engine**: a boiler, a valve, a cylinder and a flywheel.
- **Internal combustion**: intake, compression, spark, power, exhaust. Timing matters.
- **Jet or turbine**: a continuous burn spinning a bladed rigid body.
- **Rocket**: a nozzle shape actually changes thrust.
- **Stirling engine**: hot side, cold side, displacer.

**Instruments** turn the sandbox into a lab: thermometer and pressure probes, an RPM/torque gauge
on any axle, live graphs (temperature over time, P–V diagram for a cylinder), and an efficiency
readout comparing fuel energy in with work out.

**Modes:**
- *Sandbox*: no limits.
- *Lab*: instruments and slow motion, with pause and step.
- *Challenges*: "Lift 10 kg using only candles", "Build an engine with more than 30% efficiency",
  "Stop this warehouse fire with 50 units of water".

---

## 7. Why macOS native, and what to use

**Stack recommendation**
- **Swift + SwiftUI** for the app shell, inspector panels, material browser and settings.
- **C++ (or Swift with `unsafe` buffers) core** for the particle CA on the CPU. It's multithreaded
  by chunk: 64×64 chunks, a 4-pass checkerboard so neighbouring threads never collide, and dirty
  rectangles so idle regions cost nothing. This is the Noita approach, and complex per-cell rules
  are much easier to write on the CPU.
- **Metal compute shaders** for the gas field, heat diffusion and radiation. These are uniform grid
  math, which GPUs are made for.
- **Apple Silicon unified memory** is the killer feature here. The CPU particle grid and the GPU
  fields share the same buffers with no copying, which makes the hybrid design cheap. On a discrete
  GPU PC this coupling is expensive.

**Rendering (Metal)**
- **EDR/HDR on XDR and Liquid Retina displays**: fire and molten metal can be *brighter than
  white*. Few sandbox games do this, and it would look striking.
- Bloom, heat-shimmer refraction from temperature gradients, soft smoke, and fire lighting the
  scene (2D radiance cascades for global illumination).
- 120 Hz ProMotion rendering, with the simulation on a fixed timestep (60 Hz) and interpolation.

**Mac-native feel**
- Trackpad: pinch to zoom, two-finger pan, Force Touch pressure for brush strength.
- Debug view modes on hotkeys: temperature, pressure, O₂, velocity, stress, current.
- Saves as documents (iCloud Drive, Quick Look thumbnails, drag a `.kiln` file onto the window).
- **Deterministic simulation** (fixed step, seeded RNG) means replays and shared saves play
  back identically.

**Performance target**
- Particle grid of about 1024×576 at 60 ticks/s on a base M1.
- Gas field at ¼ resolution (256×144) with 6–8 species channels.
- A few hundred rigid bodies.

---

## 8. Modding and data

- Materials, reactions and colours live in TOML/JSON files, not in code.
- Scripting via Lua (or JavaScriptCore, which ships with macOS) for custom elements and challenges.
- In-app material editor: tweak ignition temperature and watch the behaviour change live.

---

## 9. Roadmap sketch

1. **Skeleton**: Swift app, Metal renderer, particle grid with sand, water, wall and a brush tool.
   Gravity is a vector from day one (strength and direction), so it can be a slider later for free.
2. **Heat**: temperature per cell, conduction, phase changes (ice/water/steam, rock/lava).
3. **Gas field**: stable-fluids solver on the GPU, buoyancy, smoke, O₂ channel.
4. **Combustion v1**: pyrolysis, the fire triangle, flame colours, smoke and soot.
5. **Pressure and explosions**: gas expansion, shock waves, particles thrown with velocity.
6. **Rigid bodies and structure**: pixel bodies, bonds and strength, load paths, unsupported
   islands breaking off, fracture, joints, heat weakening. Milestone: **a burning shed collapses**.
7. **Coupling**: gas pressure ↔ bodies. This is the milestone where **the first piston moves**.
8. **Electricity and instruments**.
9. **World settings**: gravity direction and zones, air pressure, O₂ level, wind, time scale.
10. **Challenges, saves, sharing, polish**.

The first real milestone is steps 1–4: a candle that burns out under a glass jar because it used
up the oxygen. If that looks and feels good, the concept is proven.

---

## 10. Risks

- **Scope.** Every layer is a project of its own. Cut ruthlessly, and treat steps 1–4 as the game
  until proven otherwise.
- **Coupling bugs.** Two-way gas ↔ rigid body coupling is where things explode for the wrong
  reasons. Budget time for stability work.
- **Realism vs fun.** Real constants make fire too slow or too fast. Every constant needs a
  "game scale" knob.
- **GPU vs CPU split.** Falling sand is awkward on the GPU because two particles can move into the
  same cell. Keep it on the CPU unless profiling says otherwise.

---

## 11. Open questions

- Is "engine" the **simulation engine** (tech showcase, pure sandbox), or are **in-game engines and
  machines** the goal? This doc assumes both.
- Mac-only forever, or keep the core portable (C++) for an iPad version later? Apple Pencil plus
  a sandbox is a natural fit.
- Single player only, or online save sharing like Powder Game's upload board?
