# Crystals (Spectral Pathtracer)

## Overview

Thanks for your interest in Crystals! This repository is a CPU spectral pathtracer. What does this mean? It means that this program can render scenes using realistic materials with practically no dependencies! Rather than using r, g, and b values to render color, we use the entire spectrum of light, from 100 nm to 700 nm! Because of this, you can get some really nice effects such as fluorescence and dispersion!

## Things to note

Below are some things to consider before using this renderer. Happy rendering!

### Scene formats

This renderer uses `.obj` files to represent scenes. In other words, as long as you can save a scene as an `.obj` file, this can render it! There are some slight additions to the format to represent lights, cameras, and more, but overall the geometry defined with vertices and faces can be imported from practically any common modeling software using the `.obj` format. For more details on how Crystals adds onto the `.obj` specification, see `notes/obj_specification.txt`.

### Material formats

While materials are loaded from a `.mtl` file and set using `usemtl` just like all `obj` formats, the contents and specification of how materials are defined are completely custom. Material properties are defined largely as fourier transforms that transform inputs (typically the wavelengths of light) to emulate physically accurate physical properties. For example, if we were to define the color of a material, we use a fourier transform that represents absorbtion of light as our material property. For more details on what properties are available and how to define them, usee `notes/material_specification.txt`.

### Performance

Note that this is a CPU renderer, so how fast it renderers will depend on how good your CPU is, not your GPU! Crystals does however feature pool-based maximized thread count parallelism, so it will automatically detect how many cores your machine has and parallelize the work evenly and for max occupancy throughout the duration of a render. Despite this however, don't be surprised if renders still take a LONG time! Pathtracing is ultimately a very heavy task, so it is expected for renders (especially ones with high sample counts or resolution) to take a very long time. It is recommended that you start with a low resolution (such as 360p) and a low sample count (such as 4) to test your scene and then increase these numbers later to refine your render.

## How to use

This project builds with CMake, so as long as you have CMake, you should be able to just use the CMakeLists.txt to compile it! There are no particular dependencies, so this should also work cross platform and without the need to install or consider any external libs. The only external libraries used are `glm` and `stb`, but those are included within the repository in the `vendor/` folder, so there shouldn't be any issue. If you're on linux, you can also use the provided `run.sh`, `build.sh`, and `clean.sh` scripts to do this for you.

To run the renderer, run the executable with the following format:

```
./spectrum.exe <input_path> <output_path> <samples> <width> <height>
```

The input path should be your path to your `.obj` scene file, and the output path will be where the render will save the `.png` to. The number of samples will determine how many samples the pathtracer will use per pixel, so note that performance will scale down roughly linearly as this increases. Width and height are the resolution of the output image.

If you'd like an example of rendering using this, use the following command to test out the diamond scene file!

```
./spectrum.exe resources/diamond.obj out.png 64 1280 720
```

This should render one of the example scenes to something similar to the following output! (noise may vary)

[Diamonds](outputs/example.png)
```
```

