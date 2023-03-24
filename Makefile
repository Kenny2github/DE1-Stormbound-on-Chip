#############
# Utilities #
#############

#Recursive version of wildcard (i.e. it checks all subdirectories)
# For example:
# 	$(call rwildcard, /tmp/, *.c *.h)
# will return all files ending in .c or .h in /tmp and any of tmp's sub-directories
#NOTE: the directory (e.g. /tmp/) should end with a slash (i.e. /)
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

#Recursively find directories containing specifc files (useful for finding header include directories)
# For example:
# 	$(call rfiledirs, /tmp/, *.h *.hpp)
# will return all directories in /tmp which contain .h or .hpp files
#NOTE: the directory (e.g. /tmp/) should end with a slash (i.e. /)
rfiledirs=$(sort $(dir $(call rwildcard, $1, $2)))

BUILD = build
SRC_DIR = src

SRC_FILES=$(call rwildcard, $(SRC_DIR), *.c)

combined.c: $(SRC_FILES)
	rm combined.c
	python3 combine.py $^
