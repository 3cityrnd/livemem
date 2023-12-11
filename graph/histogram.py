

import json
import matplotlib.pyplot as plt
import numpy as np
import sys
import argparse


parser = argparse.ArgumentParser(description="LIVEMEM PARSER")
parser.add_argument(
        "--yfrom",
        type=int,
        default=0,
        help="Filter from",
)
parser.add_argument(
        "--yto",
        type=int,
        default=sys.maxsize,
        help="Filter to",
    )

parser.add_argument(
        "--file",
        type=str,
        default='livemem.json',
        help="Input file",
    )

args = parser.parse_args()



yfrom=args.yfrom
yto=args.yto


with open(args.file, 'r') as file:
    data = json.load(file)

# Prepare lists for all data
all_data = []

if "0" in data:
    del data["0"]

# Convert data to the appropriate fo  rmat
for key, values in data.items():       
  if int(key) >= yfrom and int(key)<=yto: 
        for value in values:       
           all_data.append((value , int(key)))
        

# Unzip the data into separate lists for x and y coordinates
x, y = zip(*all_data)


norm_y = np.array(y)
norm_y = norm_y / norm_y.max()

# Get unique keys
unique_keys = set(list(y))

print(unique_keys)

# Create a scatter plot with different colors for each key
fig, ax = plt.subplots()
scatter = ax.scatter(x, y, c=norm_y, cmap='viridis', marker='o')

# Add labels and title
plt.xlabel('X Time ms')
plt.ylabel('Y Chunk of memory')
plt.title('Memory allocation pic')

plt.ticklabel_format(style='plain', axis='both')

# Display the plot
plt.show()
