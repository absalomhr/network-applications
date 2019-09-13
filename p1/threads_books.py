from tkinter import filedialog
dirname = filedialog.askdirectory()
import glob
import os


os.chdir(r''+dirname+'')
myFiles = glob.glob('*.txt')
print(myFiles)