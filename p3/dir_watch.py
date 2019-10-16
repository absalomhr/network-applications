import os # File management in the OS

# Constants
PATH = "./client/"

# Files right now
current = [f for f in os.listdir(PATH) if os.path.isfile(os.path.join(PATH, f))]
# Files in the past
past = current.copy()
# Files and modification dates now
fdates_c = [(fname, os.path.getmtime(PATH+fname)) for fname in current]
# Files and modification in the past
fdates_p = [(fname, os.path.getmtime(PATH+fname)) for fname in past]
# Files to modify
to_modify = []
# Files to create, initialized with current files on first iteration
to_create = []
# Files to delete
to_delete = []
while True:
    current = [f for f in os.listdir(PATH) if os.path.isfile(os.path.join(PATH, f))]
    to_delete = [f for f in past if f not in current] # past - current
    to_create = [f for f in current if f not in past] # current - past
    fdates_c = [(fname, os.path.getmtime(PATH+fname)) for fname in current]
    to_modify = [t for t in fdates_c if t not in fdates_p]
    if to_delete:
        print("del")
        print(to_delete)
    elif to_create:
        print("crea")
        print(to_create)
    elif to_modify:
        print("modify")
        print(to_modify)
    past = current.copy()
    fdates_p = fdates_c.copy()