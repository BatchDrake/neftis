AUTOMAKE_OPTIONS = subdir-objects

SUBDIRS = fs 

noinst_DATA = initrd/fs.asx

# TODO: do this with wildcards

initrd/fs.asx: fs/fs.asx
	cp fs/fs.asx initrd
