################################################################################
# Util functions

KIB = 1024
MIB = 1024 * KIB
GIB = 1024 * MIB

def bytes2str(nbytes):
    if nbytes >= GIB:
        return "%dGiB" % (nbytes / GIB)
    elif nbytes >= MIB:
        return "%dMiB" % (nbytes / MIB)
    elif nbytes >= KIB:
        return "%dKiB" % (nbytes / KIB)
    else:
        return "%dB" % nbytes

def str2bytes(s):
    if s.endswith("GiB"):
        return int(s[:-3]) * GIB
    elif s.endswith("MiB"):
        return int(s[:-3]) * MIB
    elif s.endswith("KiB"):
        return int(s[:-3]) * KIB
    else:
        return int(s)
