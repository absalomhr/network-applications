import socket
import traceback
from threading import Thread
import time
import pickle
from os import listdir
from os.path import isfile, join
import os
import math

# Constants
IP = "127.0.0.1"
PORT = 1234
BUFFER_SIZE = 1024
PATH = "./drive"
CONFIRM = 10
EOF = 15
DELETE = 20
PAUSE = 25

def init (c):
    opt = 0
    c.send(str(opt).encode('utf8'))
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
        print("Zero files")
        return
    for i in range(n):
        int(c.recv(BUFFER_SIZE).decode("utf8"))
        c.send(files_to_send[i].encode('utf8'))
        f = open(PATH+"/"+files_to_send[i], "rb")
        int(c.recv(BUFFER_SIZE).decode("utf8"))
        n_bytes = os.path.getsize(PATH+"/"+files_to_send[i])
        n_chunks = n_bytes / BUFFER_SIZE
        n_chunks = math.ceil(n_chunks)
        #print(n_bytes, n_chunks)
        c.send(str(n_chunks).encode('utf8'))
        int(c.recv(BUFFER_SIZE).decode("utf8"))
        for i in range(n_chunks):
            data = f.read(BUFFER_SIZE)
            c.send(data)
            int(c.recv(BUFFER_SIZE).decode("utf8"))
        f.close()
    return

def delete_file(c):
    c.send(str(CONFIRM).encode('utf8'))
    fname = c.recv(BUFFER_SIZE).decode("utf8")
    os.remove(PATH+"/"+fname)
    c.send(str(CONFIRM).encode('utf8'))
    return

def client_thread(conn, addr):
    ip = str(addr[0]); port = str(addr[1])
    print("CLIENT:", ip, port)
    init(conn)
    while True:
        opt = int(conn.recv(BUFFER_SIZE).decode("utf8"))
        if opt == DELETE:
            delete_file(conn)
    return

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
print("Server socket created")
s.bind((IP, PORT))
print("Binding done")
s.listen(3)
print("Listening...")
while True: 
    conn, addr = s.accept()
    # connection received, starting a thread
    try:
        Thread(target=client_thread, args=(conn, addr)).start()
    except:
        print("Error creating thread")
        traceback.print_exc()
        break
# closing the server socket       
s.close()