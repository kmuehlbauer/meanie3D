#!/usr/bin/python

# ------------------------------------------------------------------------------
# Filename: visit3D.py
#
# This module bundles python routines for handling Visit3D plotting
#
# @author Juergen Simon (juergen_simon@mac.com)
# ------------------------------------------------------------------------------

import os
import time
from pprint import pprint
# Own packages
from .utils import *
# Visit

# ------------------------------------------------------------------------------
# Adds clusters with names "*_infix_*.vtk" to the current visit window.
# The clusters are colored according to the cluster_colors colortable,
# which is created at the beginning of the visualisation code.
#
# \param infix (e.g. "_untracked_clusters_")
# \param color_table_name Name of the cluster color table
# \param color_count Number of colors in the cluster color table
# \param configuration Options to overwrite the options the clusters are
# plotted with. Clusters are added as pseudocolor plots. For information
# on the available options, check PseudocolorAttributes()
#
# ------------------------------------------------------------------------------
def add_clusters_with_colortable(infix,color_table_name,color_count,configuration):
    # now the clusters
    cluster_pattern="*"+infix+"*.vt*"
    print "Looking for cluster files at " + cluster_pattern
    cluster_list=glob.glob(cluster_pattern)

    for cluster_file in cluster_list:

        # figure out cluster number for choice of color table
        # extract the number from the cluster filename
        # to select the color table based on the ID
        cluster_num=0
        try:
            start = cluster_file.index(infix,0) + len(infix)
            end = cluster_file.index(".v",start)
            cluster_num = int( cluster_file[start:end] );
        except ValueError:
            print "Illegal filename " + cluster_file
            continue

        cluster_opacity = 1.0
        if conf["cluster_opacity"]:
            cluster_opacity = conf["cluster_opacity"]

        OpenDatabase(cluster_file)
        AddPlot("Pseudocolor", "point_color")

        cp = PseudocolorAttributes();
        cp.pointSizePixels = 2
        cp.legendFlag = 0
        cp.lightingFlag = 0
        cp.invertColorTable = 0
        cp.minFlag,cp.maxFlag = 1,1
        cp.min,cp.max = 0,color_count
        cp.colorTableName = color_table_name
        cp.opacity = 1.0
        setValuesFromDictionary(cp,configuration)
        SetPlotOptions(cp)

    return

# ------------------------------------------------------------------------------
# Adds boundaries with to the current visit window.
# @param basename first part of the search pattern
#        used to find the clusters
# @param infix (e.g. "_untracked_clusters_"
# @param color tables
# @param number of colors in the color table
# ------------------------------------------------------------------------------
@PendingDeprecationWarning
def add_boundaries(basename,color_table_name,color_count, configuration):
    infix="_boundary_"
    cluster_pattern = basename+"*"+infix+"*.vt*"
    print "Looking for boundary files at " + cluster_pattern
    cluster_list=glob.glob(cluster_pattern)
    for cluster_file in cluster_list:
        cluster_num=0
        try:
            start = cluster_file.index(infix,0) + len(infix)
            end = cluster_file.index(".v",start)
            cluster_num = int( cluster_file[start:end] );
        except ValueError:
            print "Illegal filename " + cluster_file
            continue
        # Add boundary file
        OpenDatabase(cluster_file)
        AddPlot("Pseudocolor", configuration['variable'])
        # Get pseudocolor attributes
        cp = PseudocolorAttributes();
        # Set defaults
        cp.pointSizePixels = 2
        cp.legendFlag = 0
        cp.lightingFlag = 0
        cp.invertColorTable = 0
        cp.minFlag,cp.maxFlag = 1,1
        cp.min,cp.max = 0,(color_count-1)
        cp.colorTableName = color_table_name
        cp.opacity = 0.5
        # Overwrite with config
        setValuesFromDictionary(cp,configuration)
        # Set plot attributes
        SetPlotOptions(cp)

    return

##
# Adds a vector plot from the given file.
# \param data source file
# \param configuration contains some or all of the keys for
# a vector plot. See visit's VectorAttributes().
#
def add_vectors(displacements_file,configuration):
    # open displacements vector file and add plot
    OpenDatabase(displacements_file)
    AddPlot("Vector",configuration['variable'])
    # Get vector plot attributes
    p=VectorAttributes();
    # Set defaults
    p.useStride = 1
    p.stride = 1
    p.scale = 1.0
    p.scaleByMagnitude = 0
    p.autoScale = 0
    p.headOn = 1
    p.useLegend = 0
    # Overwrite with configuration
    setValuesFromDictionary(p,configuration)
    # Set options.
    SetPlotOptions(p)
    return

# ------------------------------------------------------------------------------
# Sets default 3D view params for RADOLAN grid
# @param "local" or "national"
# @param stretching factor for z axis
# ------------------------------------------------------------------------------
def set_view_to_radolan(extend,scale_factor_z):

    v = GetView3D();

    if extend == "local":

        v.viewNormal = (0.204365, -0.63669, 0.743546)
        v.focus = (-239.212, -4222.9, 7.31354)
        v.viewUp = (-0.201314, 0.716005, 0.668438)
        v.viewAngle = 30
        v.parallelScale = 173.531
        v.nearPlane = -347.062
        v.farPlane = 347.062
        v.imagePan = (-0.00977129, 0.0399963)
        v.imageZoom = 1.4641
        v.perspective = 1
        v.eyeAngle = 2
        v.centerOfRotationSet = 0
        v.centerOfRotation = (0, 0, 0)
        v.axis3DScaleFlag = 0
        v.axis3DScales = (1, 1, 1)
        v.shear = (0, 0, 1)

    elif extend == "national":

        v.viewNormal = (0.0244371, -0.668218, 0.743564)
        v.focus = (-73.9622, -4209.15, 7.31354)
        v.viewUp = (0.00033399, 0.743792, 0.668411)
        v.viewAngle = 30
        v.parallelScale = 636.44
        v.nearPlane = -1272.88
        v.farPlane = 1272.88
        v.imagePan = (0.00341995, 0.049739)
        v.imageZoom = 1.21
        v.perspective = 1
        v.eyeAngle = 2
        v.centerOfRotationSet = 0
        v.centerOfRotation = (0, 0, 0)
        v.axis3DScaleFlag = 0
        v.axis3DScales = (1, 1, 1)
        v.shear = (0, 0, 1)

    if scale_factor_z != 1.0:
        v.axis3DScaleFlag = 1
        v.axis3DScales = (1, 1, scale_factor_z)

    SetView3D(v);
    return

# ------------------------------------------------------------------------------
# Sets up standard values for axis etc
# ------------------------------------------------------------------------------
def set_annotations():

    a = GetAnnotationAttributes()
    a.axes3D.visible=1
    a.axes3D.autoSetScaling=0
    a.userInfoFlag=0
    a.timeInfoFlag=0
    a.legendInfoFlag=1
    a.databaseInfoFlag=1

    a.axes3D.xAxis.title.visible=0
    a.axes3D.xAxis.title.userTitle = 1
    a.axes3D.xAxis.title.userUnits = 1
    a.axes3D.xAxis.title.title = "x"
    a.axes3D.xAxis.title.units = "km"

    a.axes3D.yAxis.title.visible=0
    a.axes3D.yAxis.title.userTitle = 1
    a.axes3D.yAxis.title.userUnits = 1
    a.axes3D.yAxis.title.title = "y"
    a.axes3D.yAxis.title.units = "km"

    a.axes3D.zAxis.title.visible=0
    a.axes3D.zAxis.title.userTitle = 1
    a.axes3D.zAxis.title.userUnits = 1
    a.axes3D.zAxis.title.title = "h"
    a.axes3D.zAxis.title.units = "km"

    SetAnnotationAttributes(a)
    return

# ------------------------------------------------------------------------------
# Checks if the file with the given number exists. Takes
# perspectives into account. If any perspective exists,
# all images from this number are deleted
# @param configuration
# @param basename ('source_','tracking_' etc.)
# @param number number of file
# ------------------------------------------------------------------------------
def delete_images(conf,basename,image_count):
    number_postfix = str(image_count).rjust(4,'0') + ".png";
    result = False
    if 'PERSPECTIVES' in conf.keys():
        perspective_nr = 1
        for perspective in conf['PERSPECTIVES']:
            fn = "p"+str(perspective_nr)+"_"+basename+"_"+number_postfix
            if (os.path.exists(fn)):
                os.remove(fn)
            perspective_nr = perspective_nr + 1
    else:
        fn = basename+"_"+number_postfix
        if (os.path.exists(fn)):
            os.remove(fn)

# ------------------------------------------------------------------------------
# Checks if the file with the given number exists. Takes
# perspectives into account
# @param configuration
# @param basename ('source_','tracking_' etc.)
# @param number number of file
# @return "all","none" or "partial"
# ------------------------------------------------------------------------------
def images_exist(conf,basename,image_count):
    number_postfix = str(image_count).rjust(4,'0') + ".png";
    result = False
    if 'PERSPECTIVES' in conf.keys():
        num_perspectives = len(conf['PERSPECTIVES'])
        num_found = 0
        perspective_nr = 1
        for perspective in conf['PERSPECTIVES']:
            fn = "p"+str(perspective_nr)+"_"+basename+"_"+number_postfix
            if (os.path.exists(fn)):
                num_found = num_found + 1
            perspective_nr = perspective_nr + 1
        if num_found == 0:
            return "none"
        elif num_found == num_perspectives:
            return "all"
        else:
            return "partial"
    else:
        fn = basename+"_"+number_postfix
        if os.path.exists(fn):
            return "all"
        else:
            return "none"

# ------------------------------------------------------------------------------
# Generic routine for visualizing 3D clusters in two perspectives
#
# The following configuration options exist:
#
# 'source_directory' : directory with the source data files
# 'cluster_directory' : directory with the cluster results
# 'meanie3d_home' : home directory of meanie3D (for the mapstuff file and modules)
# 'resume' : if true, the existing image files are not wiped and work is
#            picked up where it left off. Otherwise all existing images
#            are deleted and things are started from scratch
# 'with_background_gradient' : add a gray background gradient to the canvas?
# 'with_topography' : use the topography data from the mapstuff file?
# 'with_rivers_and_boundaries' : add rivers and boundaries?
# 'with_source_backround' : re-add the source data when plotting clusters?
# 'with_datetime' : add a date/time label?
# 'create_source_movie' : create a movie from the source images?
# 'create_clusters_movie' : create a movie from the cluster images?
# 'SCALE_FACTOR_Z' : scale factor for the Z axis.
# 'grid_extent' : "national" or "local"
# 'conversion_params' : parameters for meanie3D-cfm2vtk
# 'variables' : list of variables for the source data
# 'lower_tresholds' : bottom cutoff for each variable,
# 'upper_tresholds' : top cutoff for each variable,
# 'var_min' : lowest value on legend
# 'var_max' : highest value on legend
# 'colortables' : colortable to use for each variable
# 'colortables_invert_flags' : flag indicating inversion of colortable
#                              for each variable
# 'PERSPECTIVES' : array with perspective objects
# 'opacity' : opacity to use for each variable
# ------------------------------------------------------------------------------
def visualization(conf):

    print "-------------------------------------------------"
    print "visit3D.visualization started with configuration:"
    print "-------------------------------------------------"
    pprint(conf)
    print "-------------------------------------------------"

    bin_prefix = "export DYLD_LIBRARY_PATH="+ utils.get_dyld_library_path()+";"
    conversion_bin = bin_prefix + "/usr/local/bin/" + "meanie3D-cfm2vtk"

    # Set view and annotation attributes

    print "Setting annotation attributes:"
    set_annotations()

    if conf['resume'] == False:
        print "Removing results from previous runs"
        return_code=call("rm -rf images movies *.vtk *.vtr *.png", shell=True)
    else:
        print "Removing intermediary files from previous runs"
        return_code=call("rm -f *.vtk *.vtr", shell=True)

    if conf['create_clusters_movie']:
        print "Creating colortables"
        num_colors = utils.create_cluster_colortable("cluster_colors")

    if conf['with_topography']:
        utils.create_topography_colortable()

    if conf['with_background_gradient']:
        print "Setting background gradient"
        utils.add_background_gradient();

    scaleFactorZ = 1.0
    if "SCALE_FACTOR_Z" in conf.keys():
        scaleFactorZ = conf['SCALE_FACTOR_Z']

    # Glob the netcdf directory
    print "Processing files in directory " + conf['source_directory']
    netcdf_files = glob.glob(conf['source_directory']+"/*.nc");

    # Keep track of number of images to allow
    # forced re-set in time to circumvent the
    # Visit memory leak
    image_count=0

    for netcdf_file in netcdf_files:

        # construct the cluster filename and find it
        # in the cluster directory

        netcdf_path,filename    = os.path.split(netcdf_file);
        basename                = os.path.splitext(filename)[0]
        cluster_file            = conf['cluster_directory']+"/"+basename+"-clusters.nc"
        label_file              = basename+"-clusters_centers.vtk"
        displacements_file      = basename+"-clusters_displacements.vtk"

        print "netcdf_file  = " + netcdf_file
        print "cluster_file = " + cluster_file

        # check if the files both exist
        if not os.path.exists(cluster_file):
            print "Cluster file does not exist. Skipping."
            continue

        # predict the filenames for checking on resume
        number_postfix = str(image_count).rjust(4,'0')

        source_open = False
        skip_source = False

        if conf['resume'] == True:
            exists = images_exist(conf,"source",image_count)
            if exists == "all":
                print "Source visualization "+number_postfix+" exists. Skipping."
                skip_source = True
            elif exists == "partial":
                print "Deleting partial visualization " + number_postfix
                delete_images(conf,"source",image_count)

        if skip_source == False:

            OpenDatabase(netcdf_file);
            source_open = True

            if conf['create_source_movie']:

                if conf['with_topography']:
                    print "-- Adding topography data --"
                    add_map_topography(conf['grid_extent'])

                if conf['with_rivers_and_boundaries']:
                    print "-- Adding map data --"
                    add_map_rivers(conf['grid_extent'])
                    add_map_borders(conf['grid_extent'])

                if conf['with_datetime']:
                    print "-- Adding timestamp --"
                    utils.add_datetime(netcdf_file)

                print "-- Plotting source data --"
                start_time = time.time()

                # Add source data and threshold it

                variables = conf['variables']

                for i in range(len(variables)):

                    add_pseudocolor(netcdf_file,
                                            conf['variables'][i],
                                            conf['colortables'][i],
                                            conf['opacity'][i],1)
                    p = PseudocolorAttributes()
                    p.minFlag,p.maxFlag=1,1
                    p.min,p.max=conf['var_min'][i],conf['var_max'][i]
                    p.invertColorTable = conf['colortables_invert_flags'][i]
                    SetPlotOptions(p)

                    AddOperator("Threshold")
                    t = ThresholdAttributes();
                    t.lowerBounds=(conf['lower_tresholds'][i])
                    t.upperBounds=(conf['upper_tresholds'][i])
                    SetOperatorOptions(t)

                DrawPlots();

                if 'PERSPECTIVES' in conf.keys():
                    perspective_nr = 1
                    for perspective in conf['PERSPECTIVES']:
                        set_perspective(perspective,scaleFactorZ)
                        filename = "p" + str(perspective_nr) + "_source_"
                        utils.save_window(filename,1)
                        perspective_nr = perspective_nr + 1
                else:
                    set_view_to_radolan(conf['grid_extent'],conf['SCALE_FACTOR_Z'])
                    utils.save_window("source_",1)

                DeleteAllPlots()
                ClearWindow()

                print "    done. (%.2f seconds)" % (time.time()-start_time)

        if conf['create_clusters_movie']:

            skip = False

            if conf['resume'] == True:

                exists = images_exist(conf,"tracking",image_count)
                if exists == "all":
                    print "Cluster visualization "+number_postfix+" exists. Skipping."
                    skip = True
                elif exists == "partial":
                    print "Deleting partial cluster visualization " + number_postfix
                    delete_images(conf,"tracking",image_count)

            if skip == False:

                start_time = time.time()
                print "-- Converting clusters to .vtr --"

                # build the clustering command
                command=conversion_bin+" -f "+cluster_file+" "+conf['conversion_params']
                if conf['WITH_DISPLACEMENT_VECTORS']:
                    command = command + " --write-displacement-vectors"
                print command
                return_code = call( command, shell=True)

                print "    done. (%.2f seconds)" % (time.time()-start_time)

                print "-- Rendering cluster scene --"
                start_time = time.time()

                if conf['with_topography']:
                    print "-- Adding topography data --"
                    add_map_topography(conf['grid_extent'])

                if conf['with_rivers_and_boundaries']:
                    print "-- Adding map data --"
                    add_map_rivers(conf['grid_extent'])
                    add_map_borders(conf['grid_extent'])

                if conf['with_datetime']:
                    print "-- Adding timestamp --"
                    utils.add_datetime(netcdf_file)

                if conf['with_source_backround']:

                    if not source_open:
                        OpenDatabase(netcdf_file);
                        source_open = True

                    for i in range(len(variables)):

                        add_pseudocolor(netcdf_file,
                                                conf['variables'][i],
                                                conf['colortables'][i],
                                                conf['opacity'][i],1)
                        p = PseudocolorAttributes()
                        p.invertColorTable = conf['colortables_invert_flags'][i]
                        p.minFlag,p.maxFlag=1,1
                        p.min,p.max=conf['var_min'][i],conf['var_max'][i]
                        SetPlotOptions(p)

                        AddOperator("Threshold")
                        t = ThresholdAttributes();
                        t.lowerBounds=(conf['lower_tresholds'][i])
                        t.upperBounds=(conf['upper_tresholds'][i])
                        SetOperatorOptions(t)


                # Add the clusters
                basename = conf['cluster_directory']+"/"
                add_clusters_with_colortable(basename, "_cluster_", "cluster_colors", num_colors, conf)

                # Add modes as labels
                utils.add_labels(label_file,"geometrical_center")

                # Add displacement vectors
                if conf['WITH_DISPLACEMENT_VECTORS']:
                    add_displacement_vectors(displacements_file)

                DrawPlots()

                if 'PERSPECTIVES' in conf.keys():
                    perspective_nr = 1
                    for perspective in conf['PERSPECTIVES']:
                        set_perspective(perspective,scaleFactorZ)
                        filename = "p" + str(perspective_nr) + "_tracking_"
                        utils.save_window(filename,1)
                        perspective_nr = perspective_nr + 1
                else:
                    set_view_to_radolan(conf['grid_extent'],conf['SCALE_FACTOR_Z'])
                    utils.save_window("tracking_",1)

                # change perspective back
                set_view_to_radolan(conf['grid_extent'],conf['SCALE_FACTOR_Z']);

                print "    done. (%.2f seconds)" % (time.time()-start_time)

        # clean up

        DeleteAllPlots();
        ClearWindow()
        if source_open:
            CloseDatabase(netcdf_file)
            CloseDatabase(label_file)
        utils.close_pattern(basename+"*.vtr")
        utils.close_pattern(basename+"*.vtk")
        return_code=call("rm -f *.vt*", shell=True)

        # periodically kill computing engine to
        # work around the memory leak fix
        image_count=image_count+1;
        if image_count % 100 == 0:
            CloseComputeEngine()

    # close mapstuff

    close_mapstuff()

    # create loops

    if 'PERSPECTIVES' in conf.keys():

        perspective_nr = 1

        for perspective in conf['PERSPECTIVES']:

            if conf['create_source_movie']:
                movie_fn = "p" + str(perspective_nr) + "_source"
                image_fn = movie_fn + "_"
                utils.create_movie(image_fn,movie_fn+".gif")
                utils.create_movie(image_fn,movie_fn+".m4v")

            if  conf['create_clusters_movie']:
                movie_fn = "p" + str(perspective_nr) + "_tracking"
                image_fn = movie_fn + "_"
                utils.create_movie(image_fn,movie_fn+".gif")
                utils.create_movie(image_fn,movie_fn+".m4v")

            perspective_nr = perspective_nr + 1
    else:

        if conf['create_source_movie']:
            utils.create_movie("source_","source.gif")
            utils.create_movie("source_","source.m4v")

        if  conf['create_clusters_movie']:
            utils.create_movie("tracking_","tracking.gif")
            utils.create_movie("tracking_","tracking.m4v")

    # clean up

    print "Cleaning up ..."
    return_code=call("mkdir images", shell=True)
    return_code=call("mv *.png images", shell=True)
    return_code=call("mkdir movies", shell=True)
    return_code=call("mv *.gif *.m4v movies", shell=True)
    return_code=call("rm -f *.vt* visitlog.py", shell=True)

    return

