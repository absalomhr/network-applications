import socket
import pickle
from os import listdir
from os.path import isfile, join
import os
import pyinotify
import traceback
from threading import Thread
import math
import time
import traceback

S_IP = "127.0.0.1"
S_PORT = 1234
BUFFER_SIZE = 1024
PATH = "./client"
CONFIRM = 10
EOF = 15
DELETE = 20
CREATE = 25

# Crear
# IN_CREATE (touch, copiar y pegar, crear desde sublime)
# IN_MOVED_TO (mover de una carpeta al 'drive')
# Eliminar
# IN_MOVED_FROM (Supr, arrastrar a otra carpeta)
# IN_DELETE (eliminar desde consola)
# Modificar
# IN_CLOSE_WRITE (desde sublime)

class EventHandler(pyinotify.ProcessEvent):
    def __init__(self, s):
        self.creating = False
        self.s = s

    def process_IN_CREATE(self, event):
        print ("Creating:", event.name)
        self.creating = True

    def process_IN_MOVED_TO(self, event):
        print ("Creating:", event.name)
        self.creating = True
        self.process_IN_CLOSE_WRITE(event)

    def process_IN_MOVED_FROM(self, event):
        print("Delete: moved from")
        self.s.send(str(DELETE).encode('utf8'))
        int(self.s.recv(BUFFER_SIZE).decode("utf8"))
        fname = event.name
        print(fname)
        self.s.send(fname.encode('utf8'))
        int(self.s.recv(BUFFER_SIZE).decode("utf8"))
        print("Delete", fname)
        return

    def process_IN_DELETE(self, event):
        print("Delete: in delete")
        self.s.send(str(DELETE).encode('utf8'))
        int(self.s.recv(BUFFER_SIZE).decode("utf8"))
        fname = event.name
        print(fname)
        self.s.send(fname.encode('utf8'))
        int(self.s.recv(BUFFER_SIZE).decode("utf8"))
        print("Delete", fname)
        return

    def process_IN_CLOSE_WRITE(self, event):
        time.sleep(1)
        if self.creating:
            print ("sending new file:", event.name)
        print ("updating file:", event.name)
        self.s.send(str(CREATE).encode('utf8'))
        int(self.s.recv(BUFFER_SIZE).decode("utf8"))
        fname = event.name
        print(fname)
        self.s.send(fname.encode('utf8'))
        int(self.s.recv(BUFFER_SIZE).decode("utf8"))
        # sending file
        n_bytes = os.path.getsize(PATH+"/"+fname)
        n_chunks = n_bytes / BUFFER_SIZE
        n_chunks = math.ceil(n_chunks)
        f = open(PATH+"/"+fname, "rb")
        self.s.send(str(n_chunks).encode('utf8'))
        int(self.s.recv(BUFFER_SIZE).decode("utf8"))
        for i in range(n_chunks):
            data = f.read(BUFFER_SIZE)
            self.s.send(data)
            int(self.s.recv(BUFFER_SIZE).decode("utf8"))
        f.close()
        self.creating = False
        print("file sent", fname)
        return

def thread_notify(s):
    print("Creating notify")
    # Instanciate a new WatchManager (will be used to store watches).
    wm = pyinotify.WatchManager()

    flags = pyinotify.IN_CREATE | pyinotify.IN_MOVED_TO | pyinotify.IN_MOVED_FROM | pyinotify.IN_DELETE | pyinotify.IN_CLOSE_WRITE

    handler = EventHandler(s)

    # Associate this WatchManager with a Notifier (will be used to report and
    # process events).
    notifier = pyinotify.Notifier(wm, handler)

    # Add a new watch on 'drive' for the required events
    wdd = wm.add_watch(PATH, flags, rec=False)

    # Loop forever and handle events.
    notifier.loop()

def init (s):
    c_files = [f for f in listdir(PATH) if isfile(join(PATH, f))]
    data=pickle.dumps(c_files)
    s.send(data)
    n = int(s.recv(BUFFER_SIZE).decode("utf8"))
    if n == 0:
        print("Zero files")
        return
    print("Expecting", n, "files")
    for i in range(n):
        s.send(str(CONFIRM).encode('utf8'))
        name = s.recv(BUFFER_SIZE).decode("utf8")
        print(name)
        f = open(PATH+"/"+name, 'wb')
        s.send(str(CONFIRM).encode('utf8'))
        n_chunks = int(s.recv(BUFFER_SIZE).decode("utf8"))
        s.send(str(CONFIRM).encode('utf8'))
        #print(name, n_chunks)
        for i in range(n_chunks):
            data = s.recv(BUFFER_SIZE)
            f.write(data)
            s.send(str(CONFIRM).encode('utf8'))
        f.close()
    return

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((S_IP, S_PORT))

cs = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
cs.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
print("Extra socket created")
port2 = S_PORT +1
while True:
    try:
        cs.bind((S_IP, port2))
    except:
        print("Couldnt bind extra socket", S_IP, port2)
        traceback.print_exc()
        port2 += 1
        continue
# sendig extra port
s.send(str(port2).encode('utf8'))
int(s.recv(BUFFER_SIZE).decode("utf8"))
cs.listen(1)
conn2, addr2 = cs.accept()

opt = int(s.recv(BUFFER_SIZE).decode("utf8"))
if opt == 0:
    init(s)

try:
    Thread(target=thread_notify, args=[s]).start()
except:
    print("Error creating thread")
    traceback.print_exc()

print("server connected back", addr2)


# cs.listen(3)
# print("Listening...")
# while True: 
#     conn, addr = s.accept()
#     # connection received, starting a thread
#     try:
#         Thread(target=client_thread, args=(conn, addr)).start()
#     except:
#         print("Error creating thread")
#         traceback.print_exc()
#         break
#s.close()