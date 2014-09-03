bin_PROGRAMS = samba
samba_SOURCES = samba.c cmdline.c comm.c sam.c sambaflash.c fsio.c crc-1021.c \
	batch.c

# set the include path found by configure
INCLUDES = $(all_includes)

# the library search path.
samba_LDFLAGS = $(all_libraries) 
samba_LDADD = -lreadline -lncurses
noinst_HEADERS = cmdline.h defs.h comm.h sam.h fsio.h crc-1021.h batch.h

