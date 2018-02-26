import dal
import ctypes
_raft = ctypes.CDLL("./libraft.so")

_raft.edit_cmd.argtypes = (ctypes.c_char_p, ctypes.c_char_p)
_raft.delete_cmd.argtypes = (ctypes.c_int, ctypes.c_char_p)
# TODO - need to add all the c functions prototypes

callback_type_edit = ctypes.CFUNCTYPE(ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p)
callback_type_delete = ctypes.CFUNCTYPE(ctypes.c_void_p, ctypes.c_char_p)


def editValToC(key, value):
    print("edit val from the logic")
    if _raft.edit_cmd(ctypes.c_char_p(key), ctypes.c_char_p(value), callback_func_edit) == -1:
        return -1


def deleteValToC(key):
    print("delete val from the logic")
    if _raft.delete_cmd(ctypes.c_char_p(key), callback_func_delete) == -1:
        return -1


def editValToDal(key, value):
    dal.editValToDB(key, value)


callback_func_edit = callback_type_edit(editValToDal)


def deleteValToDal(key):
    dal.deleteValfromDB(key)


callback_func_delete = callback_type_delete(deleteValToDal)
