# Biology 
The biology class handles the organisms genetics and survival metrics.

## Genetics
Currently, genetics are only implemented as 14 float values. These are:
* Energy Efficiency
    * affects how much energy is gained when consuming food
* Water Efficiency
    * Affects how much water is gained when drinking
* Mass
    * Affects how much energy can be stored, and how much is drained each tick
* Terrain Efficiencies (3)
    * Affects how much energy is consumed navigated 1 of 3 Terrain types
* Chemical Efficiencies (4)
    * Affects how much health is gained or lost when consuming a chemical resource
* Vision
    * Affects how much the organism can see

## Metrics

There are 3 key metrics:
* Health
    * How healthy the organism is. Reaching 0 is equal to death. Drains when food is low and by consuming harmful chemicals. Increases only by consuming helpful chems
* Energy
    * How much energy reserves organism has. Drains when moving and each tick.
* Water
    * How much water the organism has. Drains on movement. Reaching 0 causes health to drain (not yet implemented).

## How it comes together

All of these work together by assigning costs and benefits to different actions. Every tick, the simulation executes an action for the organism. When that action is taken, the biology will update the appropriate  metrics. Then the biology updates itself. First it sums all of its genetics, takes the square root of that and divides it by the current number of genetic features. Then it multiplies this by the inverse of its mass squared (1-mass*2). This gives us the amount of energy the creature loses each tick. 

This should produce interesting results, as a creature with very large values for its genetics is highly efficient but loses a lot of energy each tick.

## Health Drain

Health is lost every cycle if the current food reserves are less than the inverse of the mass. The amount depends on the difference betweeen these values, but grows as energy approaches 0.