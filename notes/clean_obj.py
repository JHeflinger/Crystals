import sys

def main():
    if len(sys.argv) != 3:
        print("Usage: python clean_obj.py <input_file> <output_file>")
        return
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    with open(input_file, "r") as fin, open(output_file, "w") as fout:
        for line in fin:
            stripped = line.lstrip()
            if stripped.startswith("v ") or stripped.startswith("f "):
                newline = stripped
                if stripped.startswith("f "):
                    parts = stripped.split(" ")[1:]
                    newline = "f"
                    for part in parts:
                        newline += " " + part.split("/")[0].lstrip()
                    newline += "\n"
                fout.write(newline)

if __name__ == "__main__":
    main()
