# -*- coding: utf-8 -*-

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.basemap import Basemap
import matplotlib.patches as mpatches
    

def value_check(n):
    # Checks if the input can be converted to float
    # Returns True if it can
    try:
        float(n)
        return True
    except:
        return False
    
def file_check(string):
    # Checks if the file name inputted can be used to open a file
    # Returns True if the file exists and False if it doesn't
    try:
        file = open(string, 'r')
        file.close()
        return True
    except:
        print('The file ' + string + ' cannot be opened!')
        return False    
    

def sorted_data(data):
    # File is opened and then parsed
    f = open('GBplaces.csv', 'r')
    data_split = []
    for row in f:
        i = row.split(',')
        if ('% place' in i) or i == ['']:
            continue
        data_split.append(i)   
    f.close()  # File closed as it is no longer needed
    
    # The sorting function from the previous assignment is used to sort the data by order of descending population
    global data_sorted
    data_sorted = sorted(data_split, reverse = True, key = lambda x: float(x[2]))
    
    types = []
    names = []
    lon_lat_pop = []
    
    # For each row, the values are appended into the corresponding empty list
    for i in data_sorted:
        # Check to make sure that all lon, lat and pop values are valid
        if not value_check(i[4]) or not value_check(i[3]) or not value_check(i[2]):
            print('There are invalid values on this row: ' )
            print(i)
            return
        # Check to make sure that all population values are positive
        if float(i[2]) < 0:
            print('Population cannot be negative! Found on this row:')
            print(i)
            return
        lon_lat_pop.append([float(i[4]), float(i[3]), float(i[2])])
        names.append(i[0])
        types.append(i[1])
    
    # Lists converted into numpy arrays
    # Lon, lat and pop are extracted to each have their own array    
    names = np.array(names)
    lon_lat_pop = np.array(lon_lat_pop)
    lon = lon_lat_pop[:, 0]
    lat = lon_lat_pop[:, 1]
    pop = lon_lat_pop[:, 2] 
    f.close()
    return lon, lat, pop, names, types   
        


def update_annot(ind):
    # This function updates the information displayed on the annotation
    # The argument, ind, is a dictionary which can be used to access the rank/index of a point
    # First line gets position of a point
    pos = scat.get_offsets()[ind["ind"][0]]
    # Position of annotation is set to position of point
    annot.xy = pos
    text = 'Name: {0}\nType: {1}\nPopulation: {2}\nLatitude, Longitude: {3}\nPopulation Rank: {4}'
    # Text formatted to include useful information about the data point in question        
    text = text.format(str(names[ind['ind'][0]]),
                       str(types[ind['ind'][0]]),
                       str(pop[ind['ind'][0]]),
                       str(lat[ind['ind'][0]]) + ', ' +
                       str(lon[ind['ind'][0]]), 
                       str(ind['ind'][0]+1))
    annot.set_text(text)
    annot.get_bbox_patch().set_facecolor('white')
    annot.get_bbox_patch().set_alpha(0.95)

    

def hover(event):
    # Information about a data point is displayed (annotation) when the mouse hovers over it
    vis = annot.get_visible()
    if event.inaxes == axis:
        # Position of the point returned by contain
        # ind is a dictionary which can be accessed to use the ranking/index of the point
        # ind can actually contain indices of multiple points in the case of overlap
        cont, ind = scat.contains(event)
        if cont:
            update_annot(ind)
            annot.set_visible(True)
            fig.canvas.draw_idle()
        else:
            if vis:
                annot.set_visible(False)
                fig.canvas.draw_idle()

def colours(sorted_data):
    # Empty list c used to store the colour of a point based on whether it's a city or town
    # c is returned so that it can be used in the main function when the points are being plotted
    c = []
    for i in sorted_data:
        if i[1] == 'City':
            c.append('red')
        if i[1] == 'Town':
            c.append('black')
    return c    


def main(file):
    if not file_check(file):
        print('The file does not exist or could not be found.')
        return
    
    global lon, lat, pop, names, types
    lon, lat, pop, names, types = sorted_data(file)
    
    global norm, cmap
    norm = plt.Normalize(1, 4)
    cmap = plt.cm.RdYlGn
    
    global fig, axis
    fig, axis = plt.subplots(figsize = (10,20))
    
    
    map = Basemap(resolution = 'l',   # Resolution of boundary database used. 'c' (crude), 'l' (low) etc ..
                  projection = 'merc',  # How the map is projected
                  lat_0 = 51.5, lon_0 = 0.0,  # Coordinates at which the map is centred
                  llcrnrlon = -7.0, llcrnrlat = 49.0,  # Coordinates of lower left corner
                  urcrnrlon = 2.0, urcrnrlat = 58.0)  # Coordinates of upper right corner
    map.drawmapboundary(fill_color = 'aqua')  # Background colour
    map.fillcontinents(lake_color = 'aqua')  # Colour of land (grey by default) and lakes set to same as background
    map.drawcoastlines()
    
    # Projection of points to the correct positions
    lon_projected, lat_projected = map(lon, lat)
    
    global scat
    # Plotting graph
    # Colours function used to colour cities and towns correctly
    # Size of points related to size of population
    # Zorder set to 2 so that the points are plotted in front of the map
    scat = plt.scatter(lon_projected, lat_projected, c = colours(data_sorted),
                       s = pop/(1*10**4), cmap = cmap, norm = norm, zorder = 2)
    # Two patches created corresponding to colours of cities and towns
    # Patches used in plt.legend
    cities_patch = mpatches.Patch(color = 'red', label = 'Cities')
    towns_patch = mpatches.Patch(color = 'black', label = 'Towns')
    plt.legend(handles = [cities_patch, towns_patch], title = 'Places')
    global annot
    annot = axis.annotate("", xy=(0,0), xytext=(20,20),textcoords="offset points",
                    bbox=dict(boxstyle="round", fc="w"),
                    arrowprops=dict(arrowstyle="->"))
    
    # Make the annotation invisible for any position that doesn't contain a point
    # Hover function will set the argument to True when the position contains a data point
    annot.set_visible(False)
    
    fig.canvas.mpl_connect("motion_notify_event", hover)
    
    plt.show()
    
main('GBplaces.csv')  # Calling the main function with GBplaces.csv as the argument  
    
    
    
                  
    
    
    
    
    
    
    
    
    
    