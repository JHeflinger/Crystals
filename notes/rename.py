import sys

def main():
    for i in range(0, 193):
        fname = "videos/raws_old/image_" + i.__str__() + ".png"
        newname = "videos/raws/i_" + (i*2).__str__() + ".png"
        with open(fname, "rb") as f:
            with open(newname, "wb") as f2:
                f2.write(f.read())

if __name__ == "__main__":
    main()
