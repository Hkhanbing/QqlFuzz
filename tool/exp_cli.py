#!/usr/bin/env python3
# Use pwncli
# Date: 2024-08-08 11:30:33
# Editor: hkbin
# Usage:
#     Debug : python3 exp.py debug elf-file-path -t -b malloc -b \$rebase\(0x3000\)
#     Remote: python3 exp.py remote elf-file-path ip:port

from pwncli import *
cli_script()
set_remote_libc('libc.so.6')

context.arch="amd64"
file_path = "/home/hkbin/Workspace/chaitin_workspace/database_fuzz/tool/pwn"

io: tube = gift.io
elf: ELF = gift.elf
libc: ELF = gift.libc


def cmd(i, prompt):
    sla(prompt, i)

def add():
    cmd('1')
    #......

def edit():
    cmd('2')
    #......

def show():
    cmd('3')
    #......

def dele():
    cmd('4')
    #......

# one_gadgets: list = get_current_one_gadget_from_libc(more=False)
# one_gadgets: one_gadget_binary(binary_path, more)
# CurrentGadgets.set_find_area(find_in_elf=True, find_in_libc=False, do_initial=False)
# Shellcode：ShellcodeMall.amd64
# tcache safelinking: protect_ptr(address, next)
# tcache safelinking_de: reveal_ptr(data)
# recvlibc: recv_current_libc_addr(offset(int), timeout(int))
# set_libcbase: set_current_libc_base_and_log(addr(int), offset(str or int))
# set_elfbase: set_current_code_base_and_log(addr, offset)

# burp:
# for i in range(0x10):
#     try:
#         new_func()
#     except (EOFError):
#         gift.io = copy_current_io()


ia()
