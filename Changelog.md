The Last Breath Changelog V1.0.0 BETA

1) Stamina Management System:
- Introduced a new stamina management system that requires players to monitor their stamina levels during gameplay.

a) Heavy Attacks:
Unchanged.

b) Light Attacks:
- Light attacks cost a percentage of heavy attack stamina. This will allow it to scale with player stats and provide a more balanced combat experience.
- The percentage can be tweaked, default is set to 30%.
- Can be turned off entirely.

c) Jumping:
- Jumping costs a flat amount of stamina.
- The amount can be tweaked, Default is set to 10 Points.
- Can be Turned off entirely.

d) Bow Usage:
- drawing a bow now drains stamina over time.
- The drain rate can be tweaked, Default is set to 3 Points per second.
- bow drawing is cancelled at 0 stamina.
- Can be turned off entirely.

- Releasing the bow costs 10 flat stamina points.
- The amount can be tweaked.
- Can be turned off entirely.
- Works with Bow rapid combo v3.

e) Blocking:
- Blocking now drains stamina over time.
- The drain rate can be tweaked, Default is set to 5 Points per second.
- Blocking is cancelled at 0 stamina.
- Can be turned off entirely.

f) Exhaustion Debuff:
- When stamina reaches zero, players will receive an exhaustion debuff.
- Attack damage is lowered by 25%. this can be tweaked.
- movement speed is lowered by 20%. this can be tweaked.
- damage recieved is increased by 15%. this can be tweaked.
- Can be turned off entirely.

g) Stamina Loss on hit:
- gradual, stamina loss is equal to (100-n)%*15, where n = armor skill level. 15 is configurable, can be turned off entirely

2) Casting Slow Debuff:
- any sort of casting (spells, staves, etc.) will apply a slow debuff to the caster.
- The slow percentage can be tweaked, Default is set to 20%.
- scales based on the skill level of the spell type being cast.
- Heavier debuff for dual casting.
- Can be turned off entirely.


TO DO:
- Implement Stamina Exaushtion debuff effects. -> Lowered movement speed, attack speed, etc. below a certain stamina threshold.
- Implement Stamina loss on blocking a hit . -> Blocking an attack will drain a flat amount of stamina.
- Implement a timed block system. -> Successful timed blocks will reduce or negate stamina loss.
- Add stamina Drain for high carry weight.


Poise System: Still being studied.

