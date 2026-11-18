#!/usr/bin/env python3
"""img.py <lo> <hi> : raw image disassembly with absolute operands"""
import sys,os
sys.path.insert(0,os.getcwd()+'/recovery/tools'); import fe
lo,hi=int(sys.argv[1],16),int(sys.argv[2],16)
for i in fe.md.disasm(fe.imgbytes(lo,hi),lo):
    print('%x %s %s'%(i.address,i.mnemonic,i.op_str))
