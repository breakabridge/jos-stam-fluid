# jos-stam-fluid
Fast 2D fluid solver written in C++ and visualised using matplotlib in Python, based on Jos Stam's 2003 paper "Real-Time Fluid Dynamics for Games". There are many example videos I created with different initial/boundary conditions that look pretty.

How to use: -- Compile and run "fluid.cpp". You may wish to change the simulation parameters as described in the code comments. This will output a data file,
"fluid_sim.dat".

-- Run "fluid_plotter.py". This reads the data file and creates the video. NOTE: THE PROGRAM Ffmpeg MUST BE INSTALLED FOR THIS TO WORK. If you know some matplotlib you may want to change the code so that it outputs to a .gif, etc.
