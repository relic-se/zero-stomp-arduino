#!/usr/bin/python3

# SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
#
# SPDX-License-Identifier: GPLv3

try:
    import argparse
    import array
    import os.path
    import wave
except ImportError as e:
    print(e.msg)
    exit()

parser = argparse.ArgumentParser(
    prog="wav2c",
    description="Convert .wav to c array",
    epilog="Copyright (c) 2025 Cooper Dalrymple, licensed under GPLv3",
)
parser.add_argument('filename', type=str, help="Path to the WAV file (.wav), must be 16-bit mono")
parser.add_argument('-n', '--name', type=str, help="Name of the output variable, uses file basename by default")
parser.add_argument('-o', '--output', action="store_true", help="Output to a header file")
parser.add_argument('-f', '--file', type=str, help="Path to the header file (.h) to save the output")
parser.add_argument('-t', '--tab', type=int, default=4, help="Tab size")
parser.add_argument('-c', '--columns', type=int, default=16, help="Number of columns")
parser.add_argument('-w', '--width', type=int, default=8, help="Column width")

args = parser.parse_args()

if not args.filename.endswith('.wav') or not os.path.isfile(args.filename):
    print("Invalid wav file")
    exit()

if args.output and args.file and not args.file.endswith('.h'):
    print("Invalid header file")
    exit()

if args.tab < 0:
    print("Invalid tab size")
    exit()

if args.columns < 1:
    print("Invalid number of columns")
    exit()

if args.width < 0:
    print("Invalid column width")
    exit()

name = os.path.splitext(os.path.basename(args.filename))[0] if not args.name else args.name
output_file = "" if not args.output else (args.file if args.file else "{}.h".format(name))

output = ""
with wave.open(args.filename) as file:
    if file.getnchannels() != 1:
        print("Only mono files supported")
        exit()

    if file.getsampwidth() != 2:
        print("Only 16-bit sample width supported")
        exit()

    nframes = file.getnframes()
    frames = array.array('h', file.readframes(nframes))

    output += "// Generated using wav2c.py\n\n"

    output += "#ifndef _{:s}_H\n".format(name.upper())
    output += "#define _{:s}_H\n\n".format(name.upper())

    output += "#define {:s}_LENGTH ({:d})\n".format(
        name.upper(),
        nframes
    )
    output += "#define {:s}_SAMPLE_RATE ({:d})\n\n".format(
        name.upper(),
        file.getframerate()
    )

    output += "const int16_t {:s}[{:s}_LENGTH] PROGMEM = {{\n".format(name, name.upper())

    column = 0
    for i in range(nframes):
        if not column:
            output += " " * args.tab
        element = str(frames[i])
        if i < nframes - 1:
            element += ","
        if column == args.columns - 1 or i == nframes - 1:
            element += "\n"
        else:
            element = element.ljust(args.width - 1) + " "
        output += element
        column = (column + 1) % args.columns
    
    output += "};\n\n"

    output += "#endif // _{:s}_H\n".format(name.upper())

if args.output:
    with open(output_file, 'w+') as file:
        file.write(output)
else:
    print(output)
