"""
Creates a simple organism for prototyping purposes
"""

from utilities import *
import Chemicals
import random

class Body:
    """
    An organism, a container for organs and chemicals
    """
    def __init__(self, genome=None):
        """
        Setup a generic organism
        :param genome: A bit string genome, deprecated method of construction
        """
        self._id = generate_id()
        self._chems = {chem: 0 for chem in Chemicals.CHEMS}
        self._concentrations = {chem: 0 for chem in Chemicals.CHEMS}
        self._organs = []
        self._genome = genome
        self._dna_head = None
        
    def add_organ(self, organ):
        """
        Adds a new organ to the creature
        """
        self._organs.append(organ)

    def activate_organs(self):
        """
        Activates each organ based on the activation rate of that organ
        """
        for organ in self._organs:
            roll = random.random()
            if roll <= organ.get_act_rate():
                organ.activate_organ()
        self.calc_concentrations()

    def calc_concentrations(self):
        """
        Rechecks the concentrations of chemicals in the body
        """
        total = 0
        for key, val in self._chems.items():
            total += val
        if total == 0:
            total = 1
        self._concentrations = {chem: val/total for chem, val in self._chems.items()}

    def get_concentration(self, chemical):
        """
        Try to get the chemical concentration of a particular chemical
        :param chemical: The chemical to check
        :return: The concentration of that chemical, a float between 0-1
        """
        try:
            return self._concentrations[chemical]
        except:
            print("\n!!!!!! An error occured !!!! An invalid chemical was requested!\n")
            return 0

    def get_chemical(self,chemical):
        """
        Gets the number of units of a chemical in the body
        :param chemical: The chemical to check
        """
        return self._chems[chemical]

    def add_chemical(self, chemical, amount):
        """
        Adds an amount of a chemical to the body
        :param chemical: The checmicla
        :param amount: The amount to add
        """
        try:
            self._chems[chemical] += amount
        except:
            print("\n!!!!!! An error occured !!!! An invalid chemical was added to the body\n")
            return

    def rem_chemical(self, chemical, amount):
        try:
            self._chems[chemical] -= amount
            if self._chems[chemical] < 0:
                self._chems[chemical] = 0
        except:
            print("\n!!!!!! An error occured !!!! An invalid chemical was removed from the body\n")
            return

    def set_dna_head(self, node):
        self._dna_head = node
        
    def get_genome(self):
        if self._dna_head is None:
            return self._genome
        return self._dna_head.get_entire_genome()
        
    def get_organs(self):
        return self._organs

    def describe(self):
        print(f"Creature {self._id}:")
        for organ in self._organs:
            organ.describe()

    def status(self):
        print(f"Creature {self._id}:\n")
        for chemical in Chemicals.CHEMS:
            print(f"Chemical {chemical} -- units: {self._chems[chemical]}, concentrations: {self._concentrations[chemical]}\n")
        for organ in self._organs:
            organ.status()
