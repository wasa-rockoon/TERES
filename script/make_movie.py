#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import subprocess
import shutil

FRAMERATE = 30
FRAMERATE_OUT = 60

FILE_NAME  = 'pic%d.jpg'
OUTPUT = 'out.mp4'

SKIP_MAX = 2

if len(sys.argv) != 2:
    print('invalid argument.')
    sys.exit(1)

path = sys.argv[1]

i = 0;
skipped = 0

while True:
    file_path = os.path.join(path, FILE_NAME % i)
    if os.path.exists(file_path):
        skipped = 0
        print(file_path)
    else:
        if i == 0:
            print('Cannot find any images.')
            sys.exit(1)

        if skipped > SKIP_MAX:
            print('finish')
            break;

        copy_file_path = os.path.join(path, FILE_NAME % (i-1))

        shutil.copyfile(copy_file_path, file_path)

        skipped += 1

        print(file_path + ' copied!')

    i += 1


subprocess.call([
    'ffmpeg',
    '-framerate', str(FRAMERATE),
    '-i', os.path.join(path, FILE_NAME),
    '-vcodec', 'libx264',
    '-pix_fmt', 'yuv420p',
    '-r', str(FRAMERATE_OUT),
    OUTPUT
])
