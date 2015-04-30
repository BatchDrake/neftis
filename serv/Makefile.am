AUTOMAKE_OPTIONS = subdir-objects

SUBDIRS = fs auth

noinst_DATA = initrd/fs.asx initrd/auth.asx

# TODO: do this with wildcards

initrd/fs.asx: fs/fs.asx
	cp fs/fs.asx initrd

initrd/auth.asx: auth/auth.asx
	cp auth/auth.asx initrd
