#!/usr/bin/env python3

import argparse
import functools
import math
import numpy as np
from numpy import interp
from typing import List

def scale(x: int, from_lower_bound: int, from_upper_bound: int, to_lower_bound: int, to_upper_bound: int) -> int:
    scale = float(to_upper_bound - to_lower_bound)
    value = scale * float(x-from_lower_bound) / float(from_upper_bound-from_lower_bound)

    return math.ceil(to_lower_bound + value)

def generate_values(length: int, top: int, bottom: int, power: int) -> List[int]:

    lower_bound = bottom
    upper_bound = top
    upper_padding = 0

    x_values = range(0, length)

    from_lower_bound = math.pow(x_values[0], power)
    from_upper_bound = math.pow(x_values[-1], power)

    values = [
        scale(math.pow(x, power), from_lower_bound, from_upper_bound, lower_bound, upper_bound)
        for x in x_values
    ]

    for i in range(0, upper_padding):
        values.append(upper_bound)

    return values

def print_hex(values):
    print("{", end="")
    for idx, v in enumerate(values):
        if idx % 16 == 0:
            print("\n  ", end="")
        print(f'0x{int(round(v)):02x}, ', end="")
    print()
    print("};")

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument('--length', type=functools.partial(int, base=0), help='how many values', required=True)
    parser.add_argument('--top', type=functools.partial(int, base=0), help='maximum value', required=True)
    parser.add_argument('--bottom', type=functools.partial(int, base=0), help='minimum value', required=True)
    parser.add_argument('--power', type=functools.partial(float), help='power', default=2.0)

    args = parser.parse_args()
    print(args)

    values = generate_values(args.length, args.top, args.bottom, args.power)

    print(values)
    print_hex(values)
