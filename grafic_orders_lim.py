import numpy as np
import random as rd
import time as time_modul
import time
from matplotlib.patches import Rectangle
import os
import matplotlib.pyplot as plt



plt.ion()
fig, ax = plt.subplots()
ax.set_xlim(0, 45) #размеры графика
ax.set_ylim(50, 150)
while True:
    while True:
        time.sleep(0.01)
        if os.path.getsize("to_lim_order_book.txt") != 0:
            time.sleep(0.01)
            break

    
    try:
        with open("to_lim_order_book.txt","r") as file:
            data = file.readlines()
            if  "x00" in data[0]:
                continue
        with open("to_lim_order_book.txt", "w") as file:
            pass 
    except PermissionError:
        print("Не удалось прочитать continious")
    buyers = {}
    sellers = {}

    for i in data:
        typ = i[0]
        p, q = map(float, i[1:].split())
        p = int(p * 100)
        if typ == "b":
            buyers[p] = buyers.get(p, 0) + q
        else:
            sellers[p] = sellers.get(p, 0) + q
    print(len(buyers), len(sellers))
    for patch in ax.patches[:]:
        patch.remove()
    
    for id in buyers:
        p = id / 100
        q = buyers[id]
        rect = Rectangle(
            xy= (0, p),
            width = q,
            height = 0.2,
            color = "green",
            alpha = 0.7
        )
        ax.add_patch(rect)

    for id in sellers:
        p = id / 100
        q = sellers[id]
        rect = Rectangle(
            xy= (0, p),
            width = q,
            height = 0.2,
            color = "red",
            alpha = 0.7
        )
        ax.add_patch(rect)
    

    fig.canvas.draw()
    fig.canvas.flush_events()



