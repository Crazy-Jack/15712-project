# Arguments:
# -t tracefile name (trace files are put in the ./traces/ directory)
# -n number of commands

import argparse
import os
import random

def flip_coin():
    return random.choice([True,False])

# Globals
LOWVAL = 1
HIGHVAL = 10

# Create parser
parser = argparse.ArgumentParser()
parser.add_argument("-t", "--tracefile", action='store')
parser.add_argument("-n", "--num-commands", action='store', type=int)
parser.add_argument("-s", "--seed", action='store', type=int)
args = parser.parse_args()
args_dict = vars(args)

# Extract arguments from parser
num_commands = args_dict['num_commands']
tracefile = args_dict['tracefile']
seed = args_dict['seed']

# Initialize random
random.seed(seed)

# Determine path to insert file in
absolute_path = os.path.dirname(__file__)
relative_path = "../traces"
dir_path = os.path.join(absolute_path, relative_path)
complete_path = os.path.join(dir_path, str(tracefile))

file_to_write = open(complete_path, 'w')

file_to_write.write("s 0\n")
for i in range(1, num_commands):
    if (flip_coin()):
        file_to_write.write("g\n")
    else:
      val = random.randint(LOWVAL, HIGHVAL)
      str = "s " + str(val) + "\n"
      file_to_write.write(str)