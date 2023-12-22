#! /usr/bin/env python3

import os
import sys
import argparse

######################################################################
# Parsing optional command line arguments
######################################################################

import argparse

parser = argparse.ArgumentParser(description="""
Generate an instance of the normal classification problem with the
syntax used by ClassBench for IPv4 5-tuples, and the rules specified
as described in reference [1], Section 4.2.

With parameter K, where K is in the range [1,65536], generates 4*K
rules, where the rules are:

(1) K rules that are exact match on different IPv4 source addresses,
    exact match TCP on protocol, wildcard on all other fields.
(2) Same as (1), except for IPv4 dest address instead of IPv4 source
    address.
(3) Same as (1), except for L4 source port instead of IPv4 source
    address.
(3) Same as (1), except for L4 destination port instead of IPv4 source
    address.


[1] Hari Adiseshu, Subhash Suri, and Guru Parulkar. 1999. "Packet
    Filter Management for Layer 4 Switching",
    https://www.researchgate.net/publication/2447037_Packet_Filter_Management_for_Layer_4_Switching
    """)
parser.add_argument('--k', dest='K', type=int, required=True)
args = parser.parse_known_args()[0]

#print("K=%d" % (args.K))
if args.K < 1 or args.K > 65536:
    print("K must be in range 1 through 65536")
    sys.exit(1)


def print_rule(sa, sa_plen, da, da_plen, proto,
               sport, sport_wild, dport, dport_wild):
    print("@%d.%d.%d.%d/%d %d.%d.%d.%d/%d %d : %d %d : %d 0x%02x/0x%02x"
          "" % ((sa >> 24) & 0xff,
                (sa >> 16) & 0xff,
                (sa >>  8) & 0xff,
                (sa >>  0) & 0xff,
                sa_plen,
                (da >> 24) & 0xff,
                (da >> 16) & 0xff,
                (da >>  8) & 0xff,
                (da >>  0) & 0xff,
                da_plen,
                    0 if sport_wild else sport,
                65535 if sport_wild else sport,
                    0 if dport_wild else dport,
                65535 if dport_wild else dport,
                proto,
                0xff))

IPPROTO_TCP = 6

for field in ['sa', 'da', 'sport', 'dport']:
    sa = 0
    sa_plen = 0
    da = 0
    da_plen = 0
    sport = 0
    sport_wild = True
    dport = 0
    dport_wild = True
    if field == 'sa':
        sa_plen = 32
    elif field == 'da':
        da_plen = 32
    elif field == 'sport':
        sport_wild = False
    elif field == 'dport':
        dport_wild = False
    for i in range(args.K):
        print_rule(sa, sa_plen, da, da_plen, IPPROTO_TCP,
                   sport, sport_wild, dport, dport_wild)
        if field == 'sa':
            sa += 1
        elif field == 'da':
            da += 1
        elif field == 'sport':
            sport += 1
        elif field == 'dport':
            dport += 1
