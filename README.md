# blob - a 8-neighbour connected components labelling and contours extractor.

See [blob.h](blob.h) for a complete documentation.

## Reference
"A linear-time component-labeling algorithm using contour tracing technique"
 by Fu Chang, Chun-Jen Chen, and Chi-Jen Lu.

## Example
The [test](test) directory contains the source code of a small program that generates an image of the label buffer as long as a JSON file and a GNUplot data file containing the set of extracted contours.

#### source
<img src="test/data/dummy.png" width="320px"/>

#### label
<img src="test/result/dummy_label.png" width="320px"/> 

#### contours
<img src="test/result/dummy_plot.png" width="320px"/> 

Image reading and writing libraries [stb_image.h, stb_image_write.h](https://github.com/nothings/stb/) by Sean Barrett (public domain).

## Build

A CMake configuration file is provided in order to build a static library and the associated documentation.
A typical usage of CMake may be:
```bash
cmake -B build -S .
cmake --build ./build
```
On a Linux system, the CMake script will generate a static library `libblob.a`. 

If the `BUILD_TEST` option is set, the build script will generate an executable named `label` (see [Example](#example)).

The [DoxyGen](http://www.stack.nl/~dimitri/doxygen/) doxygen can be built by setting the `BUILD_DOC` option and building the `doc` target.
```bash
cmake --build ./build --target doc
```

## License
`blob` is licensed under the MIT License, see the [LICENSE](LICENSE) file for more information.
