import socket
import traceback
from threading import Thread
import os
import pyinotify
import time
import math
from os import listdir
from os.path import isfile, join
import pickle

# Constants
IP = "127.0.0.1"
PORT = 1234
BUFFER_SIZE = 1024
PATH = "./client2/"
CONFIRM = 0
DELETE = 1
CREATE = 2
INIT = 3

# Globals
server_action = False

# Dir listener object
# Create
# IN_CREATE (touch, copy and paste, creating from sublime)
# IN_MOVED_TO (move from a dir to client dir)
# Delete
# IN_MOVED_FROM (Supr, drag to another dir)
# IN_DELETE (delete from shell)
# Modify
# IN_CLOSE_WRITE (Modify from sublime)

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
        self.process_IN_DELETE(event)
        return

    def process_IN_DELETE(self, event):
        global server_action
        if server_action:
            print("Ignoring incoming action")
            return
        fname = event.name
        print("Deleting", fname)
        self.s.send(str(DELETE).encode('utf8'))
        int(self.s.recv(BUFFER_SIZE).decode("utf8"))
        self.s.send(fname.encode('utf8'))
        int(self.s.recv(BUFFER_SIZE).decode("utf8"))
        print("Delete complete", fname)
        return

    def process_IN_CLOSE_WRITE(self, event):
        global server_action
        if server_action:
            print("Ignoring incoming action")
            return
        fname = event.name
        if self.creating:
            print ("Sending new file:", fname)
        else:
            print ("Updating file:", fname)
        self.s.send(str(CREATE).encode('utf8'))
        int(self.s.recv(BUFFER_SIZE).decode("utf8"))
        self.s.send(fname.encode('utf8'))
        int(self.s.recv(BUFFER_SIZE).decode("utf8"))
        # sending file
        send_file(fname, self.s)
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

def delete_file_from_server(c):
    c.send(str(CONFIRM).encode('utf8'))
    fname = c.recv(BUFFER_SIZE).decode("utf8")
    os.remove(PATH+fname)
    c.send(str(CONFIRM).encode('utf8'))
    global server_action
    print("delete from server complete", fname)
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
    print("created file from server", fname)
    server_action = False
    return

# Functions
def listen_to_server(c_send, addr):
    global server_action
    while True:
        opt = int(c_send.recv(BUFFER_SIZE).decode("utf8"))
        if opt == CREATE:
            server_action = True
            print("creating from server")
            create_file_from_server(c_send)
        elif opt == DELETE:
            server_action = True
            print("deleting from server")
            delete_file_from_server(c_send)

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
mys.bind((IP, myport))
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
    Thread(target=thread_notify, args=[s]).start()
except:
    print("Error creating thread")
    traceback.print_exc()

listen_to_server(c_send, addr)