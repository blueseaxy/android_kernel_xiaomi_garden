#! /usr/bin/python
# -*- coding: utf-8 -*-

# Copyright (C) 2016 MediaTek Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See http://www.gnu.org/licenses/gpl-2.0.html for more details.	

import os, sys
import getopt
import traceback
import subprocess
import xml.dom.minidom

sys.dont_write_bytecode = True

sys.path.append('.')
sys.path.append('..')

def usage():
    print('''
usage: DrvGen [dws_path] [file_path] [log_path] [paras]...

options and arguments:

dws_path    :    dws file path
file_path   :    where you want to put generated files
log_path    :    where to store the log files
paras        :    parameter for generate wanted file
''')

def is_oldDws(path, gen_spec):
    if not os.path.exists(path):
        log(LogLevel.error, 'Can not find %s' %(path))
        sys.exit(-1)

    try:
        root = xml.dom.minidom.parse(dws_path)
    except Exception(e):
        log(LogLevel.warn, '%s is not xml format, try to use old DCT!' %(dws_path))
        if len(gen_spec) == 0:
            log(LogLevel.warn, 'Please use old DCT UI to gen all files!')
            return True
        old_dct = os.path.join(sys.path[0], 'old_dct', 'DrvGen')
        cmd = old_dct + ' ' + dws_path + ' ' + gen_path + ' ' + log_path + ' ' + gen_spec[0]
        if 0 == subprocess.call(cmd, shell=True):
            return True
        else:
            log(LogLevel.error, '%s format error!' %(dws_path))
            sys.exit(-1)

    return False

if __name__ == '__main__':
    opts, args = getopt.getopt(sys.argv[1:], '')

    if len(args) == 0:
        msg = 'Too less arguments!'
        usage()
        log(LogLevel.error, msg)
        sys.exit(-1)

    dws_path = ''
    gen_path = ''
    log_path = ''
    gen_spec = []

    # get DWS file path from parameters
    dws_path = os.path.abspath(args[0])

    # get parameters from input
    if len(args) == 1:
        gen_path = os.path.dirname(dws_path)
        log_path = os.path.dirname(dws_path)

    elif len(args) == 2:
        gen_path = os.path.abspath(args[1])
        log_path = os.path.dirname(dws_path)

    elif len(args) == 3:
        gen_path = os.path.abspath(args[1])
        log_path = os.path.abspath(args[2])

    elif len(args) >= 4:
        gen_path = os.path.abspath(args[1])
        log_path = os.path.abspath(args[2])
        for i in range(3,len(args)):
            gen_spec.append(args[i])

   



    
