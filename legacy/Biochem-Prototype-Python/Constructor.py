# handles reading a genome
from Body import *
from Organ import *
from BioChemGene import *
from Chemicals import *
from Genome import *
from utilities import *

GENE_READ_LENGTH = 4
GENE_TYPES = 3
GENE_OPCODES = [0b11010]
GENE_LOWER_LIMIT = 100
GENE_UPPER_LIMIT = 120
ORGAN_OPCODES = [0b11111]
ORGAN_LOWER_LIMIT = 200
ORGAN_UPPER_LIMIT = 204 
NORMAL_READ_LENGTH = 8 # used to be 5

class Decoder:
    """
    A second decoder for when the genome is stored as linked list
    """
    def __init__(self):
        """
        Initialize to begin reading a dna strand
        """
        self._genome = None
        self._current_pos = 1
        self._current_organism = Body()
        self._current_organ = None
        self._current_gene = None
        self._current_read = b''
        self._current_node = Node()
        # Create a map for how many bits to read to assemble a function
        self._bits_for_funcs = dict(zip(func_names, bits_needed))

    def set_genome(self, genome):
        """
        Sets the active genome. Genome passed should be a byte string of the genome, ideally obtained from the parent.genome_head object
        """
        self._genome = b'1' + genome # prepend a 1 to prevent leading zero discrepensies

        
    def finish_organism(self):
        """
        On reading the whole strand, the organism will be finished, and the decoder will be reset
        """
        if self._current_organ is not None:
            if self._current_gene is not None:
                self._current_node.set_noncoding(self._current_read)
                self._current_gene.set_dna_head(self._current_node)
            else:
                self._current_organ.set_dna_head(self._current_node)
            self._current_organism.add_organ(self._current_organ)
        creature = self._current_organism
        self._current_organism = Body()
        self._genome = None
        self._current_pos = 1
        self._current_organ = None
        self._current_gene = None
        self._current_node = Node()
        return creature
    
    def read_at_pos(self, pos=None, length = NORMAL_READ_LENGTH):
        """
        Reads a section of Binary DNA, at a starting position and with a length
        """
        if pos == None:
            pos = self._current_pos
        if length == 0:
            return b''
        if pos >= len(self._genome):
            self._current_pos = len(self._genome)
            return b''
        if pos + length >= len(self._genome):
            self._current_pos = len(self._genome)
            return b''
            
        val = self._genome[pos:(pos+length)]
        if pos == self._current_pos:
            self._current_pos += length
        return val

    def read_genome(self):
        """
        Continually reads the genome, constructing an organism as it goes.
        """
        while self._current_pos < (len(self._genome)-1)-100:
            read_val = self.read_at_pos()  # Returns a byte string
            # If the gene start code was encountered begin constructing a gene, unless no organ exists to house it
            if GENE_LOWER_LIMIT <= int(read_val,2) <= GENE_UPPER_LIMIT and self._current_organ is not None:
                self.start_new_node('gene') # Finish the previously read node, begin a new one
                self._current_node.set_start(read_val) # Assign the start value
                self.read_gene_data() # Start reading it

            # If the organ start code was encountered, begin constructing an organ
            elif ORGAN_LOWER_LIMIT <= int(read_val,2) <= ORGAN_UPPER_LIMIT:
                self.start_new_node('organ')
                self._current_node.set_start(read_val)
                self.read_organ_data()
                self._current_gene = None # resets the active gene so that we can use this as aflag as well
            else:
                self._current_read += read_val
        if self._current_pos < (len(self._genome)):
            self._current_read += self._genome[self._current_pos:]
        # Finalize the organism and return it when complete.
        final = self.finish_organism()
        return final

    def read_organ_data(self):
        """
        Reads organ data (right now, only has 3 parameters: health, activation, and reaction), constructs the organ.
        """
        # Assign the current organism the previously active organ
        if self._current_organ is not None:
            self._current_organism.add_organ(self._current_organ)
            
        # Create a new organ and get the parameters.
        self._current_organ = InternalOrgan('internal', self._current_organism)
        self._current_organ.set_def_health()
        val = self.read_at_pos(length=5)
        read = val
        self._current_organ.set_reaction_rate(int(val,2)/32)
        val = self.read_at_pos(length=5)
        read += val
        self._current_organ.set_act_rate(int(val,2)/32)
        self._current_node.set_params(read)
        return

    def read_gene_data(self):
        """
        Constructs a gene (currently only 2 types (emitter, and receptor))) and gets parameters for it.
        """
        # Find out what type of gene it is
        type = self.read_at_pos(length = GENE_READ_LENGTH)
        read = type
        type = int(type,2) % GENE_TYPES
        if type == 2:
            # It's gonna be easier to separate all the logic into separate functions instead
            self._current_gene = Reaction(self._current_organ, 'reaction')
            self.read_reaction_data()
            return
        elif type == 1:
            self._current_gene = Emitter(self._current_organ, 'emitter')
            rate = self.read_at_pos(length=5)
            read += rate
            self._current_gene.set_output_rate(int(rate,2))
        elif type == 0:
            self._current_gene = Receptor(self._current_organ, 'receptor')
        
        # Now parse the function this gene uses. Each function needs different parameters
        func = self.read_at_pos(length = 3)
        read += func
        func = int(func,2)
        func_name = func_names[func]
        func_read_lengths = bits_needed[func]
        params = []
        # If the function needs parameters, then read each one as needed
        for param in func_read_lengths:
            val = self.read_at_pos(length = param)
            read += val
            params.append(int(val,2))
        if params:
            function = functions[func](*params)
        else:
            function = functions[func]()
        self._current_gene.set_activation(func_name, function)

        # Now handle the other parameters of the gene
        val = self.read_at_pos(length = 4)
        read += val
        val = int(val,2) % self._current_organ.get_param_numbers()
        p=  self._current_organ._parameters[val]
        self._current_gene.set_parameter(p[0], p[1])
        val = self.read_at_pos(length = 4)
        read += val
        val = int(val,2)
        self._current_gene.set_chemical(val)
        self._current_organ.add_gene(self._current_gene)
        self._current_node.set_params(read)

    def read_reaction_data(self):
        read = b'0010'
        if self._current_pos > len(self._genome)-50:
            return
        left = self.read_at_pos(length = 4)
        right = self.read_at_pos(length = 4)
        read += left + right
        self._current_gene.set_num_of_chems_left(int(left,2))
        self._current_gene.set_num_of_chems_right(int(right,2))
        chems = []
        for i in range((int(left,2) % 2) + 1 + (int(right,2) % 3)):
            val = self.read_at_pos(length = 6)
            read += val
            chem = self.read_at_pos(length = 4)
            read += chem
            chems.append((int(val,2),int(chem,2)))
        self._current_gene.set_chems_and_coefficients(chems)
        self._current_organ.add_gene(self._current_gene)
        self._current_node.set_params(read)
        
    def start_new_node(self, type):
        self._current_node.set_noncoding(self._current_read)
        self._current_node.next = Node()
        self._current_read = b''
        
        if self._current_organ is None:
            # If organ opcode was encountered but no organ started yet, this is the first node
            self._current_organism.set_dna_head(self._current_node)

        # If organ opcode was encountered and no gene was constructed in the last one, then set this node to the previous organ
        elif type == 'organ':
            if self._current_gene is None:
                self._current_organ.set_dna_head(self._current_node)
            else:
                self._current_gene.set_dna_head(self._current_node)
                
        else:
            if self._current_gene is not None:
                self._current_gene.set_dna_head(self._current_node)
        
        self._current_node = self._current_node.next
