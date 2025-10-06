import os
import time
import math
import matplotlib
import numpy as np
import random as rd
from threading import Thread
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
from matplotlib.patches import Rectangle
from matplotlib.collections import LineCollection

#matplotlib.use('Agg')
open("broker_to_continious.txt", "w").close()
open ("broker_to_limit.txt","w").close()
open("continious_to_broker.txt","w").close()
open("limit_to_broker.txt","w").close()
open("volatility.txt","w").close()
with open("part_compl_cont.txt","w") as file:
    file.write("part q_max time \n")
with open("part_compl_lim.txt","w") as file:
    file.write("part q_max time \n")
    

print("start")
time.sleep(2)
start_time = time.time()

lambda_per_second = 2
t_max = 120

koef = 1.01
price_continious = 100
prices_continious = [100]

price_limit = 100
prices_limit = [100]

times_continious = [0]
times_limit = [0]

cnt = 0 
first = True


plt.ion() #позволяет обновлять график онлайн
fig, ax = plt.subplots()
#scatter = ax.scatter(time, prices_continious,s=5)
ax.set_xlim(0, 350) #размеры графика
ax.set_ylim(prices_continious[-1] * 0.7, prices_continious[-1] * 1.3)

lines = []

def add_lines(x1, y1, x2, y2, color):
    segment = Line2D([x1, x2], [y1, y2], color=color, lw=2)
    ax.add_line(segment)
    lines.append(segment)  # Сохраняем ссылку
    fig.canvas.draw()
    fig.canvas.flush_events()
    return segment


def random_predict(change_size, forward_time, last_price):
    pred = []
    
    for _ in range(forward_time):
        delta = rd.choices([change_size, 1 / change_size], k=1)[0]

        last_price *= delta 
        pred.append(last_price)
    return pred

def volatility2(prices, times):
    p_med = 0
    for i in range(len(prices) - 1):
        p_med += prices[i] * (times[i + 1] - times[i]) / times[-1]

    sigma2 = 0
    for i in range(len(prices) - 1):
        sigma2 += ((times[i + 1] - times[i]) * ((prices[i] - p_med) ** 2)) / times[-1]
    
    sigma = sigma2 ** 0.5
    
    #print(sigma)
    return sigma

def volatility(prices, times, decay = 0.02):
    T = times[-1]
    weights = []
    for i in range(len(prices) - 1):
        dt = times[i+1] - times[i]
        w = dt * math.exp(-decay * (T - times[i]))
        weights.append(w)

    W = sum(weights)
    
    p_med = sum(prices[i] * weights[i] for i in range(len(prices)-1)) / W
    
    sigma2 = sum(weights[i] * (prices[i] - p_med) ** 2 for i in range(len(prices)-1)) / W
    sigma = math.sqrt(sigma2)
    
    return sigma


def make_orders():
    global cnt
    while True:
        if time.time() - start_time > 350:
            print("END")
        # price_continious = 100
        # for i in range(1, len(prices_continious) - 1):
        #     if prices_continious[-i] != 999999:
        #         price_continious = prices_continious[-i]
        #         break
        price_continious = prices_continious[-1]
        price_limit =  prices_limit[-1]
        wait_time = np.random.exponential(scale=1 / lambda_per_second) 
        wait_time = max(0.1, wait_time)
        time.sleep(wait_time)
        
        #t_max = 30
        forward_time = 30
        q_max = max(1, np.random.normal(loc=10,scale=6))
        #print(q_max)
        u_max = q_max / t_max * 2
        order_type = rd.choices(["s","b",], weights=[0.5, 0.5], k=1)[0]

        predict = random_predict(change_size=koef, forward_time=forward_time, last_price=1)
        p_delta = predict[-1]

        if abs(1 - p_delta)  < 0.01:
            continue
        #print(time.time())
        #print(wait_time)
        try:
            with open("broker_to_limit.txt","a") as file:
                file.write(order_type + " " + str(price_limit * p_delta) + " " + str(q_max) + " " + str(t_max) + "\n")
        except:
            time.sleep(0.001)
            with open("broker_to_limit.txt","a") as file:
                file.write(order_type + " " + str(price_limit * p_delta) + " " + str(q_max) + " " + str(t_max) + "\n")


        
        
        if p_delta > 1:
            p_l = price_continious
            p_h = max(0, 2 * p_delta * price_continious - price_continious)
            
            with open("broker_to_continious.txt", "a") as file:
                file.write(order_type + " " + str(p_l) + " " + str(p_h)
                            + " " + str(u_max) + " " + str(q_max) + " " + str(t_max) + "\n")
                # print(order_type + " " + str(p_l) + " " + str(p_h)
                #             + " " + str(u_max) + " " + str(q_max) + " " + str(t_max) + "\n")
            cnt += 1
        
        elif p_delta < 1:
            p_h = price_continious
            p_l = max(0, 2 * p_delta * price_continious - price_continious)

            with open("broker_to_continious.txt", "a") as file:
                file.write(order_type + " " + str(p_l) + " " + str(p_h)
                            + " "+ str(u_max) + " " + str(q_max) + " " + str(t_max) + "\n")
                # print(order_type + " " + str(p_l) + " " + str(p_h)
                #             + " "+ str(u_max) + " " + str(q_max) + " " + str(t_max) + "\n")
            cnt += 1
        print(cnt)

def graphic():
    while True:
        while True:
            time.sleep(0.01)
            if os.path.getsize("continious_to_broker.txt") != 0 and os.path.getsize("limit_to_broker.txt") != 0:
                break

        
        try:
            with open("continious_to_broker.txt","r") as file:
                data_continious = file.readlines()
                new_price, new_time = map(float,data_continious[0].split())
                if  "\x00\x00" in data_continious[0]:
                    print("Save!")
                    continue
            with open("continious_to_broker.txt", "w") as file:
                pass 
        except:
            print("Не удалось прочитать continious")

        
        try:
            with open("limit_to_broker.txt","r") as file:
                data_limit = file.readlines()
                new_price, new_time = map(float,data_limit[0].split())
                if "\x00\x00" in data_limit[0]:
                    print("Save!")
                    continue
            with open("limit_to_broker.txt", "w") as file:
                pass 
        except:
            print("Не удалось прочитать limit")

        limit_segments = []
        for i in range(len(data_limit)):
            new_price, new_time = map(float,data_limit[i].split())
            prices_limit.append(new_price)
            times_limit.append(new_time)
            #print(new_time, "time")
            if(prices_limit[-2] != 999999):
                #color = "red"
                x1 = times_limit[-2]
                y1 = prices_limit[-2]
                x2 = times_limit[-1]
                y2 = prices_limit[-2]
                limit_segments.append([(x1, y1), (x2, y2)])
                # segment = Line2D([x1, x2], [y1, y2], color=color, lw=2)
                # ax.add_line(segment)
                # lines.append(segment)  # Сохраняем ссылку
                # fig.canvas.draw()
                # fig.canvas.flush_events()
        limit_lc = LineCollection(limit_segments, colors='red', linewidths=2)
        ax.add_collection(limit_lc)

        #print(data_continious)
        continious_segments = []
        for i in range(len(data_continious)):
            new_price, new_time = map(float,data_continious[i].split())
            if new_price != 999999:
                prices_continious.append(new_price)
            else: 
                prices_continious.append(prices_continious[-1])
            times_continious.append(new_time)
            #print(new_time, "time")
            if(prices_continious[-2] != 999999):
                x1 = times_continious[-2]
                y1 = prices_continious[-2]
                x2 = times_continious[-1]
                y2 = prices_continious[-2]
                continious_segments.append([(x1, y1), (x2, y2)])

                # segment = Line2D([x1, x2], [y1, y2], color=color, lw=2)
                # ax.add_line(segment)
                # lines.append(segment)  # Сохраняем ссылку
                # fig.canvas.draw()
                # fig.canvas.flush_events()

        continious_lc = LineCollection(continious_segments, colors='blue', linewidths=2)
        ax.add_collection(continious_lc)

        fig.canvas.draw()
        fig.canvas.flush_events()

        #if(len(prices_continious) != len(times_continious) or len(times_continious) != len(times_limit) or len(times_limit) != len(prices_limit)):
        #    print(len(prices_continious),len(times_continious), len(times_limit), len(prices_limit))
        #    print("AAAAAAAAAA")
            #print(1/0)
        sigma_cont = round(volatility(prices_continious[-100:], times_continious[-100:]),2)
        sigma_lim = round(volatility(prices_limit[-100:], times_limit[-100:]),2)
        #print(sigma_cont, sigma_lim)

        with open("volatility.txt", "a") as file:
                file.write(str(sigma_cont) + " " + str(sigma_lim) + "\n")

        

            
        
        

            

if __name__ == "__main__":
    Thread(target=make_orders, daemon=True).start()

    #Thread(target=graphic).start()
    try:
        graphic()
    except KeyboardInterrupt:
        print("\nПрограмма завершена")