//
//  cf-cfm2vtk
//  cf-algorithms
//
//  Created by Jürgen Lorenz Simon on 28/1/13.
//  Copyright (c) 2012 Jürgen Lorenz Simon. All rights reserved.
//

#include <boost/tokenizer.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/exception/info.hpp>
#include <boost/smart_ptr.hpp>

#include <cf-algorithms/cf-algorithms.h>
#include <meanie3D/meanie3D.h>

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <exception>
#include <locale>

#include <netcdf>

using namespace boost;
using namespace cfa::meanshift;
using namespace cfa::utils::visit;
using namespace cfa::utils::vectors;
using namespace m3D;

using namespace netCDF;
using namespace std;

/** Feature-space data type */
typedef double FS_TYPE;

/** Verbosity */
typedef enum {
    FileTypeClusters,
    FileTypeComposite,
    FileTypeUnknown
} FileType;

void parse_commmandline(program_options::variables_map vm,
                        string &filename,
                        string &destination,
                        string &variable,
                        vector<size_t> &vtk_dimension_indexes,
                        FileType &type,
                        bool &extract_skin,
                        bool &write_as_xml)
{
    // filename
    
    if ( vm.count("file") == 0 )
    {
        cerr << "Missing 'file' argument" << endl;
        
        exit( 1 );
    }
    else
    {
        filename = vm["file"].as<string>();
    }
    
    // destination
    
    if ( vm.count("destination") != 0 )
    {
        destination = vm["destination"].as<string>();
    }
    
    // variable
    
    if ( vm.count("variable") == 0 )
    {
        cerr << "Missing 'variable' argument" << endl;
        
        exit( 1 );
    }
    else
    {
        variable = vm["variable"].as<string>();
    }

    // figure out file type
    
    if ( vm.count("type") == 0 )
    {
        cerr << "Missing 'type' argument" << endl;
        
        exit(-1);
    }
    
    string type_str = vm["type"].as<string>();
    
    boost::algorithm::to_lower(type_str);
    
    if (type_str == string("cluster"))
    {
        type = FileTypeClusters;
    }
    else if (type_str == string("composite"))
    {
        type = FileTypeComposite;
    }
    else
    {
        type = FileTypeUnknown;
    }
    
    // Delauney filtering?
    
    extract_skin = vm["extract-skin"].as<bool>();

    // xml?
    
    write_as_xml = vm["write-as-xml"].as<bool>();
    
    // VTK dimension mapping
    
    if ( vm.count("vtk-dimensions") > 0 )
    {
        // parse dimension list
        
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        
        boost::char_separator<char> sep(",");
        
        string str_value = vm["vtk-dimensions"].as<string>();
        
        tokenizer dim_tokens( str_value, sep );
        
        try
        {
            NcFile *file = new NcFile(filename,NcFile::read);
            
            vector<NcDim> dimensions = file->getVar(variable).getDims();
        
            for ( tokenizer::iterator tok_iter = dim_tokens.begin(); tok_iter != dim_tokens.end(); ++tok_iter )
            {
                const char* name = (*tok_iter).c_str();
                
                NcDim dim = file->getDim( name );
                
                vector<NcDim>::const_iterator fi = find( dimensions.begin(), dimensions.end(), dim );
                
                if ( fi == dimensions.end() )
                {
                    cerr << "--vtk-dimension parameter " << dim.getName() << " is not part of " << variable <<"'s dimensions" << endl;
                    exit(-1);
                }
                
                size_t index = fi - dimensions.begin();
                
                vtk_dimension_indexes.push_back( index );
            }
            
            if ( vtk_dimension_indexes.size() != dimensions.size() )
            {
                cerr << "The number of vtk-dimensions must be identical to the number of dimensions of " << variable << endl;
                
                exit(-1);
            }
            
            delete file;
        }
        catch (const netCDF::exceptions::NcException &e)
        {
            cerr << e.what() << endl;
            
            exit(-1);
        }
    }

}

void convert_clusters(const string &filename,
                      const string& variable_name,
                      const string &destination,
                      bool extract_skin,
                      bool write_as_xml)
{
    CoordinateSystem<FS_TYPE> *cs = NULL;
    
    ClusterList<FS_TYPE> *list = ClusterList<FS_TYPE>::read(filename, &cs);
    
    //::m3D::utils::VisitUtils<FS_TYPE>::write_clusters_vtr(list, cs, list->source_file, true, false, true);
    
    ::m3D::utils::VisitUtils<FS_TYPE>::write_clusters_vtu(list, cs, list->source_file, 5, true, extract_skin, write_as_xml);
    
    boost::filesystem::path path(filename);
    string centers_path = path.filename().stem().string() + "_centers.vtk";
    ::m3D::utils::VisitUtils<FS_TYPE>::write_geometrical_cluster_centers_vtk(centers_path, list->clusters);
    
    delete list;
    
    delete cs;
}

void convert_composite(const string &filename, const string& variable_name, const string &destination)
{
    try
    {
        // construct the destination path
        
        std::string filename_noext;
        
        string filename_only = boost::filesystem::path(filename).filename().string();
        
        boost::filesystem::path destination_path = boost::filesystem::path(destination);
        
        destination_path /= filename_only;
        
        destination_path.replace_extension("vtk");
        
        string dest_path = destination_path.generic_string();
        
        
        
        NcFile *file = new NcFile(filename,NcFile::read);
        
        vector<NcDim> dimensions = file->getVar(variable_name).getDims();
        
        vector<NcVar> dim_vars;
        
        for ( size_t i=0; i < dimensions.size(); i++ )
        {
            NcVar var = file->getVar(dimensions[i].getName());
            
            dim_vars.push_back(var);
        }
        
        vector<NcVar> variables;
        
        variables.push_back(file->getVar(variable_name));
        
        vector<NcVar> feature_variables(dim_vars);
        for (size_t i=0; i<variables.size(); i++)
            feature_variables.push_back(variables[i]);
        
        CoordinateSystem<FS_TYPE> *cs = new CoordinateSystem<FS_TYPE>( dimensions,dim_vars );
        
        const map<int,double> lower_thresholds,upper_thresholds,fill_values;
        
        vector<string> variable_names;
        for (size_t i=0; i<variables.size(); i++)
            variable_names.push_back(variables.at(i).getName());

        NetCDFDataStore<FS_TYPE> *dataStore =
            new NetCDFDataStore<FS_TYPE>(filename, cs, variable_names, 0);
        
        FeatureSpace<FS_TYPE> *fs = new FeatureSpace<FS_TYPE>(cs,
                                                              dataStore,
                                                              lower_thresholds,
                                                              upper_thresholds,
                                                              fill_values);
        

        ::cfa::utils::VisitUtils<FS_TYPE>::write_featurespace_variables_vtk(filename,
                                                                            fs,
                                                                            feature_variables,
                                                                            variables);
        
        delete cs;
 
        delete fs;
        
        delete file;
        
        delete dataStore;
    }
    catch (const netCDF::exceptions::NcException &e)
    {
        cerr << e.what() << endl;
        
        exit(-1);
    }
}


/**
 *
 *
 */
int main(int argc, char** argv)
{
    using namespace cfa::utils::timer;
    using namespace cfa::utils::visit;
    using namespace cfa::meanshift;
    
    // Declare the supported options.
    
    program_options::options_description desc("Options");
    desc.add_options()
    ("help,h", "produce help message")
    ("file,f", program_options::value<string>(), "CF-Metadata compliant NetCDF-file or a Meanie3D-cluster file")
    ("variable,v", program_options::value<string>(), "Name of the variable to be used")
    ("extract-skin,s", program_options::value<bool>()->default_value(false), "Use delaunay filter to extract skin file")
    ("destination,d", program_options::value<string>()->default_value("."), "Name of output directory for the converted files (default '.')")
    ("write-as-xml,x", program_options::value<bool>()->default_value(false), "Write files in xml instead of ascii")
    ("type,t", program_options::value<string>(), "'clusters' or 'composite'")
    ("vtk-dimensions", program_options::value<string>(), "VTK files are written in the order of dimensions given. This may lead to wrong results if the order of the dimensions is not x,y,z. Add the comma-separated list of dimensions here, in the order you would like them to be written as (x,y,z)")

    ;
    
    program_options::variables_map vm;
    try
    {
        program_options::store( program_options::parse_command_line(argc, argv, desc), vm);
        program_options::notify(vm);
    }
    catch (std::exception &e)
    {
        cerr << "Error parsing command line: " << e.what() << endl;
        cerr << "Check meanshift_clustering --help for command line options" << endl;
        exit(-1);
    }
    
    if ( vm.count("help")==1 || argc < 2 ) 
    {
        cout << desc << "\n";
        return 1;
    }
    
    // Evaluate user input
    
    string filename;
    string destination;
    FileType type;
    string variable;
    vector<size_t> vtk_dimension_indexes;
    bool extract_skin = false;
    bool write_as_xml = false;

    try
    {
        parse_commmandline( vm, filename, destination, variable, vtk_dimension_indexes, type, extract_skin,write_as_xml );

        // Make the mapping known to the visualization routines
        
        ::cfa::utils::VisitUtils<FS_TYPE>::VTK_DIMENSION_INDEXES = vtk_dimension_indexes;
        ::m3D::utils::VisitUtils<FS_TYPE>::VTK_DIMENSION_INDEXES = vtk_dimension_indexes;
        
        // Select the correct point factory
        PointFactory<FS_TYPE>::set_instance( new M3DPointFactory<FS_TYPE>() );
        
        switch (type)
        {
            case FileTypeClusters:
                convert_clusters(filename, variable, destination, extract_skin, write_as_xml);
                break;

            case FileTypeComposite:
                convert_composite(filename, variable, destination);
                break;
                
            default:
                cerr << "Unknown file type" << endl;
                break;
        }
    }
    catch (const std::exception &e)
    {
        cerr << e.what() << endl;
        exit(-1);
    }
    
    return 0;
};
