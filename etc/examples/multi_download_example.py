##############################################################################
# INetGet - Lightweight command-line front-end to WinINet API
# Copyright (C) 2018 LoRd_MuldeR <MuldeR2@GMX.de>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
# See https:#www.gnu.org/licenses/gpl-2.0-standalone.html for details!
##############################################################################

import sys
import re
import math

from os import devnull, remove
from subprocess import Popen, PIPE, DEVNULL, CREATE_NEW_CONSOLE
from shutil import copyfileobj
from time import sleep

sys.stdout.write('INetGet multi-download *example* script\n\n')

if (len(sys.argv) < 4) or (sys.argv[1].lower() == '--help') or (sys.argv[1] == '/?'):
    sys.stdout.write('Usage:\n')
    sys.stdout.write('   multi_download_example.py [options] <num_chunks> <download_url> <out_filename>\n\n')
    sys.stdout.write('Options:\n')
    sys.stdout.write('   --no-progress  Do not display progress output of sub-processes\n\n')
    sys.stdout.write('Example:\n')
    sys.stdout.write('   multi_download_example.py 5 \\\n')
    sys.stdout.write('      http://releases.ubuntu.com/16.04.4/ubuntu-16.04.4-desktop-amd64.iso \\\n')
    sys.stdout.write('      ubuntu-16.04.4-desktop-amd64.iso\n\n')
    sys.stdout.write('Note: Additional options will be passed through to INetGet.\n\n')
    sys.exit(-1)

arg_offset, hidden_mode, extra_args = 1, False, []
while sys.argv[arg_offset].startswith('--'):
    switch_name = sys.argv[arg_offset][2:].lower().split("=")[0]
    if not switch_name:
        break
    if switch_name == "no-progress":
        hidden_mode = True
    elif (switch_name == "range-end") or (switch_name == "range-off") or (switch_name == "verb"):
        sys.stdout.write('WARNING: Switch "%s" is ignored!\n\n' % sys.argv[arg_offset])
    else:
        extra_args.append(sys.argv[arg_offset])
    arg_offset = arg_offset + 1

if arg_offset+2 >= len(sys.argv):
    sys.stdout.write('ERROR: At least one required argument is missing! Use "--help" for details.\n\n')
    sys.exit(-1)

NCHUNKS = int(sys.argv[arg_offset]) if sys.argv[arg_offset].isdigit() else 0
ADDRESS = sys.argv[arg_offset+1].strip()
OUTNAME = sys.argv[arg_offset+2].strip()

if (not ADDRESS) or (not OUTNAME):
    sys.stdout.write('ERROR: The download URL ("%s") or the output file name ("%s") is empty!\n\n' % (sys.argv[arg_offset+1], sys.argv[arg_offset+2]))
    sys.exit(-1)

if (NCHUNKS < 1) or (NCHUNKS > 9):
    sys.stdout.write('ERROR: The number of chunks ("%s") must be within the 1 to 9 range!\n\n' % sys.argv[arg_offset])
    sys.exit(-1)


##############################################################################
# STEP #1: Determine the total file size
##############################################################################

sys.stdout.write('Determining file size, please wait...\n')

try:
    process = Popen(['INetGet.exe', '--verb=HEAD', *extra_args, ADDRESS, devnull], stderr=PIPE)
    stdout, stderr = process.communicate()
except:
    sys.stdout.write('\nERROR: Failed to launch INetGet process! Is INetGet.exe in the path?\n\n')
    raise

if not process.returncode == 0:
    sys.stdout.write('\nERROR: Failed to determine file size! Please see INetGet log below for details.\n\n')
    sys.stdout.write(stderr.decode("utf-8"))
    sys.exit(-1)

match = re.search(r"Content\s+length\s*:\s*(\d+)\s*Byte", stderr.decode("utf-8"), re.IGNORECASE)

if not match:
    sys.stdout.write('\nERROR: Failed to determine file size! Does the server offer "resume" support?\n\n')
    sys.stdout.write(stderr.decode("utf-8"))
    sys.exit(-1)

sys.stdout.write('Done.\n\n')

size_total = int(match.group(1))
sys.stdout.write('Total file size is: %d Byte\n\n' % size_total)

if size_total < 1:
    sys.stdout.write('\nERROR: The requested file appears to be empty!\n\n')
    sys.exit(-1)

if size_total < (NCHUNKS * 1024):
    sys.stdout.write('\nERROR: The requested file is too small for chunk\'ed download!\n\n')
    sys.exit(-1)


##############################################################################
# STEP #2: Start the download processes
##############################################################################

size_chunk = size_total // NCHUNKS
size_rmndr = math.fmod(size_total, size_chunk)

while size_rmndr >= NCHUNKS:
    size_chunk = size_chunk + (size_rmndr // NCHUNKS)
    size_rmndr = math.fmod(size_total, size_chunk)

sys.stdout.write('Chunksize: %d\n'   % size_chunk)
sys.stdout.write('Remainder: %d\n\n' % size_rmndr)

proc_list = []
offset, file_no = 0, 0

digits = len(str(size_total - 1))
format = 'Chunk #%%d range: %%0%dd - %%0%dd\n' % (digits, digits)

if size_chunk > 0:
    for i in range(0, NCHUNKS):
        range_end = offset + size_chunk - 1 + (0 if (i < NCHUNKS-1) else size_rmndr) #add remainder to *last* chunk!
        sys.stdout.write(format % (file_no, offset, range_end))
        proc_list.append(Popen(['INetGet.exe', \
            '--range-off=%d' % offset, '--range-end=%d' % range_end, *extra_args,
            ADDRESS, OUTNAME+"~chunk%d" % file_no], \
            stderr = (DEVNULL if hidden_mode else None), creationflags = (0 if hidden_mode else CREATE_NEW_CONSOLE)))
        offset, file_no = offset + size_chunk, file_no + 1
        sleep(.25)

sys.stdout.write('\nDownloads are running in the background, please be patient...\n')


##############################################################################
# STEP #3: Wait for completion...
##############################################################################

success = True
procs_completed = []

while len(procs_completed) < len(proc_list):
    for i in range(0, len(proc_list)):
        subproc = proc_list[i]
        if subproc in procs_completed:
            continue
        retval = subproc.poll()
        if retval == None:
            continue;
        procs_completed.append(subproc)
        if retval != 0:
            sys.stdout.write('Chunk #%d failed !!!\n' % i)
            success = False
            break
        sys.stdout.write('Chunk #%d succeeded.\n' % i)
    sleep(.015625)

if success:
    sys.stdout.write('Completed.\n\n')
else:
    sys.stdout.write('\nERROR: At least one chunk failed to download!\n\n')
    sys.exit(-1)


##############################################################################
# STEP #4: Concatenate chunk files
##############################################################################

sys.stdout.write('Concatenating chunks, please wait...\n')

try:
    with open(OUTNAME, 'wb') as wfd:
        for i in range(0, NCHUNKS):
            with open(OUTNAME+"~chunk%d" % i, 'rb') as fd:
                copyfileobj(fd, wfd)
except:
    sys.stdout.write('\nERROR: Failed to concatenate chunk files. Out of space?\n\n')
    raise

sys.stdout.write('Done.\n\n')

try:
    for i in range(0, NCHUNKS):
        remove(OUTNAME+"~chunk%d" % i)
except:
    sys.stdout.write('\nWARNING: Failed to remove temporary files!\n\n')
