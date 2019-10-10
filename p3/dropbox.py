import pyinotify

class EventHandler(pyinotify.ProcessEvent):
    def __init__(self):
        self.creando = False
        self.arch_creando = ""

    def process_IN_CREATE(self, event):
        print ("Creating:", event.pathname)
        self.creando = True

    def process_IN_MOVED_TO(self, event):
        print ("Creating:", event.pathname)
        print ("Mandando creacion:", event.pathname)

    def process_IN_MOVED_FROM(self, event):
        print ("Deleting:", event.pathname)

    def process_IN_DELETE(self, event):
        print ("Deleting:", event.pathname)

    def process_IN_CLOSE_WRITE(self, event):
        if self.creando:
            print ("Mandando creación:", event.pathname)
            return
        print("Mandando modificacion")

# Instanciate a new WatchManager (will be used to store watches).
wm = pyinotify.WatchManager()

flags = pyinotify.IN_CREATE | pyinotify.IN_MOVED_TO | pyinotify.IN_MOVED_FROM | pyinotify.IN_DELETE | pyinotify.IN_CLOSE_WRITE

handler = EventHandler()

# Associate this WatchManager with a Notifier (will be used to report and
# process events).
notifier = pyinotify.Notifier(wm, handler)

# Add a new watch on 'drive' for the required events
wdd = wm.add_watch('./drive', flags, rec=False)

# Loop forever and handle events.
notifier.loop()

# Crear
# IN_CREATE (touch, copiar y pegar, crear desde sublime)
# IN_MOVED_TO (mover de una carpeta al 'drive')
# Eliminar
# IN_MOVED_FROM (Supr, arrastrar a otra carpeta)
# IN_DELETE (eliminar desde consola)
# Modificar
# IN_CLOSE_WRITE (desde sublime)