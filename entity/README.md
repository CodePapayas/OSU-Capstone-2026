# Entity description
Each simulation is composed of entities navigating about the environment. The Entity class is a container which links up the individuals biology, brain, and location, into an easy to use API. This would allow the simulation to only query the Entity class when information is needed.

## What I'm envisioning
I'm imagining that the simulation is handled by a simulation manager. During each tick, the manager iterates through each entity and performs the following steps:

* Gets the brains decision based on input
* Executes that action (do nothing, move forward, etc.)
* Updates the location of the entity
* Updates the entity's biology
* Checks for entity death

Rather than having the manager interact with each of these components, it would be easier for the manager to only interact with the container for those components. Requests are feed down to individual components and returns are propogated upwards. This prevents some issues that arise from scope.

For movement, I figure it would be easier to have the actual function defined in the simulation, rather than the entity. The entity only feeds back what it wants to do, and then the simulation executes it and updates the location as needed. This means that the entity class doesn't need to know specifics about how movement works, or the simulation constraints.

## Attributes

* ID
  * This represents a unique id associated with the entity. Can be generated however we want. The simulation can keep a hashmap of IDs to objects for efficient lookup
* Biology
  * This is the container for the organs and chemicals. Should be either a direct reference to the object or the ID of the object. I believe python will only store a pointer to the object, so it's space efficient regardless
* Brain
  * The decision center. Again, either a direct reference or ID, pending what people think
* Location
  * Where the entity is in the simulation. Depending on the direction we take, this could either be a reference to the cell object of the simulation or the actual coordinates
 
## Methods

## Example of single tick process
* The simulation iterates through each ID saved in its hashmap
* For each entity, it calls EntityCycle() (or whatever we decide to call it)
* This first calls the Entity to make a decision:
  * The entity passes the current location and biology to the brain for input
  * The output is feed back up to the simulation manager
* The simulation manager executes this action and depending on action requested:
  * Updates the location
  * Adds chemicals
  * Removes Energy
  * Adds Energy
* Then, the simulation asks the Entity to update all of its biology:
  * The entity requests its biology to activate all of its organs
  * The biology activates each organ/gene
  * The biology updates its chemicals and energy accordingly
* The simulation then asks if the entity should be dead:
  * The entity asks the biology to describe its current health and energy
  * The entity informs the simulation whether or not it is dead
  * The simulation removes this entity from the hashmap if needed.
