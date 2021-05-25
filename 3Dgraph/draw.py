import matplotlib.pyplot as plt
import numpy as np





fig = plt.figure(figsize=(50,50))
ax = fig.add_subplot(projection='3d')


def draw_Layer(x1,y1,x2,y2,z):

    if(z%2==1):
        color = "blue"
        for y in range(y1,y2+1):
            ax.plot([x1,x2],[y,y],[z,z], color)
            ax.plot([x1,x2],[y,y],[z,z], color)
    else :
        color = "red"
        for x in range(x1,x2+1):
            ax.plot([x,x],[y1,y2],[z,z], color)
            ax.plot([x,x],[y1,y2],[z,z], color)
   


    
def draw_graph(x1,y1,x2,y2,Layer,capacity):
    
    #Lim setting
    ax.set_xlabel('X Label')
    ax.set_ylabel('Y Label')
    ax.set_zlabel('Z Label')
    plt.xlim([x1,x2])
    plt.ylim([y1,y2])
    ax.set_zlim(1,Layer)


    #Ticks setting
    xticks = np.linspace(x1,x2,x2-x1+1)
    yticks = np.linspace(y1,y2,y2-y1+1)
    zticks = np.linspace(1,Layer,Layer)
    ax.set_xticks(xticks)
    ax.set_yticks(yticks)
    ax.set_zticks(zticks)
    
    # Draw Layer
    for z in range(1,Layer+1):
        draw_Layer(x1,y1,x2,y2,z)

    # Draw Via
    for y in range(y1,y2+1):
        for x in range(x1,x2+1):
            ax.plot([x,x],[y,y],[1,Layer],"black")
    
    # Draw Ggrid
    for z in range(1,Layer+1):
        for y in range(y1,y2+1):
            for x in range(x1,x2+1):
                color = "orange"
                ax.scatter(x,y,z,s=1000,c=color)



def addjust_cap(capacity,x,y,z,offset):
    capacity[z-1][y-1][x-1]+=offset

x1 = 1
y1 = 1
x2 = 3
y2 = 1
Layer = 9
capacity = [[[1000 for i in range(x2-x1+1)]for y in range(y2-y1+1)] for z in range(1,Layer+1)]


draw_graph(x1,y1,x2,y2,Layer,capacity)
plt.show()




