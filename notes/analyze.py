import sys

def main():
    if len(sys.argv) != 2:
        print("Usage: python analyze.py <input_file>")
        return
    input_file = sys.argv[1]
    vs = 0
    faces = 0
    cameras = 0
    lights = 0
    spheres = 0
    ngs = 0
    with open(input_file, "r") as f:
        for line in f:
            if line.lstrip().startswith("v"):
                vs += 1
            if line.lstrip().startswith("f"):
                faces += 1
            if line.lstrip().startswith("camera"):
                cameras += 1
            if line.lstrip().startswith("ld"):
                lights += 1
            if line.lstrip().startswith("sphere"):
                spheres += 1
            if line.lstrip().startswith("ng"):
                ngs += 1
    print("Vertices: " + vs.__str__())
    print("Faces: " + faces.__str__())
    print("Cameras: " + cameras.__str__())
    print("Lights: " + lights.__str__())
    print("Spheres: " + spheres.__str__())
    print("Non-geometric values: " + ngs.__str__())

if __name__ == "__main__":
    main()
