import os
import traceback
from os import listdir
from os.path import isfile, join
import pickle
import socket
from threading import Thread
import math

# Constants
IP = "127.0.0.1"
MYIP = "127.0.0.1"
PORT = 8080
BUFFER_SIZE = 1024
PATH = "./client/"
CONFIRM = 0
DELETE = 1
CREATE = 2
INIT = 3

# Functions
def listen_to_server(c_send, addr):
    global server_action
    while True:
        opt = int(c_send.recv(BUFFER_SIZE).decode("utf8"))
        if opt == CREATE:
            server_action = True
            #print("creating from server")
            create_file_from_server(c_send)
        elif opt == DELETE:
            server_action = True
            #print("deleting from server")
            delete_file_from_server(c_send)

def delete_file_from_server(c):
    c.send(str(CONFIRM).encode('utf8'))
    fname = c.recv(BUFFER_SIZE).decode("utf8")
    os.remove(PATH+fname)
    c.send(str(CONFIRM).encode('utf8'))
    global server_action
    #print("delete from server complete", fname)
    server_action = False
    return

def create_file_from_server(c):
    c.send(str(CONFIRM).encode('utf8'))
    fname = c.recv(BUFFER_SIZE).decode("utf8")
    c_files = [f for f in listdir(PATH) if isfile(join(PATH, f))]
    if fname in c_files:
        os.remove(PATH+fname)
    c.send(str(CONFIRM).encode('utf8'))
    receive_file(fname, c)
    global server_action
    #print("created file from server", fname)
    server_action = False
    return

def receive_file(fname, s):
    f = open(PATH+fname, 'wb')
    n_chunks = int(s.recv(BUFFER_SIZE).decode("utf8"))
    s.send(str(CONFIRM).encode('utf8'))
    #print(name, n_chunks)
    for i in range(n_chunks):
        data = s.recv(BUFFER_SIZE)
        f.write(data)
        s.send(str(CONFIRM).encode('utf8'))
    f.close()

def send_file(fname, s):
    n_bytes = os.path.getsize(PATH+fname)
    n_chunks = n_bytes / BUFFER_SIZE
    n_chunks = math.ceil(n_chunks)
    f = open(PATH+fname, "rb")
    s.send(str(n_chunks).encode('utf8'))
    int(s.recv(BUFFER_SIZE).decode("utf8"))
    for i in range(n_chunks):
        data = f.read(BUFFER_SIZE)
        s.send(data)
        int(s.recv(BUFFER_SIZE).decode("utf8"))
    f.close()

def delete_file(fname, s):
    os.remove(PATH+fname)
    s.send(str(DELETE).encode('utf8'))
    s.send(fname.encode('utf8'))
    int(s.recv(BUFFER_SIZE).decode("utf8"))
    return

def create_file(fname):
    f = open(PATH+fname, "w")
    f.close()
    return

def push(fname, s):
    s.send(str(CREATE).encode('utf8'))
    int(s.recv(BUFFER_SIZE).decode("utf8"))
    s.send(fname.encode('utf8'))
    int(s.recv(BUFFER_SIZE).decode("utf8"))
    send_file(fname, s)
    return

def mod_file(fname):
    osCommandString = "subl " + PATH+fname
    os.system(osCommandString)
    return

def client_terminal(s):
    file_to_push = ""
    file_pending = False
    while True:
        cmd = input("Expecting command:\n")
        cmd = cmd.split(' ')
        n = len(cmd)

        if file_pending and cmd[0] != "push":
            print("File: ", file_to_push, " its pending")
            continue
        if cmd[0] == "push":
            if not file_pending:
                print("No file pending\n")
                continue
            else:
                push(file_to_push, s)
                file_pending = False
                file_to_push = ""
        elif cmd[0] == "del":
            if n < 2:
                print("Command del receives 1 or more arguments\n")
                continue
            for i in range(1, n):
                delete_file(cmd[i], s)                
        elif cmd[0] == "new":
            if n < 2:
                print("Command new receives 1 or more arguments\n")
                continue
            create_file(cmd[1])
            push(cmd[1], s)
        elif cmd[0] == "mod":
            if n < 2:
                print("Command mod receives 1 or more arguments\n")
            file_to_push = cmd[1]
            file_pending = True
            mod_file(cmd[1])
            
def init_client(s):
    s.send(str(CONFIRM).encode('utf8'))
    c_files = [f for f in listdir(PATH) if isfile(join(PATH, f))]
    data=pickle.dumps(c_files)
    s.send(data)
    n = int(s.recv(BUFFER_SIZE).decode("utf8"))
    if n == 0:
        return
    print("Expecting", n, "files from server")
    for i in range(n):
        s.send(str(CONFIRM).encode('utf8'))
        fname = s.recv(BUFFER_SIZE).decode("utf8")
        receive_file(fname, s)
    print("Files received from server")

# Connecting to the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((IP, PORT))

# receiving port number for bidirectional connection
myport = int(s.recv(BUFFER_SIZE).decode("utf8"))
print("Port received", myport)
mys = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
mys.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
print("My server socket created")
mys.bind((MYIP, myport))
print("Binding done")
mys.listen(3)
print("Listening...")
s.send(str(CONFIRM).encode('utf8'))
c_send, addr = mys.accept()
print("Server connection received", addr)
opt = int(s.recv(BUFFER_SIZE).decode("utf8"))
if opt == INIT:
    init_client(s)
# listening to changes in clients dir
try:
    Thread(target=client_terminal, args=[s]).start()
except:
    print("Error creating thread")
    traceback.print_exc()

listen_to_server(c_send, addr)