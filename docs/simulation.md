# Simulation Model

## Particle State

Each particle stores:

- position: `posX`, `posY`
- velocity: `velX`, `velY`
- type index: `type`

The particle type determines how it interacts with every other type.

## Simulation Parameters

The runtime parameter block includes:

- `particleCount`
- `numTypes`
- `deltaTime`
- `frictionFactor`
- `forceScale`
- `maxDistance`
- `minDistance`
- `repulsionStrength`
- `worldSize`

These values shape the strength, distance falloff, and stability of the interaction field.

## Attraction Matrix

The engine stores a fixed-size attraction matrix with room for `8 x 8` type interactions. Each cell represents how type `A` responds to type `B`.

- Positive values produce attraction
- Negative values produce repulsion
- Stronger magnitudes produce more aggressive motion patterns

Randomizing this matrix produces distinct emergent behaviors without changing the shader code.

## Spatial Rules

The simulation world wraps around. When particles move beyond one edge, they reappear on the opposite side. This makes the world topologically toroidal and avoids hard-boundary clumping.

Close-range repulsion prevents total collapse, while medium-range interactions drive clustering, orbiting, and separation patterns.

## Practical Limits

- Default particle count: `4000`
- Maximum particle count: `50000`
- Default type count: `6`
- Maximum type count: `8`

Higher counts increase visual richness, but they also increase compute cost per frame.
