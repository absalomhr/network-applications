import socket
import traceback
from threading import Thread
import os
from os import listdir
from os.path import isfile, join
import math
import pickle

# Constants
# IP = "127.0.0.1"
IP = "192.168.43.96"
PORT = 8080
BUFFER_SIZE = 1024
PATH = "./drive/"
CONFIRM = 0
DELETE = 1
CREATE = 2
INIT = 3

# Globals
n_cons = 0 # connections received
con_dict = {}

# Functions
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
    return

def create_file(c, addr):
    c.send(str(CONFIRM).encode('utf8'))
    fname = c.recv(BUFFER_SIZE).decode("utf8")
    s_files = [f for f in listdir(PATH) if isfile(join(PATH, f))]
    if fname in s_files:
        os.remove(PATH+fname)
    c.send(str(CONFIRM).encode('utf8'))
    receive_file(fname, c)
    # Sending petition to other clients
    for k in con_dict.keys():
        if k == addr:
            continue
        c2 = con_dict[k]
        c2.send(str(CREATE).encode('utf8'))
        int(c2.recv(BUFFER_SIZE).decode("utf8"))
        c2.send(fname.encode('utf8'))
        int(c2.recv(BUFFER_SIZE).decode("utf8"))
        send_file(fname, c2)
    return

def delete_file(c, addr):
    c.send(str(CONFIRM).encode('utf8'))
    fname = c.recv(BUFFER_SIZE).decode("utf8")
    os.remove(PATH+fname)
    c.send(str(CONFIRM).encode('utf8'))
    # Sending petition to other clients
    for k in con_dict.keys():
        if k == addr:
            continue
        c2 = con_dict[k]
        c2.send(str(DELETE).encode('utf8'))
        int(c2.recv(BUFFER_SIZE).decode("utf8"))
        c2.send(fname.encode('utf8'))
        int(c2.recv(BUFFER_SIZE).decode("utf8"))
    return

def listen_to_client(c, addr):
    while True:
        opt = int(c.recv(BUFFER_SIZE).decode("utf8"))
        if opt == CREATE:
            create_file(c, addr)
        elif opt == DELETE:
            delete_file(c, addr)

def init_server(c):
    int(c.recv(BUFFER_SIZE).decode("utf8"))
    data = c.recv(BUFFER_SIZE)
    c_files = pickle.loads(data)
    s_files = [f for f in listdir(PATH) if isfile(join(PATH, f))]
    files_to_send = []
    for  f in s_files:
        if f not in c_files:
            files_to_send.append(f)
    n = len(files_to_send)
    c.send(str(n).encode('utf8'))
    if n == 0:
        #print("Zero files")
        return
    for i in range(n):
        int(c.recv(BUFFER_SIZE).decode("utf8"))
        fname = files_to_send[i]
        c.send(fname.encode("utf8"))
        send_file(fname, c)


def client_thread (c, addr):
    global n_cons
    cport = n_cons + PORT
    c.send(str(cport).encode('utf8'))
    opt = int(c.recv(BUFFER_SIZE).decode("utf8"))
    if opt == CONFIRM:
        # Connecting to the client again
        c_rec = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print(addr)
        c_rec.connect((addr[0], cport))
        con_dict[addr] = c_rec
        c.send(str(INIT).encode('utf8'))
        init_server(c)
        listen_to_client(c, addr)


# Creating the server socket and accepting connections
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
print("Server socket created")
s.bind((IP, PORT))
print("Binding done")
s.listen(3)
print("Listening...")
while True: 
    c, addr = s.accept()
    print("Connection received", addr)
    n_cons += 1
    # connection received, starting a thread
    try:
        Thread(target=client_thread, args=(c, addr)).start()
    except:
        print("Error creating thread")
        traceback.print_exc()
        break
# closing the server socket       
s.close()