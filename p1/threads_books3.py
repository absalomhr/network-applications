import tkinter as tk
from tkinter import *
from tkinter import filedialog
from tkinter import scrolledtext

# Method called when selecting file button 
def file_clicked():
    # Getting the file path and name
    filename = filedialog.askopenfilename()

    # Creating new window for showing results
    t = Toplevel(window)
    # Frame for the text
    frame = tk.Frame(t, width=100)
    frame.grid(row=2, column=0, columnspan=30, rowspan=30)

    # Scrollable text area
    edit_space = scrolledtext.ScrolledText(
        master=frame,
        wrap='word',  # wrap text at full words only
        width=45,      # characters
        height=10,      # text lines
         # background color of edit area
    )
    # Showing the text area
    edit_space.pack(fill='both', expand=True, padx=8, pady=8)
    # Inserting the text to the text area

    # Hilos van aqui we
    #result = resultado del analisis en forma de cadena a huevo 

    # Writing to the text area
    edit_space.insert('insert', filename + "\n")
    # Disabling the text area so the user cant edit the text
    edit_space.config(state="disabled")
    t.mainloop()

# Method called when selecting directory button
def dir_clicked():
    # Getting the dir path and name
    dirname = filedialog.askdirectory()
    # Creating new window for showing results
    t = Toplevel(window)
    # Frame for the text
    frame = tk.Frame(t, width=100)
    frame.grid(row=2, column=0, columnspan=30, rowspan=30)

    # Scrollable text area
    edit_space = scrolledtext.ScrolledText(
        master=frame,
        wrap='word',  # wrap text at full words only
        width=45,      # characters
        height=10,      # text lines
         # background color of edit area
    )
    # Showing the text area
    edit_space.pack(fill='both', expand=True, padx=8, pady=8)
    # Inserting the text to the text area

    # Hilos van aqui we
    #result = resultado del analisis en forma de cadena a huevo 

    # Writing to the text area
    edit_space.insert('insert', dirname + "\n")
    # Disabling the text area so the user cant edit the text
    edit_space.config(state="disabled")
    t.mainloop()

# Main window
window = Tk()
 
window.title("Text analyzer")
window.geometry('550x50')
# Greeting label
lbl = Label(window, text="Select an option to begin the analysis")
lbl.grid(column=0, row=0)

# File button
btn = Button(window, text="Select a file", bg="#60a561", command=file_clicked)
# Directory button
btn2 = Button(window, text="Select a directory", bg="#60a561", command=dir_clicked)

# Put the widgets on the window
btn.grid(column=0, row=1)
btn2.grid(column=1, row=1)

window.mainloop()