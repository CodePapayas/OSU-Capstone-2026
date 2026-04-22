class Entity:
    """
    A class that represents an individual in the simulation. Coordinates function between biology and decision centers, as well as with the actual simulation
    """
    def __init__(self):
        #self._id = GenerateID() # Replace with desired Id generation tool
        self._biology   = None
        self._brain     = None
        self._location  = None

    # General setters and getters
    
    def set_biology(self, biology):
        """
        Sets the organisms biology container. Houses the genetic code for the body as well as the organs and chemicals
        """
        ################################
        # Uncomment this section once certain of method. I imagine the id of the associated biology, or the actual object
        #if not isinstance(biology, str):
        #    return False
        ################################
        self._biology = biology

    def set_brain(self, brain):
        """
        Sets the organisms decision center.
        """
        ################################
        # Uncomment this section once certain of method. I imagine the id of the associated brain obj
        #if not isinstance(biology, str):
        #    return False
        ################################
        self._brain = brain

    def set_location(self, location):
        """
        Sets the organisms present location
        """
        ################################
        # Uncomment this section once certain of method. I imagine the id of the associated cell in the sim
        #if not isinstance(biology, str):
        #    return False
        ################################
        self._location = location

    def get_id(self):
        """
        Returns the entities id
        """
        return self._id
        
    def get_biology(self):
        """
        Returns the organisms biology.
        """
        return self._biology
    
    def get_brain(self):
        """
        Returns the organisms brain.
        """
        return self._brain

    def get_location(self):
        """
        Returns the organisms location
        """
        return self._location

    # Brain and biology related functions

    def brain_decide(self,inputs):
        """
        Calls upon the brain to make a decision as to what to do.
        """
        inputs.append(self.biology_get_health())
        inputs.append(self.biology_get_energy())
        inputs.append(self.biology_get_water())
        return self._brain.decide(inputs) # need to verify method and needed inputs

    def biology_add_chemical(self, chem, amount):
        """
        Adds a quantity of a given chemical to the biology
        :param chem: The id of the chemical
        :param amount: the amount to be added
        """
        self._biology.add_chemical(chem, amount)

    def biology_rem_chemical(self, chem, amount):
        """
        Removes a quantity of a given chemical from the biology.
        :param chem: The id of the chemical
        :param amount: the amount to be removed
        """
        self._biology.add_chem(chem, amount * -1)

    def biology_eat(self, amount):
        """
        Passes an amount of energy to be consumed by the creature
        :param amount: the amount to be modified
        """
        print(f"Creature consumed {amount} energy for net "
              f"{self._biology.eat_energy(amount)} energy")

    def biology_add_energy(self, amount):
        """
        Adds a quantity of energy to the biology
        :param amount: the amount to be added
        """
        self._biology.add_energy(amount)

    def biology_rem_energy(self, amount):
        """
        Removes a quantity of energy from the biology.
        :param amount: the amount to be removed
        """
        self._biology.add_energy(amount * -1)

    def biology_drink(self, amount):
        """
        Passes an amount of energy to be consumed by the creature
        :param amount: the amount to be modified
        """
        print(f"Creature consumed {amount} water for net "
              f"{self._biology.drink_water(amount)} water")

    def biology_add_water(self, amount):
        """
        Adds a quantity of water to the biology
        :param amount: the amount to be added
        """
        self._biology.add_water(amount)

    def biology_rem_water(self, amount):
        """
        Removes a quantity of water from the biology.
        :param amount: the amount to be removed
        """
        self._biology.add_water(amount * -1)

    def biology_add_health(self, amount):
        """
        Adds a quantity of health to the biology
        :param amount: the amount to be added
        """
        self._biology.add_health(amount)

    def biology_rem_health(self, amount):
        """
        Removes a quantity of health from the biology.
        :param amount: the amount to be removed
        """
        self._biology.add_health(amount * -1)

    def biology_check_death(self):
        """
        Checks if the individual should have died.
        :return: True if the organism should die, false otherwise
        """
        if self._biology.check_death():
            return True
        return False

    def update_biology(self):
        """
        Requests the biology to update itself for a tick.
        """
        self._biology.update()

    def biology_get_metrics(self):
        h = self._biology.get_health()
        e = self._biology.get_energy()
        w = self._biology.get_water()
        self._biology.print_vals()
        return {"Health":h, "Energy":e, "Water":w}
    
    def biology_get_genetics(self):
        """
        Returns the genetic values of the organism
        """
        self._biology.display_genetic_vals()
        return self._biology.get_genetic_vals()
    
    def biology_get_genetic_value(self, gene):
        """
        Polls the biology for a particular genetic value
        :return: The value, returns None if value not found
        """
        try:
            val = self._biology.get_efficiency(gene)
        except KeyError:
            print(f"Key error! Genetics had no key {gene}")
            return None
        print(f"{gene}: {val}")
        return val
    
    def biology_movement(self, terrain):
        """
        Tells the biology to drain energy based on terrain type
        """
        try:
            edrain = self._biology.movement_energy_drain(terrain)
        except KeyError:
            edrain = None
        try:
            wdrain = self._biology.movement_water_drain(terrain)
        except:
            wdrain = None
        print(f"Energy drain for {terrain}: {edrain}"
              f"Water drain for {terrain}: {wdrain}")
        return (edrain, wdrain)

    # Alias methods
    def __getattr__(self, name):
        """
        Alias get methods as attributes of the object. Prevent reassignment of private attributes.
        For example, calling Entity.id instead of Entity.get_id()
        """
        if name == "id" or name == "ID":
            return self.get_id()
        if name == "brain" or name == "decision_center":
            return self.get_brain()
        if name == "body" or name == "biology":
            return self.get_biology()
        if name == 'dead' or name == "death":
            return self.biology_check_death()
        if name == 'cell' or name == "location":
            return self.get_location()
