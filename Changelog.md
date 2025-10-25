The Last Breath Changelog V0.0.1 BETA

1) Stamina Management System:
- Introduced a new stamina management system that requires players to monitor their stamina levels during gameplay.

a) Heavy Attacks:
Unchanged.

b) Light Attacks:
- Light attacks cost a percentage of heavy attack stamina. This will allow it to scale with player stats and provide a more balanced combat experience.
- The percentage can be tweaked, default is set to 35%.
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
- When stamina reaches a threshold, players will receive an exhaustion debuff. The threshold is configurable, Default is set to 20 Points.
- Attack damage is lowered by 25%. this can be tweaked.
- movement speed is lowered by 30%. this can be tweaked.
- Can be turned off entirely.

g) Stamina Loss on hit:
- gradual, stamina loss is equal to (100-n)%*15, where n = armor skill level. 15 is configurable, can be turned off entirely

2) Timed Blocking:
- Tracks when the block key is pressed, after a very slight animation delay (configurable), the timed block window begins, blocking an attack in this window can have different effects depending on the ini configuration, currently, hitting a successfull timed block:
- Recovers 20 stamina and negates the blocking damage by 50%.

- Subsequent timed blocks become progressively stronger if hit within a window of time, this window is seperate from the actual timed block window, and it increases with eache successful timed block.

- every timed block in the sequence staggers the enemy harder, on the 5th timed block (perfect parry) the enemy is completely staggered, and an elden counter can be initiated (by pressing the power attack button, i didnt change how elden counter works only it's conditions)

- every timed block in the sequence becomes progressively harder to land by narrowing the window.

- Timed block is entriely locked behind a 45 block skill requirement.


Everything said so far is configurable, and can be toggled on/off
