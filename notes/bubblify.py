import sys
import random

def main():
    if len(sys.argv) != 3:
        print("Usage: python bubblify.py <input_file> <output_file>")
        return
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    with open(input_file, "r") as fin, open(output_file, "w") as fout:
        for line in fin:
            stripped = line.lstrip()
            newline = stripped
            if stripped.startswith("f "):
                parts = line.split(" ")
                center = parts[1]
                radius = random.uniform(0.05, 0.2)
                newline = "sphere " + center + " " + radius.__str__() + "\n"
            fout.write(newline)

if __name__ == "__main__":
    main()
