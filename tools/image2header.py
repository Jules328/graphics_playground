#! /usr/bin/python3

import argparse
import math
import sys
import os
from PIL import Image

def get_pixel_string(image: Image) -> str:
    pixels = image.load()
    mode = image.mode
    (w,h) = image.size

    pixel_str = ""
    for j in range(h):
        pixel_str += "\t{ "
        for i in range(w):
            pixel = pixels[i,j]
            if mode == "RGBA":
                (r, g, b, a) = pixel
                # pixel_str += f"{{{r}u, {g}u, {b}u, {a}u}}"
                if r == g and g == b:
                    lum = r
                else:
                    lum = round(math.sqrt((r/255)**2 + (g/255)**2 + (b/255)**2)*255)
                pixel_str += f"{lum}u"
            elif mode == "RGB":
                (r, g, b) = pixel
                # pixel_str += f"{{{r}u, {g}u, {b}u}}"
                if r == g and g == b:
                    lum = r
                else:
                    lum = round(math.sqrt((r/255)**2 + (g/255)**2 + (b/255)**2)*255)
                pixel_str += f"{lum}u"
            else:
                pixel_str += f"{pixel}u"
            
            # add bracket at end of line
            if i == w - 1:
                pixel_str += " }"
            # add comma between each pixel
            if i != w - 1 or j != h - 1:
                pixel_str += ", "
        if j != h -1:
            pixel_str += "\n"
    
    return pixel_str

def get_header_string(name: str, image: Image) -> str:
    (w,h) = image.size
    m_name = name.upper() # macro name
    v_name = name.lower() # variable name

    parr = ""
    bytes_per_pixel = 1
    # if image.mode == "RGBA":
    #     parr = "[4]"
    #     bytes_per_pixel = 4
    # elif image.mode == "RGB":
    #     parr = "[3]"
    #     bytes_per_pixel = 3

    header_string  = "\n".join([
    f"#ifndef _{m_name}_H_",
    f"#define _{m_name}_H_",
    "",
    "#include <stdint.h>",
    "",
    f"#define {m_name}_HEIGHT     {h}u",
    f"#define {m_name}_WIDTH      {w}u",
    f"#define {m_name}_NUM_PIXELS {w*h}u",
    f"#define {m_name}_SIZE_BYTES {w*h*bytes_per_pixel}u",
    "",
    f"const uint8_t {v_name}_pixels[{h}][{w}]{parr} = {{",
    get_pixel_string(image),
    "};",
    "",
    f"#endif /* _{m_name}_H_ */"
    ])

    return header_string

if __name__ == "__main__":
    # parse arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('image', help="The image file to convert")
    parser.add_argument('-n', '--name', help="The name of the output file")
    parser.add_argument('-p', '--path', help="The path to the output directory of the header")
    args = parser.parse_args()

    # get the file's name if no name is provided
    if args.name is None:
        args.name = args.image.split('.')[0]
    if args.path is None:
        args.path = ""

    # get pixel values
    with Image.open(args.image) as im:
        (w, h) = im.size
        print(f'Opened "{args.image}": {im.format}, {w}x{h}, {im.mode}')
        os.makedirs(args.path, exist_ok=True)
        with open(f"{args.path}/{args.name}.h", 'w') as outfile:
            print(f'Writing image header to "{args.path}/{args.name}.h"')
            print(get_header_string(args.name, im), file=outfile)
