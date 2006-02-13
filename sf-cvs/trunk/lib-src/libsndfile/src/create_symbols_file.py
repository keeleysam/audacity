#!/usr/bin/python

# Copyright (C) 2003,2004 Erik de Castro Lopo <erikd@mega-nerd.com>
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#     * Neither the author nor the names of any contributors may be used
#       to endorse or promote products derived from this software without
#       specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import re, sys

#----------------------------------------------------------------
# These are all of the public functions exported from libsndfile.
#
# Its important not to change the order they are listed in or
# the ordinal values in the second column.

ALL_SYMBOLS = (
	(	"sf_command", 		1	),
	(	"sf_open",			2	),
	(	"sf_close",			3	),
	(	"sf_seek",			4	),
	(	"sf_error",			7	),
	(	"sf_perror",		8	),
	(	"sf_error_str",		9	),
	(	"sf_error_number",	10	),
	(	"sf_format_check",	11	),
	(	"sf_read_raw",		16	),
	(	"sf_readf_short",	17	),
	(	"sf_readf_int",		18	),
	(	"sf_readf_float",	19	),
	(	"sf_readf_double",	20	),
	(	"sf_read_short",	21	),
	(	"sf_read_int",		22	),
	(	"sf_read_float",	23	),
	(	"sf_read_double",	24	),
	(	"sf_write_raw",		32	),
	(	"sf_writef_short",	33	),
	(	"sf_writef_int",	34	),
	(	"sf_writef_float",	35	),
	(	"sf_writef_double",	36	),
	(	"sf_write_short",	37	),
	(	"sf_write_int",		38	),
	(	"sf_write_float",	39	),
	(	"sf_write_double",	40	),
	(	"sf_strerror",		50	),
	(	"sf_get_string",	60	),
	(	"sf_set_string",	61	),
	(	"sf_open_fd",		70	),
	(	"sf_open_virtual",	80	),
	(	"sf_write_sync",	90	)
	)

#-------------------------------------------------------------------------------

def linux_symbols (progname, version):
	print "# Auto-generated by %s\n" %progname
	print "libsndfile.so.%s" % version
	print "{"
	print "  global:"
	for name, ordinal in ALL_SYMBOLS:
		print "    %s ;" % name
	print "  local:"
	print "    * ;"
	print "} ;"
	print
	return

def darwin_symbols (progname, version):
	print "# Auto-generated by %s\n" %progname
	for name, ordinal in ALL_SYMBOLS:
		print "_%s" % name
	print
	return

def win32_symbols (progname, version):
	print "; Auto-generated by %s\n" %progname
	print "LIBRARY libsndfile.dll"
	print "EXPORTS\n"
	for name, ordinal in ALL_SYMBOLS:
		print "%-20s @%s" % (name, ordinal)
	print
	return

def no_symbols (os_name):
	print
	print "No known way of restricting exported symbols on '%s'." % os_name
	print "If you know a way, please contact the author."
	print
	return

#-------------------------------------------------------------------------------

progname = re.sub (".*[\\/]", "", sys.argv [0])

if len (sys.argv) != 3:
	print
	print "Usage : %s <target OS name> <libsndfile version>." % progname
	print
	print "    Currently supported values for target OS are:"
	print "          linux"
	print "          darwin     (ie MacOSX)"
	print "          win32      (ie wintendo)"
	print
	sys.exit (1)

os_name = sys.argv [1]
version = re.sub ("\.[a-z0-9]+$", "", sys.argv [2])

if os_name == "linux":
	linux_symbols (progname, version)
elif os_name == "darwin":
	darwin_symbols (progname, version)
elif os_name == "win32":
	win32_symbols (progname, version)
else:
	no_symbols (os_name)

sys.exit (0)

# Do not edit or modify anything in this comment block.
# The arch-tag line is a file identity tag for the GNU Arch
# revision control system.
#
# arch-tag: 5814f35c-318f-4023-a0c3-d9cf7c9e5f6c

