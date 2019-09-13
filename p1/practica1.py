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

def thread_work(filename, results, index):
    # print(index)
    results[index] = []

    f = open(filename, encoding='latin-1')
    raw_text = f.read()

    # Make the text lowercase
    clean_text = lower_case(raw_text)

    # Tokenizing the text
    text_list = tokenize_text(clean_text)

    # Getting rid of punctuation
    text_list = only_words(text_list)
    total_words = len(text_list)

    for w in words:
        results[index].append(str(text_list.count(w)))

    results[index].append(str(get_ident()))
    results[index].append(os.path.basename(filename))
    results[index].append(str(total_words))


# Method called when selecting file button 
def file_clicked():
    # Getting the file path and name
    filename = filedialog.askopenfilename()

    if type(filename) != type(""):
        return False

    # Thread
    results = [[]]

    thread = Thread(target=thread_work, args=(filename, results, 0))
    thread.start()
    thread.join()

    file_res = ""
    summary = ""
    total_words_all = 0
    total_counts = [0] * len(words)

    
    file_res += "Thread: " + results[0][-3] + "\n"
    file_res += "File: " + results[0][-2] + "\n"
    file_res += "Words count: " + results[0][-1] + "\n"
    total_words_all += int(results[0][-1])

    for j in range(len(words)):
        file_res += words[j] + ": " + results[0][j] + "\n"
        total_counts[j] += int(results[0][j])
    file_res += "\n\n\n"

    summary = "Total number of words: " + str(total_words_all) + "\n"
    for j in range(len(words)):
        if total_counts[j] == 0:
            summary += words[j] + ": " + "{0:.1f}".format((total_counts[j] / total_words_all) * 100) + "%" +"\n"
        else:
            summary += words[j] + ": " + "{0:.3f}".format((total_counts[j] / total_words_all) * 100) + "%" +"\n"


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
    edit_space.insert('insert', (file_res+summary))
    # Disabling the text area so the user cant edit the text
    edit_space.config(state="disabled")
    t.mainloop()

# Method called when selecting directory button
def dir_clicked():
    import glob
    import os

    # Getting the dir path and name
    dirname = filedialog.askdirectory()

    if dirname == '':
        return False
    
    os.chdir(r''+dirname+'')
    myFiles = glob.glob('*.txt')

    n_files = len(myFiles)

    if (n_files == 0):
        return False

    results = [[]] * n_files
    threads = []

    for i in range(n_files):
        threads.append(Thread(target=thread_work, args=(myFiles[i], results, i)))

    for i in range(n_files):
        threads[i].start()

    for i in range(n_files):
        threads[i].join()


    # Results is a list of lists
    # Each list has the following structure:

    #[each_word_count..., thread_name, file_name, file_size]

    file_res = ""
    summary = ""
    total_words_all = 0
    total_counts = [0] * len(words)

    for i in range(n_files):
        file_res += "Thread: " + results[i][-3] + "\n"
        file_res += "File: " + results[i][-2] + "\n"
        file_res += "Words count: " + results[i][-1] + "\n"
        total_words_all += int(results[i][-1])

        for j in range(len(words)):
            file_res += words[j] + ": " + results[i][j] + "\n"
            total_counts[j] += int(results[i][j])
        file_res += "\n\n\n"

    summary = "Total number of words: " + str(total_words_all) + "\n"
    for j in range(len(words)):
        if total_counts[j] == 0:
            summary += words[j] + ": " + "{0:.1f}".format((total_counts[j] / total_words_all) * 100) + "%" +"\n"
        else:
            summary += words[j] + ": " + "{0:.3f}".format((total_counts[j] / total_words_all) * 100) + "%" +"\n"

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
    edit_space.insert('insert', file_res + summary)
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