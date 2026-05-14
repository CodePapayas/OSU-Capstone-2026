"""
Gives rules for genes that govern biochemistry properties. These produce constructs which are grouped together into an 'organ', each gene governs a potential reaction, monitors for a specific chemical, or affects the global attributes of the organ.
"""
import utilities

REACTION_MAX = 4

class BioChemGene:
    """
    General Biochemistry gene
    """
    def __init__(self, organ, type):
        self._organ = organ
        self._id = utilities.generate_id()
        self._type = type
        self._dna_head = None
    
    def set_activation(self, name, function):
        """
        Assigns an activation function
        :param name: The name of the fucntion
        :param function: The actual function
        """
        self._activation_function = function
        self._func_name = name

    def get_type(self):
        """
        Returns the type of gene
        """
        return self._type

    def set_dna_head(self, node):
        """
        Sets the node of the genome linked list structure assigned to this strucutre
        """
        self._dna_head = node

    def get_dna_head(self):
        """
        Gets the node assigned to this structure
        """
        return self._dna_head

    def get_genome(self):
        """
        Gets the full structure of the genome recursively from this node
        """
        if self._dna_head is not None:
            return self._dna_head.get_entire_genome()
        return None
        
class Receptor(BioChemGene):
    """
    The gene for a receptor
    """
            
    def set_parameter(self, name, parameter):
        """
        Sets what this gene can adjust in the organ. Presently needs to be passed a function from the organ, such as organ.get_rate() or organ.get_health(), these are then averaged over all receptors in organ
        """
        self._param_name = name
        self._parameter = parameter

    def set_chemical(self, chemical):
        """
        Sets which chemical this gene detects
        """
        self._chemical = chemical

    def set_activation_function(self, name, function):
        """
        Sets the function that controls output, should already be parameterized if needed before assignment. Parameters are function specific and read from genome.
        """
        self._activation_function = function
        self._func_name = name

    def read_input(self):
        """
        Reads the current concentration of a chemical from the body
        """
        return self._organ.get_concentration(self._chemical)

    def get_output(self):
        return self._activation_function(self.read_input())
        
    def adjust_parameter(self):
        """
        outputs signal to the parameter it should effect
        """
        self._parameter(self.get_output())

    def describe(self):
        """
        Outputs all atttributes of the gene in an easy to read manner
        """
        s1 = f"\t\tGene {self._id}:\n"
        s2 = f"\t\t\t This gene monitors for chemical {self._chemical}. Based on the concentration it adjusts {self._param_name}.\n"
        s3 = f"\t\t\t\t Organ parameter: {self._param_name}\n"
        s4 = f"\t\t\t\t Activation function: f{self._func_name}\n"
        s5 = f"\t\t\t\t Chemical read: {self._chemical}\n"
        s6 = f"\t\t\t\t Example: Chemical at .1 produces {self._activation_function(.1)}, Chemical at .5 produces {self._activation_function(.5)},  Chemical at .9 produces {self._activation_function(.9)}\n"
        print(s1,s2,s3,s4,s5,s6)

class Emitter(BioChemGene):
    """
    Reads a parameter from the host organ, and outputs a specific chemical with a strength modified by organ health
    """

    def set_parameter(self, name, parameter):
        """
        Sets the organs attribute to monitor
        """
        self._param_name = name
        self._parameter = parameter

    def set_chemical(self, chemical):
        """
        Sets the chemical that this releases
        """
        self._chemical = chemical

    def set_output_rate(self, rate):
        """
        Determines how much is released by default. I'm thinking a number between 0-10, mutations can increase? Outputs units, rather than concentrations
        """
        self._rate = (rate+1)/5

    def read_param(self):
        """
        Gets the parameter of the organ this is tied to
        """
        return self._organ.get_parameter(self._param_name)

    def get_output_amt(self):
        """
        determines the amount of chemical to release
        """
        return self._activation_function(self.read_param()) * self._rate

    def release_chemical(self):
        """
        Releases the chemical to the body
        """
        self._organ.release_chemical(self._chemical, self.get_output_amt())

    def describe(self):
        """
        Outputs all atttributes of the gene in an easy to read manner
        """
        s1 = f"\t\tGene {self._id}:\n"
        s2 = f"\t\t\t This gene monitors the organ's {self._param_name}. Based on the rate it releases chemical {self._chemical}.\n"
        s3 = f"\t\t\t\t Organ parameter: {self._param_name}\n"
        s4 = f"\t\t\t\t Activation function: f{self._func_name}\n"
        s5 = f"\t\t\t\t Chemical output: {self._chemical}\n"
        s7 = f"\t\t\t\t Output rate: {self._rate}\n"
        s6 = f"\t\t\t\t Example: {self._param_name} at .1 produces {self._activation_function(.1)*self._rate} units of {self._chemical}, {self._param_name} at .5 produces {self._activation_function(.5)*self._rate} units of {self._chemical},  {self._param_name} at .9 produces {self._activation_function(.9)*self._rate} units of {self._chemical}\n"
        print(s1,s2,s3,s4,s5,s7,s6)

class Reaction(BioChemGene):
    """
    Converts chemicals of one type to another, symbol equation suchas A+B=2C+D
    """
    def set_num_of_chems_left(self, num):
        """
        Determines how many chemicals on the left of the equation, 1 or 2
        """
        self._num_of_chems_left = (num % 2)+1

    def set_num_of_chems_right(self, num):
        self._num_of_chems_right = num % 3

    def set_chems_and_coefficients(self, list):
        """
        Save the chemicals in the format [(1.43, 13),(2.5, 6)]
        """
        self._chems = list

    def get_equation_params(self):
        """
        Gets the number of chemicals on either side of the reaction equation
        """
        return (self._num_of_chems_left, self._num_of_chems_right)
        
    def check_for_requirements(self):
        """
        Test whether the host has enough of a chemical for a reaction
        :return: True if the reaction can happen
        """
        for i in range(self._num_of_chems_left):
            chem = self._chems[i]
            q = self._organ.get_chem_quant(chem[1])
            print(chem)
            if q < chem[0]*(REACTION_MAX)/64:
                return False
        return True

    def react(self):
        """
        Consumes chemicals on left of equation and produces chems on right.
        """
        for i in range(len(self._chems)):
            chem = self._chems[i]
            q = chem[0]*(REACTION_MAX)/64
            if i < self._num_of_chems_left:
                self._organ.consume_chemical(chem[1], q)
            else:
                self._organ.release_chemical(chem[1], q)

    def describe(self):
        """
        Outputs all atttributes of the gene in an easy to read manner
        """
        eq = ''
        for i in range(len(self._chems)):
            chem = self._chems[i]
            if i < self._num_of_chems_left:
                if i == 1:
                    eq += " + "
                s = f"{chem[0]*REACTION_MAX/64}(Chemical {chem[1]})"
                eq += s
            else:
                if i == self._num_of_chems_left:
                    eq += " = "
                else:
                    eq += " + "
                s = f"{chem[0]*REACTION_MAX/64}(Chemical {chem[1]})"
                eq += s

        s1 = f"\t\tGene {self._id}:\n"
        s2 = f"\t\t\t This gene performs a chemical reaction.\n"
        s3 = f"\t\t\t\t Equation: {eq}\n"
        print(s1,s2,s3)
