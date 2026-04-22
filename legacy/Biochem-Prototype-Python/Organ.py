"""
A simple structure for prototyping
"""
from utilities import *

class Organ:
    def __init__(self, type, owner):
        # Should probably have the owner referenced by ID to save space, then have a get_owner_by_id() to get the actual organism
        self._genes = []
        self._id = generate_id()
        self._owner = owner
        self._health = 1
        self._parameters = []

    def set_dna_head(self, node):
        self._dna_head = node

    def get_dna_hear(self):
        return self._dna_head
        
    def get_genome(self):
        if self._dna_head is not None:
            return self._dna_head.get_entire_genome()
        return None
        
    def get_id(self):
        return self._id

class InternalOrgan(Organ):
    """
    Internal organs focus on managing bloodstream and chemicals
    """
    def add_gene(self, gene):
        self._genes.append(gene)

    def set_def_health(self, val=1):
        """
        Sets the default health
        """
        self._health = val
        self._health_receptors = []
        self._parameters.append(('health', self.health_adjust))
    
    def set_act_rate(self, act_rate):
        """
        Determines how often this organ is activated
        """
        self._act_rate = act_rate
        self._act_rate_receptors = []
        self._parameters.append(('activation rate', self.act_rate_adjust))

    def set_reaction_rate(self, rate):
        self._reaction_rate = rate
        self._reaction_rate_receptors = []
        self._parameters.append(('reaction rate', self.reaction_rate_adjust))

    def get_genes(self):
        return self._genes
        
    def debug_set_health(self, val):
        self._health = max(min(val,1),0)
        
    def get_param_numbers(self):
        return len(self._parameters)

    def get_param_at_index(self, index):
        return self._parameters[index]

    def read_health_from_gene(self, output):
        """
        This is passed to the gene(hopefully space efficient) and it should add the output to a queue, which is then averaged
        """
        self._health_receptors.append(output)

    def read_act_rate_from_gene(self, output):
        self._act_rate_receptors.append(output)

    def read_reaction_rate_from_gene(self, output):
        self._reaction_rate_receptors.append(output)

    def clear_receptors(self):
        self._health_receptors = []
        self._act_rate_receptors = []
        self._reaction_rate_receptors = []
    
    def get_parameter(self, param):
        if param == 'health':
            return self._health
        if param == 'activation rate':
            return self._act_rate
        if param == 'reaction rate':
            return self._reaction_rate
            
    def release_chemical(self, chemical, amount):
        self._owner.add_chemical(chemical, amount)

    def consume_chemical(self, chemical, amount):
        self._owner.rem_chemical(chemical, amount)

    def health_adjust(self, value):
        self._health_receptors.append(value)

    def act_rate_adjust(self, value):
        self._health_receptors.append(value)

    def reaction_rate_adjust(self, value):
        self._health_receptors.append(value)
    
    def get_health(self):
        return self._health

    def get_act_rate(self):
        return self._act_rate

    def get_reaction_rate(self):
        return self._reaction_rate

    def get_concentration(self, chemical):
        return self._owner.get_concentration(chemical)

    def get_chem_quant(self, chemical):
        return self._owner.get_chemical(chemical)
    
    def get_param_adjust(self, gene):
        return gene.adjust_parameter()
    
    def activate_organ(self):
        for gene in self._genes:
            self.activate_gene(gene)
        self.update_params()
    
    def activate_gene(self, gene):
        type = gene.get_type()
        if type == 'receptor':
            gene.adjust_parameter()
        elif type == 'emitter':
            gene.release_chemical()
        elif type == 'reaction':
            if gene.check_requirements():
                gene.react()

    def update_params(self):
        self._reaction_rate = sum(self._reaction_rate_receptors) / max(len(self._reaction_rate_receptors),1)
        self._act_rate = sum(self._act_rate_receptors) / max(len(self._act_rate_receptors), 1)
        average_health_change = sum(self._health_receptors) / max(len(self._health_receptors), 1)
        self._health = health_decay(self._health, average_health_change)
        
    def describe(self):
        """
        Provide a readout on all aspects of the organ, including parameters and genes
        """
        s1 = f"Organ {self._id}:\n"
        s2 = f"\t This organ has Health: {self._health}, Activation Rate: {self._act_rate}, Reaction Rate: {self._reaction_rate}\n"
        s3 = f"\t This organ has {len(self._genes)} genes:\n"
        print(s1, s2, s3)
        for gene in self._genes:
            gene.describe()


    def status(self):
        s1 = f"Organ{self._id}\n"
        s2 = f"Health: {self._health}\n"
        s3 = f"Activation Rate: {self._act_rate}\n"
        s4 = f"Reaction Rate: {self._reaction_rate}\n"
        print(s1, s2, s3, s4)
