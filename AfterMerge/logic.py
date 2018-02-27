import dal
import ctypes

_raft = ctypes.CDLL("./libraft.so")

_raft.edit_cmd.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
_raft.delete_cmd.argtypes = [ctypes.c_char_p]
_raft.show_node_status.argtypes = []
_raft.show_system_status.argtypes = []
_raft.run.argtypes = []


callback_type_edit = ctypes.CFUNCTYPE(ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p)
callback_type_delete = ctypes.CFUNCTYPE(ctypes.c_void_p, ctypes.c_char_p)


def initToC():
    _raft.run(callback_func_delete, callback_func_edit)


def editValToC(key, value):
    print("Logic: edit value of {0} to be {1}".format(key, value))
    if _raft.edit_cmd(ctypes.c_char_p(key), ctypes.c_char_p(value)) == -1:
        return -1


def deleteValToC(key):
    print("Logic: delete {0}".format(key))
    if _raft.delete_cmd(ctypes.c_char_p(key)) == -1:
        return -1


def editValToDal(key, value):
    dal.editValToDB(key, value)


callback_func_edit = callback_type_edit(editValToDal)


def deleteValToDal(key):
    dal.deleteValfromDB(key)


callback_func_delete = callback_type_delete(deleteValToDal)

def showNodeStatus():
    nodeStatus = _raft.show_node_status()
    print("The node status are: ")
    if nodeStatus == 0:
        print("Follower")
    elif nodeStatus == 1:
        print("Candidate")
    else:
        print("Leader")

def showSysStatus():
    pass

def showLog():
    pass