#!/usr/bin/env python2.7

import sys

if len(sys.argv) < 3:
    print "usage: compare.py a b"
    sys.exit(1)

a = open(sys.argv[1], 'r')
b = open(sys.argv[2], 'r')

while True:
    a_line = a.readline()
    b_line = b.readline()
    
    if not a_line and not b_line:
        break

    if not a_line or not b_line:
        print "unequal number of lines in these files"
        break

    a_pos, a_accel = a_line.split(' ')
    b_pos, b_accel = b_line.split(' ')

    split = a_pos[1:-1].split(',')
    a_pos = (split[0], split[1], split[2], split[3])
    split = b_pos[1:-1].split(',')
    b_pos = (split[0], split[1], split[2], split[3])

    if a_pos != b_pos:
        print "position mismatch: %s vs %s" % (a_pos, b_pos)
        continue

    split = a_accel[1:-1].split(',')
    a_accel = (split[0], split[1], split[2])    
    split = b_accel[1:-1].split(',')
    b_accel = (split[0], split[1], split[2])
    
    if a_accel != b_accel:
        print "accel mismatch: %s %s vs %s %s" % (a_pos, a_accel, b_pos, b_accel)

    
