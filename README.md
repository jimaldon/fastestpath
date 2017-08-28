# Fastest Path: Bachelor Problem

## Scenario

A bachelor stranded on an island `(BACHELOR_X, BACHELOR_Y)`, needs to get to his wedding location `(WEDDING_X, WEDDING_Y)` using an AUDI rover `(ROVER_X, ROVER_Y)`; both located in the same island.

The island map has `2048x2048` cells, each of which have a terrain type and elevation index.

The terrain types are:
* Rivers
* Marsh land
* Water basin
* Land

The rover can only traverse on land. Additionally, every cell on land has an elevation value between `1` to `255`. The rover travels with a constant speed of `1 cell per island second`. It is faster on slopes going down and slower on positive slopes.

## Task

1. Build a rover speed model with plausible assumptions.

2. Find the fastest path from `ROVER` to `BACHELOR` to `WEDDING`.

## Assumptions

1. 1 island second = 1 second.

2. The rover can traverse any gradient on `elvevation > 0` on the island.

3. Maximum angle of slope between two adjacent cells is `45 $\degrees$`, since it's assumed the rover cannot physically climp a steeper slope without slipping.

4. Car model assumed to be the **AUDI Q5** with the following parameters:
  * Kerb weight or mass = `1850 kg`
  * Maximum power = `200 kW`
  * Top speed (at maximum power) = `200 kph` or `55.5 m/s`

