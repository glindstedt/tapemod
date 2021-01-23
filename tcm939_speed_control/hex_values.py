#!/usr/bin/env python3

import math
import numpy as np
from numpy import interp

def func(x):
    return math.pow(x, 9)

def scale(x, max_value, lower_bound, upper_bound):
    return (float(x) / float(max_value)) * (upper_bound - lower_bound) + lower_bound

print(math.pow(255, 2))

lower_bound = 29
upper_bound = 255
upper_padding = 5

index_range = range(0, 255)
x_values = range(0, 256-upper_padding)

max_value = func(upper_bound)
print(max_value)
print(scale(func(x_values[-1]), max_value, lower_bound, upper_bound))

values = [ scale(func(x), max_value, lower_bound, upper_bound) for x in x_values]

for i in range(0, upper_padding):
    values.append(255)

# print(values)
# x_coords = range(1, 256)

#value_range = np.log(index_range)
#interpol_range = interp(x_coords, x_coords, value_range)
#values = range(1,256)

print("{", end="")
for idx, v in enumerate(values):
    if idx % 16 == 0:
        print("\n  ", end="")
    print(f'0x{int(round(v)):02x}, ', end="")
    #print(f'{v}, ', end="")
print()
print("};")

print(math.log(1))
print(math.log(256))
