import tkinter as tk
from tkinter import *
from tkinter import filedialog
from tkinter import scrolledtext
from threading import *
import os

words = ["poesía","vida","cielo","amistad","amor","sentimiento","dulce","novio","novia","romance","beso","caricia","abrazo","nada","odio","ilusión","regalo","emoción","alegría","felicidad","cariño"]

def lower_case(text_string):
    return text_string.lower()
    
def tokenize_text(text_string):
    from nltk.tokenize import word_tokenize
    return word_tokenize(text_string)
    
def only_words(text_list):
    import re
    new_tokens = []
    for token in text_list:
        clean_token = ""
        for c in token:
            if re.match(r'[a-záéíóúñü]', c):
                clean_token += c
        if clean_token != '':
            new_tokens.append(clean_token)
    return new_tokens

def thread_work(filename, results, i):

    f = open(filename, encoding='utf-8')
    raw_text = f.read()

    # Make the text lowercase
    clean_text = lower_case(raw_text)

    # Tokenizing the text
    text_list = tokenize_text(clean_text)

    # Getting rid of urls
    #$text_list = delete_urls(text_list)

    # Getting rid of punctuation
    text_list = only_words(text_list)
    total_words = len(text_list)

    res_str = "Thread: " +  str(current_thread().ident) + "\n"
    res_str += "File: " + os.path.basename(filename) + ", Total words: " + str(total_words) + "\n"

    for w in words:
        c = text_list.count(w)
        res_str += "Word: " + w + ", Count: " + str(c) + ", Percent: " + str((c / total_words)*100) + "%" + "\n"
    res_str += "\n\n"
    results[i] = res_str

# Method called when selecting file button 
def file_clicked():
    # Getting the file path and name
    filename = filedialog.askopenfilename()

    # Thread
    results = [""]

    thread = Thread(target=thread_work, args=(filename, results, 0))
    thread.start()
    thread.join()

    # Creating new window for showing results
    t = Toplevel(window)
    # Frame for the text
    frame = tk.Frame(t, width=100)
    frame.grid(row=2, column=0, columnspan=50, rowspan=70)

    # Scrollable text area
    edit_space = scrolledtext.ScrolledText(
        master=frame,
        wrap='word',  # wrap text at full words only
        width=60,      # characters
        height=50      # text lines
         # background color of edit area
    )
    # Showing the text area
    edit_space.pack(fill='both', expand=True, padx=8, pady=8)

    # Writing to the text area
    edit_space.insert('insert', (results[0] + "\n"))
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
    frame.grid(row=2, column=0, columnspan=50, rowspan=70)

    # Scrollable text area
    edit_space = scrolledtext.ScrolledText(
        master=frame,
        wrap='word',  # wrap text at full words only
        width=60,      # characters
        height=50      # text lines
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