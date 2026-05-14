"""
Handles possible methods for organism reproduction
"""
from Body import *
from Constructor import *
from utilities import *
from Genome import *
import random

MUTATION_RATE = .01
START_FLIP_DIVISOR = 20
PARAM_FLIP_DIVISOR = 4
NON_CODING_DIVISOR = 1

# Genome parsing - possible combine this with decoder, that would mean genome is read once. Maybe having the Genome parsed into a directed graph would be easier?

# PARTHENOGENIC METHODS
def flip_at(pos, strand):
    val = int(strand[pos])-48
    chg = val^1
    strand = strand[:pos]+chr(chg+48).encode('utf-8')+strand[pos+1:]
    return strand

def flip_segment(segment, mutation_rate=MUTATION_RATE, divisor=1):
    count = 0
    for i in range(len(segment)):
        odds = (1 -(mutation_rate/divisor)) ** len(segment)
        roll = random.random()
        while roll > odds:
            count += 1
            odds = (1 -(mutation_rate/divisor)) ** (len(segment)-count)
            roll = random.random()
        count = min(count, len(segment))
        choices = random.sample(range(len(segment)),count)
        for i in choices:
            segment = flip_at(i, segment)
        """
        for _ in range(len(count)):
            index = random.choice(range(len(segment)))
            segment = flip_at(index, segment)
        """
    return segment

def increment_frame(frame, val=1):
    """
    When given a binary string, will increment or decrement by the provided value
    """
    original_length = len(frame)
    if int(frame,2) == 0:
        mask = int(frame,2)+ 2**len(frame)
        mask-= 1
        return bytes(bin(mask),'utf-8')[2:]
    new_val = int(frame, 2) + val
    new_val = bytes(str(bin(new_val))[2:],'utf-8')
    new_length = len(new_val)
    while len(new_val) != original_length:
        if len(new_val) < original_length:
            new_val = b'0' + new_val
        else:
            new_val = new_val[1:]
    return new_val
            
def random_bit_flip(organism, mutation_rate = MUTATION_RATE):
    """
    performs random bit flipping across the genome with equal weighting (highly chaotic, probably). Assumes it is getting an int back.
    """
    genome = bin(organism.get_genome())
    for i in range(1, genome.bit_length()):
        if random.random() < mutation_rate:
            genome = bit_flip(genome, i)
    return genome

def random_bit_flip_string(organism, mutation_rate = MUTATION_RATE):
    """
    Performs random bit flipping across genome, but treats genome as string instead. More promising I think. Assumes byt string returned
    """
    genome = organism.get_genome()
    for i in range(len(genome)):
        if random.random() < mutation_rate:
            genome = flip_at(i, genome)
    return genome
    
def bit_flip_in_params(organism, mutation_rate = MUTATION_RATE):
    """
    performs random bit flipping only on structures (does not flip OpCodes
    Add testing to ensure that structure is preserved.
    """
    organ_string = b''
    for organ in organism.get_organs():
        node= organ.get_dna_head()
        params = node.get_params()
        for i in range(len(params)):
            if random.random() < mutation_rate:
                params = flip_at(i, params)
        organ_string += params
        organ_string = organ_string + node.get_start() + params + node.get_noncoding()
        for gene in organ.get_genes():
            node = gene.get_dna_head()
            params = node.get_params()
            for i in range(len(params)):
                if random.random() < mutation_rate:
                    params = flip_at(i, params)
            organ_string += node.get_start() + params + node.get_noncoding()
    genome = organism.get_dna_head().get_noncoding()+organ_string
    return genome
    
def bit_flip_weighted(organism, mutation_rate = MUTATION_RATE):
    """
    Performs bit flipped on genome, but more strongly weighted for non coding sections
    Instead of checking every bit, lets just roll against the whole string? If it has length of 20, then its 1- odds ^ 20
    """
    genome = b''
    node = organism.get_dna_head()
    while node.next:
        start = node.get_start()
        if start is None:
            start = b''
        params = node.get_params()
        if params is None:
            params = b''
        noncoding = node.get_noncoding()
        if noncoding is None:
            noncoding = b''
        start = flip_segment(start, mutation_rate, START_FLIP_DIVISOR)
        params = flip_segment(params, mutation_rate, PARAM_FLIP_DIVISOR)
        noncoding = flip_segment(noncoding, mutation_rate, NON_CODING_DIVISOR)
        
        genome += start+params+noncoding
        node = node.next
    return genome


def insert_to_preserve_order(organism, mutation_rate = MUTATION_RATE):
    """
    When a flip is performed that adds a structure that needs parameter values, random bits are inserted to preserve future reading frames
    # This might need to be handled by the decoder, perhaps by parsing the parents structure?
    """
    pass

def increment_decrement_frame(organism, mutation_rate = MUTATION_RATE):
    """
    Whenever a frame is read, that frame may be decremented or incremented.
    Perhaps test it with weighting as well
    """
    genome = b''
    node = organism.get_dna_head()
    while node.next:
        if start is None:
            start = b''
        params = node.get_params()
        if params is None:
            params = b''
        noncoding = node.get_noncoding()
        if noncoding is None:
            noncoding = b''
        if random.random() < mutation_rate:
            val = random.choice([1,-1])
            start = increment_frame(start, val) 
        if random.random() < mutation_rate:
            val = random.choice([1,-1])
            params = increment_frame(params, val) # Wait no, this needs to read frame by frame first.
        if random.random() < mutation_rate:
            val = random.choice([1,-1])
            noncoding = increment_frame(noncoding, val) # Wait no, this needs to read frame by frame first.
        genome += start + params + noncoding
        node = node.next
    return genome
    
# ANALOGOUS TO REALITY

def retrotransposition(organism, mutation_rate=MUTATION_RATE):
    """
    Copys a structure and pastes it elsewhere in the genome.
    """
    node = organism.get_dna_head().next # need to skip over the start
    new_node = None
    while node:
        if random.random() < mutation_rate:
            if new_node is None:
                new_node = Node()
                new_node.set_start(node.get_start())
                new_node.set_params(node.get_params())
                new_node.set_noncoding(node.get_noncoding())
            else:
                new_node.next = node.next
                non_coding = node.get_noncoding()
                place = random.choice(range(len(non_coding)))
                part_a = non_coding[:place]
                part_b = non_coding[place+1:]
                node.set_noncoding = part_a
                new_node.set_noncoding = new_node.get_noncoding() + part_b
                node.next = new_node
                new_node = None
                node = node.next
            if node.next is None and new_node:
                node = organism.get_dna_head().next # this probably isn't the best way to do it, but it works for now
            else:
                node = node.next
                
def deletion(organism, mutation_rate=MUTATION_RATE):
    """
    Deletes a structure from the genome, preserve reading frame
    """
    prev_node = organism.get_dna_head()
    node = prev_node.next
    while node:
        if random.random() < mutation_rate:
            prev_node.next = node.next
        else:
            prev_node = node
        node = node.next
    

def duplication(organism, mutation_rate=MUTATION_RATE):
    """
    Duplicates a gene or structure and places it adjacent to this structure
    """
    prev_node = organism.get_dna_head()
    node = prev_node.next
    while node:
        if random.random() < mutation_rate:
            new_node = Node()
            new_node.set_start(node.get_start())
            new_node.set_params(node.get_params())
            new_node.set_noncoding(node.get_noncoding())
            new_node.next = node.next
            node.next = new_node
        node = node.next
    
def point_deletion(organism, mutation_rate= MUTATION_RATE):
    """
    deletes a number of bits, preserve reading frame
    """
    pass
