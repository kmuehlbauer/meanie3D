#ifndef M3D_TEST_FSBASE_IMPL_H
#define M3D_TEST_FSBASE_IMPL_H

#include <meanie3D/meanie3D.h>

#include <iostream>
#include <vector>
#include <limits>
#include <boost/algorithm/string.hpp>

#include "testcase_base.h"

using namespace std;

#pragma mark -
#pragma mark Test case 1 - ellipsoid sample means - data generation

template<class T>
const T FSTestBase<T>::FILL_VALUE = std::numeric_limits<T>::min();

template<class T>
FSTestBase<T>::FSTestBase() : m_file(NULL), m_settings(NULL), m_coordinate_system(NULL),
m_totalPointCount(0), m_featureSpace(NULL), m_featureSpaceIndex(NULL)
{
}

template<class T>
FSTestBase<T>::~FSTestBase()
{
    if (m_settings != NULL) {
        delete m_settings;
    }
}

template<class T>
const char * FSTestBase<T>::dimensionName(size_t dimensionIndex)
{
    assert(dimensionIndex < 4);

    const char* dimensionName = NULL;

    switch (dimensionIndex) {
        case 0:
            dimensionName = "x";
            break;

        case 1:
            dimensionName = "y";
            break;

        case 2:
            dimensionName = "z";
            break;

        case 3:
            dimensionName = "t";
            break;

        default:
            break;
    }

    return dimensionName;
}

template<class T>
void FSTestBase<T>::generate_dimensions()
{
    INFO << "generating dimensions" << endl;

    try {
        size_t nupoints = m_settings->num_gridpoints();

        // add grid dimensions
        vector<NcDim> dimensions;
        for (size_t i = 0; i < m_settings->num_dimensions(); i++) {
            std::string dimension = dimensionName(i);
            m_dimensions.push_back(dimension);
            dimensions.push_back(this->file()->addDim(dimension, nupoints + 1));
        }

        // and dimension variables
        vector<NcVar> dimension_variables;
        for (size_t i = 0; i < m_settings->num_dimensions(); i++) {
            // create dimension variable. Values range [-max_h ... to max_h]
            float max_h = m_settings->axis_bound_values()[i];

            // Create variable with same name as dimension
            std::string name = dimensionName(i);
            NcVar var = this->file()->addVar(name, ncDouble, dimensions[i]);
            var.putAtt("valid_min", ncDouble, -max_h);
            var.putAtt("valid_max", ncDouble, max_h);
            var.putAtt("units", "m");

            // Write out it's axis data
            float* values = (float *) malloc((nupoints + 1) * sizeof (float));
            for (size_t gridIndex = 0; gridIndex < nupoints + 1; gridIndex++) {
                values[gridIndex] = -max_h + gridIndex * (2 * max_h / ((float) nupoints));
            }

            var.putVar(values);
            free(values);

            dimension_variables.push_back(var);
            m_dimension_variables.push_back(name);
        }

        m_coordinate_system = new CoordinateSystem<T>(dimensions, dimension_variables);
    } catch (netCDF::exceptions::NcException &e) {
        cerr << "FATAL:could not create coordinate system:" << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

template<class T>
NcFile *
FSTestBase<T>::file()
{
    if (m_file == NULL) {
        try {
            this->m_filename = m_settings->test_filename();
            boost::filesystem::path path(this->m_filename);
            if (boost::filesystem::exists(path)) {
                INFO << "removing test data file " << this->m_filename << endl;
                boost::filesystem::remove(path);
            }

            INFO << "creating test data file " << this->m_filename << endl;
            m_file = new NcFile(this->m_filename, NcFile::newFile);
        } catch (netCDF::exceptions::NcException &e) {
            cerr << "FATAL:could not create file " << m_filename
                    << ":" << e.what() << endl;
            exit(EXIT_FAILURE);
        }
    }

    return m_file;
}

template<class T>
NcVar
FSTestBase<T>::add_variable(string name, T valid_min, T valid_max)
{
    try {
        std::string type_name = typeid (T).name();

        if (!(type_name == "f" || type_name == "d")) {
            cerr << "FATAL:only float and double are supported at this time." << endl;
            exit(EXIT_FAILURE);
        }

        NcType type = (type_name == "f") ? ncFloat.getTypeClass() : ncDouble.getTypeClass();

        NcVar var = this->file()->addVar(name, type, this->coordinate_system()->dimensions());
        var.putAtt("valid_min", type, valid_min);
        var.putAtt("valid_max", type, valid_max);
        var.putAtt("_FillValue", type, FSTestBase<T>::FILL_VALUE);

        m_variables.push_back(name);

        return var;
    } catch (netCDF::exceptions::NcException &e) {
        cerr << "FATAL:could not add variable " << name << " to file "
                << this->m_filename << ":" << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

template<class T>
void
FSTestBase<T>::reopen_file_for_reading()
{
    INFO << "closing file " << m_filename << endl;
    this->file()->~NcFile();

    try {
        INFO << "open file " << m_filename << " for reading" << endl;

        m_file = new NcFile(this->m_filename, NcFile::read);

        // Reset the coordinate system so that
        // no references to the closed file remain

        delete m_coordinate_system;
        m_coordinate_system = NULL;
    }    catch (netCDF::exceptions::NcException &e) {
        cerr << "ERROR:could not reopen file" << m_filename << " for reading:"
                << e.what() << endl;
        exit(EXIT_FAILURE);
        ;
    }
}

template<class T>
CoordinateSystem<T> *
FSTestBase<T>::coordinate_system()
{
    if (m_coordinate_system == NULL) {
        vector<string> dimensions;

        for (size_t i = 0; i < m_settings->num_dimensions(); i++)
            dimensions.push_back(dimensionName(i));

        m_coordinate_system = new CoordinateSystem<T>(this->file(), dimensions);
    }

    return m_coordinate_system;
}

template<class T>
void FSTestBase<T>::SetUp()
{
}

template<class T>
void FSTestBase<T>::TearDown()
{
    delete m_coordinate_system;
    m_coordinate_system = NULL;

    delete m_featureSpace;
    m_featureSpace = NULL;

    delete m_featureSpaceIndex;
    m_featureSpaceIndex = NULL;

    delete m_data_store;
    m_data_store = NULL;

    delete m_file;
    m_file = NULL;

#if CLEAN_TESTFILES_UP    
    boost::filesystem::path path(this->m_filename);
    if (boost::filesystem::exists(path)) {
        INFO << "removing test data file " << this->m_filename << endl;
        boost::filesystem::remove(path);
    }
#endif
}

template<class T>
void FSTestBase<T>::generate_featurespace()
{
    INFO << "Creating featurespace from file " << m_filename << endl;
    const map<int, double> lower_thresholds, upper_thresholds, fill_values;
    this->reopen_file_for_reading();

    INFO << "Creating NetCDFDataStore from variables "
            << m_variables << endl;

    m_data_store = new NetCDFDataStore<T>(this->m_filename,
            this->m_variables,
            this->m_dimensions,
            this->m_dimension_variables);

    INFO << "Creating featurespace from data store "
            << m_variables << endl;

    this->m_featureSpace = new FeatureSpace<T>(this->coordinate_system(),
            m_data_store,
            lower_thresholds,
            upper_thresholds,
            fill_values,
            INFO_ENABLED);

    ASSERT_GT(this->m_featureSpace->size(), 0);

    this->m_featureSpaceIndex = PointIndex<T>::create(this->m_featureSpace->get_points(),
            this->m_featureSpace->rank());

    INFO << "construction done (" << m_featureSpace->size() << " points)" << endl;
}

template<class T>
string FSTestBase<T>::filename_from_current_testcase()
{
    const ::testing::TestInfo * const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    string filename = test_info->test_case_name() + string(".nc");
    boost::replace_all(filename, "/", "_");
    return filename;
}

#endif
