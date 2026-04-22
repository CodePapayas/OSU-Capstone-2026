import random
from . biology_constants import *

class Biology:

    def __init__(self, debug=False):
        """
        Initialize a creatures biology with random values
        :param debug: True to have default genetic values
        """
        self._energy = 1
        self._health = 1
        self._water = 1
        self._genetic_values = GENETIC_VALUES
        if not debug:
            self.set_random_attributes()

    def set_random_attributes(self):
        """
        Sets the genetic values to random values between 0 and 1
        """
        for key in list(self._genetic_values.keys()):
            self._genetic_values[key] = random.random() ** 2

    def display_genetic_vals(self):
        """
        Outputs all of the genetic values for the biology
        """
        for k, v in self._genetic_values.items():
            print(k,": ",v)

    def get_genetic_vals(self):
        """
        Returns the genetic vlues of the organism
        :return: A dictionary containing the keys and values
        """
        return self._genetic_values

    def add_health(self, val):
        """
        Adds an amount of health to the biology
        :param val: the amount to add
        """
        self._health = min(self._health + val, 1.0)

    def add_energy(self, val):
        """
        Adds an amount of energy to the biology
        :param val: Float, the amount to add
        """
        self._energy = min(self._energy + val, 1.0)

    def add_water(self, val):
        """
        Adds an amount of water to the biology
        :param val: Float, The amount to add
        """
        self._water = min(self._health + val, 1.0)

    def get_health(self):
        """
        Returns the current health of the organism
        """
        return self._health

    def get_energy(self):
        """
        Returns the current energy of the organism
        :return: Float, The amount of energy the creature currently has
        """
        return self._energy

    def get_water(self):
        """
        Returns the current water level of the organism
        :return: Float, The amount of water the creature currently has
        """
        return self._water

    def get_efficiencies(self):
        """
        Returns all of the creatures genetic values (efficiencies)
        :return: Dictionary, contains all efficiency values
        """
        return self._genetic_values

    def get_efficiency(self, efficiency):
        """
        Gets a particular efficiency value from the creature
        :param efficiency: The value to query
        :return: Float, that parameters value
        """
        try:
            return self._genetic_values[efficiency]
        except:
            raise KeyError

    def set_efficiencies(self, vals):
        """
        Assiigns the genetic values passed to the organism
        """
        # Should add some sanitization here
        self._genetic_values = vals

    def set_efficiency(self, type, value):
        """
        Assigns a new value to a particular genetic trait
        """
        if not 0<=value<=1:
            raise ValueError
        # Might need to change this actually, since its assignment
        # Maybe just check membership in keys for gen vals
        try:
            self._genetic_values[type] = value
        except:
            raise KeyError

    def eat_energy(self, quantity):
        """
        Adjusts the energy stores obtained from eating based on the creatures
        various efficiencies.
        :param quantity: The original amount of energy
        :return: Float, the net amount
        """
        amount = quantity * self._genetic_values["Energy Efficiency"]
        amount *= (1- self._genetic_values['Mass'])**.5
        self.add_energy(amount)
        return amount

    def drink_water(self, quantity):
        """
        Adjusts the water reserves a creature obtains from drinking based on
        efficiency
        :param quantity: The original amount of water
        :return: Float, the net adjusted amount
        """
        amount = quantity * self._genetic_values["Water Efficiency"]
        self.add_water(amount)
        return amount

    def movement_energy_drain(self, terrain_type):
        """
        Drains energy based on the type of terrain the creature moved through
        :param terrain_type: The particular efficiency to use
        :return: The amount of energy that movement drained
        """
        try:
            efficiency = 1-self._genetic_values[terrain_type]
        except KeyError:
            efficiency = 1-self._genetic_values['Traversal Efficiency 1']
        amount = max(
            efficiency * TERRAIN_ENERGY_COEFFICIENT * self._genetic_values['Mass'],
            .01)
        self.add_energy(amount * -1)
        return amount

    def movement_water_drain(self, terrain_type):
        """
        Drains water based on the type of terrain navifated
        :param terrain_type: The particular efficiency to use
        :return: the net water drainage
        """
        try:
            efficiency = 1-self._genetic_values[terrain_type]
        except KeyError:
            efficiency = 1-self._genetic_values['Traversal Efficiency 1']
        water_efficiency = 1-self._genetic_values["Water Efficiency"]
        amount = max(water_efficiency * efficiency *
                      TERRAIN_WATER_COEFFICIENT, .01)
        self.add_water(amount * -1)
        return amount

    def add_chemical(self, chemical_type, quantity):
        """
        Adjusts health based on the chemical type passed
        :param chemical_type: Which chemical the creature is absorbing
        :param quantity: The amount the creature is absorbing
        :return: The amount that health was edjusted
        """
        try:
            efficiency = self._genetic_values[chemical_type] - .5
        except:
            raise KeyError
        amount = efficiency * quantity * HEALTH_COEFFICIENT
        self.add_health(amount)
        return amount

    def tick_energy_drain(self):
        """
        Determines how much energy the creature loses every tick
        :return: The net amount drained
        """
        total = 0
        for k, v in self._genetic_values.items():
            if k == 'Mass':
                continue
            else:
                total += v
        total = total ** .5 / len(list(self._genetic_values.keys()))
        total = max(total * (1-(self._genetic_values["Mass"]**2)),.02)
        self.add_energy(total * -1 * ENERGY_DRAIN_COEFFICIENT)
        return total

    def tick_health_drain(self):
        """
        Determines how much health the creature loses each tick
        :return: The amount of health drained
        """
        if self._energy < 1 - self._genetic_values["Mass"]:
            difference = self._genetic_values["Mass"] - self._energy
            drain = difference ** 2
            self.add_health(drain * -1)
            return drain
        return 0

    def update(self):
        self._energy = max(self._energy, 0)
        self._water = max(self._water, 0)
        print('Tick energy loss: ', self.tick_energy_drain())
        print('Tick Health loss', self.tick_health_drain())

    def print_vals(self):
        print(f"Current Health: {self._health}")
        print(f"Current Energy: {self._energy}")
        print(f"Current Water: {self._water}")
        
    def check_death(self):
        if self._health <= 0.0:
            return True
        return False

