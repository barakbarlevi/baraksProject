Using an open source simulation called CADAC to produce ballistic trajectories. Wrote own code to run them live and plot on Google Earth.

Simulation is here: https://github.com/barakbarlevi/CADAC

./SIX_DOF inputOriginal.asc 
should work



For this project, Primary_Controller.kml needs to be dragged into Google Earth.


Demo.png shows overlapping trajectories hapenning.


I believe problem lies within the loop:
for (size_t i = 0; i < currentCollectorPriamryInputFiles.size(); ++i)


Please execuse the massive amount of globals, comments, and diorder. It's still in writing.
