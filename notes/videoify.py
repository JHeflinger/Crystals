import subprocess
import os
import sys

def make_video(input_dir, output_file):
    # Ensure ffmpeg exists
    if subprocess.call(["ffmpeg", "-version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL) != 0:
        raise RuntimeError("ffmpeg is not installed or not in PATH.")

    # Build the ffmpeg command
    # %d.png expects image_0.png, image_1.png, ... image_n.png
    cmd = [
        "ffmpeg",
        "-y",                      # overwrite output
        "-framerate", "60",        # input framerate
        "-i", os.path.join(input_dir, "i_%d.png"),
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        "-crf", "18",              # high quality
        output_file
    ]

    subprocess.run(cmd, check=True)
    print("Video written to:", output_file)

if __name__ == "__main__":
    make_video(sys.argv[1], sys.argv[2])
