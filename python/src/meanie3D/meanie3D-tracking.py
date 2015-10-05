#!/usr/bin/python

## meanie3D-tracking.py
#
# Python script for running a whole set of netcdf files through the clustering and tracking process.
# \author Juergen Simon (juergen.simon@uni-bonn.de)

import os
import sys
import getopt

# dynamically append search path
# to include the meanie3D modules
# Parameters
MEANIE3D_HOME = os.getenv("MEANIE3D_HOME","NOT_SET");
if MEANIE3D_HOME == "NOT_SET":
    print "ERROR: environment variable MEANIE3D_HOME is not set."
    sys.exit(2)
sys.path.append(MEANIE3D_HOME+"/python/python-modules/meanie3D")
import meanie3D


# ----------------------------------------------------------------------------
## Prints usage and exits
#
def usage():
    print "meanie3D-tracking.py -c=<json file> -f=<netcdf directory> [-s=<scale>] [--start=t1 --end=t2]  [--resume,-r] [--help,-h] [--version]"
    print "runs a complete set of netcdf files through the clustering/tracking"
    print "-c : json config file specifying variables etc."
    print "-f : directory containing the files. It is assumed that"
    print "           the files are in the correct order when sorted alphabetically."
    print "-s  : (optional) comma separated list of scale parameters. Overrides any scale values in the configuration."
    print "--json-example : prints an example .json configuration and exits"
    print "--start index of time step to start with in files with time dimension"
    print "--end   index of time step to end at in files with time dimension"
    print "--resume,-r : if this flag is present, the algorithm assumes to resume"
    print "              operations where it last left off. If not present, previous"
    print "              results will be erased before starting"
    print "--help, -h  : print this message and exit."
    print "--version   : prints the version information and exits"
    sys.exit(1)
    return
# ----------------------------------------------------------------------------

# ----------------------------------------------------------------------------
## Prints the configuration file .json file description and exits
#
def print_configuration_format():
    print '''{'''
    print '''   "description" : "...",  /* Description of file content */'''
    print '''   "data" : {...},         /* call --help config.data for help on this section */'''
    print '''   "detection" : {...},    /* call --help config.detection for help on this section */'''
    print '''   "tracking" : {...},     /* call --help config.tracking for help on this section */'''
    print '''   "postprocessing" : {...} /* call --help config.postprocessing for help on this section */'''
    print '''}'''
    sys.exit(1)
# ----------------------------------------------------------------------------

# ----------------------------------------------------------------------------
## Prints version info and exits
#
def print_version():
    print "meanie3D.py: " + meanie3D.getVersion() + "\n"
    print "meanie3D-detect: " + meanie3D.app.external.execute_command_with_stdout("meanie3D-detect","--version")
    print "meanie3D-track: " + meanie3D.app.external.execute_command_with_stdout("meanie3D-track","--version")
    sys.exit(1)
    return
# ----------------------------------------------------------------------------

# ----------------------------------------------------------------------------
## Main function
def main(argv):
    # Parse command line
    try:
        opts, args = getopt.getopt(argv, "c:f:s:o:r:h", ["json-example","resume","help","version","start=","end="])
    except getopt.GetoptError as detail:
        print detail
        sys.exit(2)

    scales = []
    resume = False
    config_file = ""
    output_dir = "."
    netcdf_dir = ""
    num_params = 0

    start_time_index = -1;
    end_time_index = -1;
    
    for o, a in opts:
    
        if o == "-c":
            config_file = a
            num_params = num_params + 1
        
        elif o == "-f":
            netcdf_dir = a
            num_params = num_params + 1
        
        elif o == "-s":
            scales = str(a).split(',')

        elif o == "-o":
            output_dir = a

        elif o in ["--json-example"]:
            print_configuration_format()

        elif o in ["--resume","-r"]:
            resume = True

        elif o in ["--start"]:
            start_time_index = int(float(a))

        elif o in ["--end"]:
            end_time_index = int(float(a))
        
        elif o in ["--help"]:
            usage()
            sys.exit()

        elif o in ["--version"]:
            print_version()

        else:
            usage()
                
    if num_params < 2:
        print num_params
        usage()

    uses_time = False

    # sanity checks on time range

    if start_time_index != -1 or end_time_index != -1 :

        if (start_time_index == -1 and end_time_index != -1) or (start_time_index != -1 and end_time_index == -1):
            print "start-time-index and end-time-index must both be set"
            sys.exit(2)

        if start_time_index > end_time_index:
            print "start-time-index is after end-time-index!"
            sys.exit(2)

        uses_time = True

    # Parse configuration data and expand
    configuration = meanie3D.utils.load_configuration(config_file);

    if (configuration['detection'] or configuration['tracking']):
        configuration["source_directory"] = netcdf_dir
        configuration["output_dir"] = output_dir
        configuration["m3d_home"] = MEANIE3D_HOME
        configuration["resume"] = resume

        # run the actual clustering/tracking script
        if not scales:
            if uses_time == False:
                meanie3D.tracking.run(configuration,-1)
            else:
                for time_index in range(start_time_index,end_time_index):
                    meanie3D.tracking.run(configuration,time_index)

        else:
            for scale in scales:
                configuration["scale"] = scale
                if uses_time == False:
                    meanie3D.tracking.run(configuration,-1)
                else:
                    for time_index in range(start_time_index,end_time_index):
                        meanie3D.tracking.run(configuration,time_index)

    return
# ----------------------------------------------------------------------------

# ----------------------------------------------------------------------------
# entry point
# ----------------------------------------------------------------------------

if __name__ == "__main__":
    main(sys.argv[1:])