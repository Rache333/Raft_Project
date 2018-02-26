import ctypes
_raft = ctypes.CDLL("./libraft.so")

#_raft.add_cmd.argtypes = (ctypes.c_int, ctypes.c_char_p, ctypes.c_char_p)
_raft.edit_cmd.argtypes = (ctypes.c_char_p, ctypes.c_char_p)
#_raft.delete_cmd.argtypes = (ctypes.c_int, ctypes.c_char_p)
# TODO - need to add all the c functions prototypes

#TODO - chenge the callback to the right function from c
callback_type = ctypes.CFUNCTYPE(ctypes.c_void_p)

def addval():
    print("Call back Func!! going back to c")
    # if _raft.edit_cmd(ctypes.c_char_p(key), ctypes.c_char_p(value)) == -1:
    #     return -1


callback_func = callback_type(addval)

def editval(key, value):
    #print("edit val from the logic")
    if _raft.edit_cmd(ctypes.c_char_p(key), ctypes.c_char_p(value), callback_func) == -1:
        return -1


def deleteval(key):
    #print("delete val from the logic")
    if _raft.delete_cmd(ctypes.c_char_p(key)) == -1:
        return -1

