import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

file = open("fluid_sim.dat")
data = file.readlines()
file.close()
line = data[0].split(",")

N = int(line[0])
frames = int(line[1])

dens = np.zeros((N+2, N+2), dtype = 'float')
line = data[1].split(",")
for i in range(N+2):
        for j in range(N+2):
            dens[i][j] = float(line[i + (N+2) * j])
fig = plt.figure()
ax = fig.add_subplot(111, autoscale_on = True)
ax.set_xticks([])
ax.set_yticks([])
im = plt.imshow(dens, cmap = 'gray')

def init():
    return []

def animate(t, im, dens):
    line = data[t+1].split(",")
    for i in range(N+2):
        for j in range(N+2):
            dens[i][j] = float(line[i + (N+2) * j])
    im.set_data(dens)
    return im,

if __name__ == "__main__":

    ani = animation.FuncAnimation(fig, animate, frames = frames, fargs = (im, dens), init_func = init, interval=500, blit=True)

    ani.save('video.mp4', fps = 24)
