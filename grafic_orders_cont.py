import matplotlib.pyplot as plt
import numpy as np
import random as rd
import time as time_modul
import time
from matplotlib.patches import Rectangle
import os


plt.ion()
fig, ax = plt.subplots()


class buyer:
    def __init__(self, p_l, p_h, u_max):
        self.p_l = p_l
        self.p_h = p_h
        self.u_max = u_max
    def u_demand(self, p):
        if p >= self.p_h:
            return 0
        if p <= self.p_l:
            return self.u_max
        u = self.u_max * (self.p_h - p) / (self.p_h - self.p_l) #считаем объем по специальной формуле, которая должна быть линейна на участке, где мы что-то покупаем
        return u
    
class seller:
    def __init__(self, p_l, p_h, u_max):
        self.p_l = p_l
        self.p_h = p_h
        self.u_max = u_max
    def u_supply(self, p):
        if p <= self.p_l:
            return 0
        if p >= self.p_h:
            return self.u_max 
        u = self.u_max * (p - self.p_l) / (self.p_h - self.p_l)
        return u
    

buyers = {} #список покупателей
sellers = {} #список продавцов


while True:
    while True:
        time.sleep(0.01)
        if os.path.getsize("to_cont_order_book.txt") != 0:
            break

    
    try:
        with open("to_cont_order_book.txt","r") as file:
            data = file.readlines()
            if  "x00" in data[0]:
                continue
        with open("to_cont_order_book.txt", "w") as file:
            pass 
    except PermissionError:
        print("Не удалось прочитать continious")
        
    id_order = 0; 
    p_min = 999999
    p_max = 0

    for i in data:
        typ = i[0]
        p_l, p_h, u_max = map(float, i[1:].split())
        p_min = min(p_min, p_l)
        p_max = max(p_max, p_h)
        if typ == "b":
            buyers[id_order] = buyer(p_l=p_l, p_h=p_h, u_max=u_max)
        if typ == "s":
            sellers[id_order] = seller(p_l=p_l, p_h=p_h, u_max=u_max)

        id_order += 1


    demand = [] 
    supply = []
    prices1 = []
    for price100 in range(int(p_min * 0.95) * 50, int(p_max * 1.05) * 50):
        price1 = price100 / 50
        demand1 = 0
        for id in buyers:
            demand1 += buyers[id].u_demand(price1)
        supply1 = 0
        for id in sellers:
            supply1 += sellers[id].u_supply(price1)
        demand.append(demand1)
        supply.append(supply1)
        prices1.append(price1)
    print(str(len(buyers)) + " + " + str(len(sellers)) + " = " + str(len(buyers) + len(sellers)))
    

    buyers.clear()
    sellers.clear()

    ax.clear()
    ax.scatter(prices1, demand, c='green', s=2)
    ax.scatter(prices1, supply, c='red', s=2)

    # Обновляем график
    fig.canvas.draw()
    fig.canvas.flush_events()
    
