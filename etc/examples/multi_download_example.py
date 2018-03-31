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

from subprocess import Popen, PIPE, CREATE_NEW_CONSOLE
from os import devnull
from shutil import copyfileobj
from time import sleep

sys.stdout.write('INetGet multi-download *example* script\n\n')

if (len(sys.argv) < 4) or (not sys.argv[1].strip()) or (not sys.argv[2].strip()) or (not sys.argv[3].strip()):
    sys.stdout.write('Usage:\n   multi_download_example.py <num_chunks> <download_url> <out_filename>\n\n')
    sys.stdout.write('Example:\n   multi_download_example.py 5 \\\n      http://releases.ubuntu.com/16.04.4/ubuntu-16.04.4-desktop-amd64.iso \\\n      ubuntu-16.04.4-desktop-amd64.iso\n\n')
    sys.exit(-1)

NCHUNKS = int(sys.argv[1])
ADDRESS = sys.argv[2].strip()
OUTNAME = sys.argv[3].strip()

if (NCHUNKS < 1) or (NCHUNKS > 9):
    sys.stdout.write('ERROR: The number of chunks must be in the 1 to 9 range!\n\n')
    sys.exit(-1)


##############################################################################
# STEP #1: Determine the total file size
##############################################################################

sys.stdout.write('Determining file size, please wait...\n')

try:
	process = Popen(['INetGet.exe', '--verb=HEAD', ADDRESS, devnull], stderr=PIPE)
	stdout, stderr = process.communicate()
except:
	sys.stdout.write('\nERROR: Failed to launch INetGet process! Is INetGet.exe in path?\n\n')
	raise

if not process.returncode == 0:
    sys.stdout.write('ERROR: Failed to determine file size!\n\n')
    sys.stdout.write(stderr.decode("utf-8"))
    sys.exit(-1)

match = re.search(r"Content\s+length\s*:\s*(\d+)\s*Byte", stderr.decode("utf-8"), re.IGNORECASE)

if not match:
    sys.stdout.write('\nERROR: Failed to determine file size!\n\n')
    sys.stdout.write(stderr.decode("utf-8"))
    sys.exit(-1)

sys.stdout.write('Done.\n\n')

size_total = int(match.group(1))
sys.stdout.write('Total file size is: %d Byte\n\n' % size_total)

if size_total < 1:
    sys.stdout.write('\nERROR: File appears to be empty!\n\n')
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
        proc_list.append(Popen(['INetGet.exe', '--range-off=%d' % offset, '--range-end=%d' % range_end, ADDRESS, OUTNAME+"~%d" % file_no], creationflags=CREATE_NEW_CONSOLE))
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
    sleep(.01)

if success:
    sys.stdout.write('Completed.\n\n')
else:
    sys.stdout.write('\nERROR: At least one chunk failed to download!\n\n')
    sys.exit(-1)


##############################################################################
# STEP #4: Concatenate chunk files
##############################################################################

sys.stdout.write('Concatenating chunks, please wait...\n')

with open(OUTNAME, 'wb') as wfd:
    file_no = 0
    for i in range(0, NCHUNKS):
        with open(OUTNAME+"~%d" % file_no, 'rb') as fd:
            copyfileobj(fd, wfd)
            file_no = file_no + 1

sys.stdout.write('Done.\n\n')
