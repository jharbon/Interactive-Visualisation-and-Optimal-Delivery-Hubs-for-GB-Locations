# Data

Data is stored in GB_places.csv. Includes 100 rows with columns including place name, type (city or town), population, latitude (degrees) and longitude (degrees).

# Data visualisation

Used Python and Matplotlib's basemap to create an interactive visualisation of the places. Map of GB was created with circles of different sizes (proportional to population)
representing cities and towns. Hovering the mouse over a place brings up a frame with information about the place name, population, latitude, longitude and population rank. 
See script and its comments for details.

# Optimal Delivery Hub Locations 

Implemented an algorithm in c++ which can find an optimal delivery hub location. Did this by using the sum of great circle distances (Haversine formula) from hub to delivery
locations as a cost function. Hill climbing algorithm was used to minimise cost function. Started by assuming that a company can only afford one hub and deliveries can only 
be made from hub to a location. Then adjusted for the case where deliveries can be made between arbitrary locations, rather than just from the hub. Finally, created extra 
functions to solve for the case where a company can afford two hubs. 
