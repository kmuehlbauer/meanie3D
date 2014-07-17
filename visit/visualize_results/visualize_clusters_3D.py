#!/usr/bin/python

# Parameters
NETCDF_DIR = "P_NETCDF_DIR"
CLUSTER_DIR = "P_CLUSTER_DIR"
M3D_HOME = "P_M3D_HOME"

# Import modules
import sys
sys.path.append(M3D_HOME+"/visit/modules")
import glob
import os
import time
#import visit2D
import visit3D
import visitUtils
from subprocess import call

#print [key for key in locals().keys()
#       if isinstance(locals()[key], type(sys)) and not key.startswith('__')]


# General parameters
VAR_NAME = "zh"
VAR_MIN = 35;
VAR_MAX = 82.5;

# Stretching data and objects for better 3D impression
SCALE_FACTOR_Z = 3.0

# Conversion program params
CONVERSION_PARAMS  = "-t cluster "
CONVERSION_PARAMS += " -v "+VAR_NAME
CONVERSION_PARAMS += " --write-as-xml=false"
CONVERSION_PARAMS += " --extract-skin=false"
CONVERSION_PARAMS += " --vtk-dimensions x,y,z"

#-f herz-oase-20120805t1510utc-0500m-bonnjue-3d-v01a_clusters.nc "

# binaries
DYLD_LIBRARY_PATH="/usr/local/lib"
bin_prefix    = "export DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:"+DYLD_LIBRARY_PATH+";"
conversion_bin = bin_prefix + "/usr/local/bin/" + "meanie3D-cfm2vtk"
print "Conversion Command:"
print conversion_bin

# Silent
SuppressMessages(True)
SuppressQueryOutputOn()

# Set view and annotation attributes

print "Setting annotation attributes:"
visit3D.set_annotations()

print "Cleaning up *.vtk *.vtr *.png"
return_code=call("rm -f *.vtk *.vtr *.png", shell=True)

# Setting 3D view parameters
print "Setting 3D view parameters"
visit3D.set_view_to_radolan(1,SCALE_FACTOR_Z);

print "Creating colortables"
num_colors = visitUtils.create_cluster_colortable("cluster_colors")
visitUtils.create_topography_colortable()

# Add gray/black background gradient
print "Setting background gradient"
visitUtils.add_background_gradient();

# Glob the netcdf directory
print "Processing files in directory " + NETCDF_DIR
netcdf_files = glob.glob(NETCDF_DIR+"/*.nc");

for netcdf_file in netcdf_files:

    # construct the cluster filename and find it
    # in the cluster directory
    
    netcdf_path,filename    = os.path.split(netcdf_file);
    basename                = os.path.splitext(filename)[0]

    cluster_file            = CLUSTER_DIR+"/"+basename+"-clusters.nc"
    label_file              = basename+"-clusters_centers.vtk"

    print "netcdf_file  = " + netcdf_file
    print "filename     = " + filename
    print "basename     = " + basename
    print "cluster_file = " + cluster_file
    print "label_file   = " + label_file
    
    #./herz-oase-20120805t1405utc-0500m-bonnjue-3d-v01a-clusters_centers.vtk

    # check if the files both exist
    print "Visualzing file "+netcdf_file+" and cluster file "+cluster_file
    if not os.path.exists(cluster_file):
        print "Cluster file does not exist. Skipping."
        continue

    # add 3D topograpy
    visit3D.add_mapstuff("local")

    # now plot the data
    OpenDatabase(netcdf_file);

    visit3D.set_annotations()

    # Re-add the source with colours
    visit3D.add_pseudocolor(netcdf_file,VAR_NAME,"hot_desaturated",0.75,1)
    AddOperator("Threshold")
    t = ThresholdAttributes();
    t.lowerBounds=(VAR_MIN)
    SetOperatorOptions(t)

    # date/time
    visitUtils.add_datetime(netcdf_file)

    DrawPlots()
    visitUtils.save_window("source_",1)
    DeleteAllPlots()
    ClearWindow()

    start_time = time.time()
    print "-- Converting clusters to .vtr --"

    # build the clustering command
    command=conversion_bin+" -f "+cluster_file+" "+CONVERSION_PARAMS
    print command
    return_code = call( command, shell=True)

    print "    done. (%.2f seconds)" % (time.time()-start_time)
    print "-- Rendering cluster scene --"
    start_time = time.time()

    # add 3D topograpy
    visit3D.add_mapstuff("local")

    # re-plot source data as canvas
    visit3D.add_pseudocolor(netcdf_file, VAR_NAME, "gray", 0.25, 0 )

    # threshold as before
    AddOperator("Threshold")
    t = ThresholdAttributes();
    t.lowerBounds=(VAR_MIN)
    SetOperatorOptions(t)

    # date/time
    visitUtils.add_datetime(netcdf_file)

    # Add the clusters
    basename = CLUSTER_DIR+"/"
    visit3D.add_clusters_with_colortable(basename,"_cluster_","cluster_colors",num_colors)

    # or the boundaries
    #visit3D.add_boundaries(basename,"cluster_colors",num_colors)

    # Add modes as labels
    visitUtils.add_labels(label_file,"geometrical_center")

    # save as image
    DrawPlots()
    visitUtils.save_window("p1_tracking_",1)

    # change perspective
    visit3D.set_view_to_radolan(2,SCALE_FACTOR_Z);

    visitUtils.save_window("p2_tracking_",1)

    # change perspective back
    visit3D.set_view_to_radolan(1,SCALE_FACTOR_Z);

    print "    done. (%.2f seconds)" % (time.time()-start_time)

    # clean up
    DeleteAllPlots();
    ClearWindow()
    CloseDatabase(netcdf_file)
    CloseDatabase(label_file)
    visit3D.close_mapstuff()
    visitUtils.close_pattern(basename+"*.vtr")
    visitUtils.close_pattern(basename+"*.vtk")
    return_code=call("rm -f *.vt*", shell=True)

quit()