#ifndef _M3D_ClusterList_H_
#define _M3D_ClusterList_H_

#include <meanie3D/defines.h>
#include <meanie3D/namespaces.h>

#include <vector>
#include <map>
#include <assert.h>
#include <boost/shared_ptr.hpp>
#include <stdlib.h>

#include <cf-algorithms/cf-algorithms.h>

#include <meanie3D/types/cluster.h>
#include <meanie3D/types/point.h>

namespace m3D {
    
	using namespace std;
	using ::cfa::meanshift::Point;

    /** Cluster of points in feature space. A cluster is a point in feature space,
     * where multiple trajectories of original feature space points end. This end
     * point is called the cluster's 'mode'.
     */
    template <class T>
    class ClusterList
    {
        
    protected:
        
        typedef map< vector<T>, typename Cluster<T>::ptr > ClusterMap;
        
#if WRITE_MODES
        vector< vector<T> >         m_trajectory_endpoints;
        vector<size_t>              m_trajectory_lengths;
#endif
        ClusterMap                  m_cluster_map;
        
    public:
        
        typename Cluster<T>::list clusters;
        
#pragma mark -
#pragma mark Constructor/Destructor
        
        /** Standard constructor
         */
        ClusterList();
        
        /** Destructor
         */
        ~ClusterList();
        
#pragma mark -
#pragma mark Accessing the list
        
        /** @return number of clusters in the list
         */
        size_t size();
        
        /** @param index
         * @return cluster at index
         */
        typename Cluster<T>::ptr operator[] (size_t index);
        
#pragma mark -
#pragma mark Adding / Removing points
        
        typedef typename FeatureSpace<T>::Trajectory Trajectory;
        
        /** Adds the end point of the trajectory as cluster. If a cluster already
         * exists, this point is added to it.
         * All points within grid resolution along the trajectory are also added as
         * points to this cluster.
         * @param feature space point x (the start point)
         * @param list of feature space coordinates (trajectory)
         * @param bandwidth of the iteration
         * @param fuzziness, which defines how close clusters can get to each other.
         * @param index
         */
        void add_trajectory( const typename Point<T>::ptr x,
                            Trajectory *trajectory,
                            FeatureSpace<T> *fs );
        
#pragma mark -
#pragma mark Post-Processing
        
        /** Aggregate all clusters who's modes are within grid resolution of each other
         * into superclusters. The resulting superclusters will replace the existing
         * clusters.
         * @param Original featurespace
         * @param How close the clusters must be, in order to be added to the same supercluster.
         */
        void
        aggregate_by_superclustering( const FeatureSpace<T> *fs, const vector<T> &resolution );
        
        
        /** For each cluster in this list, checks the parent_clusters and finds the modes, that
         * are in the cluster's point list and collects all their points into a list. Finally,
         * it replaces the cluster's point list with this aggregated list of parent points.
         * Used to build superclusters hierarchically from smaller clusters.
         * @param parent cluster
         * @param min_cluster_size : drop clusters from the result that have fewer points
         */
        void aggregate_with_parent_clusters( const ClusterList<T> &parent_clusters );
        
        /** Iterate through the list of clusters and trow all with smaller number of
         * points than the given number out.
         * @param min number of points
         */
        void apply_size_threshold( unsigned int min_cluster_size, const bool& show_progress = true );
        
        
#pragma mark -
#pragma mark Clustering by Graph Theory
        
        // TODO: make the API more consistent by ordering the parameters equally
        // TODO: assign access classifiers (private/protected/public) where needed
        
        /** This requires that the shift has been calculated at each point
         * in feature-space and is stored in the 'shift' property of each
         * point. Aggregates clusters by taking each point, calculate the
         * shift target and find the closest point in feature-space again.
         * If two or more points have the same distance to the shifted
         * coordinate, the point with the steeper vector (=longer) is
         * chosen. If
         */
        void aggregate_cluster_graph( const size_t &variable_index,
                                     FeatureSpace<T> *fs, const
                                     vector<T> &resolution,
                                     const bool& show_progress );
        
        /** Finds the 'best' graph predecessor for given point p. This is done
         * by adding the 'shift' property to calculate an end point and find
         * the closest neighbour.
         * TODO: find the closest n neighbours and pick the one with the smallest
         * shift (closer to local maximum)
         * @param feature-space
         * @param feature-space index
         * @param point
         * @return best predecessor along the shift. Might return the argument,
         *         in which case we have found a mode.
         */
        typename Point<T>::ptr
        predecessor_of(FeatureSpace<T> *fs,
                       FeatureSpaceIndex<T> *index,
                       const vector<T> &resolution,
                       const size_t &variable_index,
                       typename Point<T>::ptr p);
        
        /** Find all directly adjacent clusters to the given cluster
         * @param cluster
         * @param feature-space index for searching
         * @param resolution search radius for finding neighbours (use cluster_resolution)
         * @return list of neighbouring clusters (can be empty)
         */
        typename Cluster<T>::list
        neighbours_of( typename Cluster<T>::ptr cluster,
                      FeatureSpaceIndex<T> *index,
                      const vector<T> &resolution,
                      const size_t &variable_index );
        
        /** Analyses the two clusters and decides, if they actually belong to the same
         * object or not. Make sure this is only invoked on direct neighbours!
         * @param cluster 1
         * @param cluster 2
         * @param variable index
         * @param feature-space index
         * @param search range (use cluster_resolution)
         * @return yes or no
         */
        bool
        should_merge_neighbouring_clusters( typename Cluster<T>::ptr c1,
                                           typename Cluster<T>::ptr c2,
                                           const size_t &variable_index,
                                           FeatureSpaceIndex<T> *index,
                                           const vector<T> &resolution,
                                           const double &drf_threshold);
        
        /** Find the boundary points of two clusters.
         * @param cluster 1
         * @param cluster 2
         * @return vector containing the boundary points
         * @param feature-space index
         * @param search range (use cluster resolution)
         */
        void
        get_boundary_points( typename Cluster<T>::ptr c1,
                            typename Cluster<T>::ptr c2,
                            typename Point<T>::list &boundary_points,
                            FeatureSpaceIndex<T> *index,
                            const vector<T> &resolution );
        
        
        /** Calculates the relative variablity of values in the given list
         * of points. High variability indicates strong profile in the given
         * value across the boundary
         * @param variable_index
         * @param list of points
         * @return CV = s / m
         */
        T
        relative_variability( size_t variable_index,
        		  	  	  	  const typename Point<T>::list &points );
        
        /** Classifies the properties of the dynamic range of the given list of points, compared to
         * the given range in the variable denoted by variable_index.
         * Strong dynamic range component indicates a boundary, that cuts through high signal areas.
         * Weak dynamic range component indicates a more clean cut in a through.
         * @param list of points to check
         * @param variable_index
         * @param lower bound of comparison range
         * @param upper bound of comparison range
         * @return classification
         */
        T
        dynamic_range_factor( typename Cluster<T>::ptr cluster,
                             const typename Point<T>::list &points,
                             const size_t &variable_index );
        
        /** Merges the two clusters into a new cluster and removes the mergees from
         * the list of clusters, while inserting the new cluster. The mode of the merged
         * cluster is the arithmetic mean of the modes of the merged clusters.
         * @param cluster 1
         * @param cluster 2
         * @return pointer to the merged cluster
         */
        typename Cluster<T>::ptr
        merge_clusters( typename Cluster<T>::ptr c1, typename Cluster<T>::ptr c2 );
        
        
        /** Finds the neighbours of each cluster and analyses the boundaries between them.
         * If the analysis indicates that a merge is due, the clusters are merged and the
         * procedure starts over. This is repeated, until no more merges are indicated.
         * @param index of variable used for boundary analysis
         * @param feature-space index (for searching)
         * @param resolution (use cluster_resolution)
         */
        void aggregate_clusters_by_boundary_analysis( const size_t &variable_index,
                                                     FeatureSpaceIndex<T> *index,
                                                     const vector<T> &resolution,
                                                     const double &drf_threshold,
                                                     const bool& show_progress);
        
        /** Used for analyzing the boundaries and signal correlation
         */
        void write_boundaries( const size_t &variable_index,
                              FeatureSpace<T> *fs,
                              FeatureSpaceIndex<T> *index,
                              const vector<T> &resolution );
        
#pragma mark -
#pragma mark I/O
        
        /** Writes out the cluster list into a NetCDF-file.
         * For the format, check documentation at
         * http://git.meteo.uni-bonn.de/projects/cf-algorithms/wiki/Meanshift_Clustering
         * @param full path to filename, including extension '.nc'
         * @param feature space
         * @param parameters used in the run
         */
        void write( const string& path,
                   const FeatureSpace<T> *feature_space,
                   const string& parameters );
        
        /** Static method for reading cluster lists back in.
         * @param path      : path to the cluster file
         * @param list      : contains the clusters when done
         * @param source    : contains the source file attribute's value
         * @param parameters: contains the run's parameter list
         * @param var_names : contains the list of variables used
         */
        static
        void
        read( const string& path,
        	  ClusterList<T> &list,
        	  string& source,
        	  string &parameters,
        	  string &variable_names );
        
        /** Prints the cluster list out to console
         */
        void print();
        /** Counts the number of points in all the clusters. Must be
         * equal to the number of points in the feature space.
         */
        
#pragma mark -
#pragma mark Miscellaneous
        
        // TODO: find a better place for this!
        static
        void
        reset_clustering( FeatureSpace<T> *fs )
        {
            struct clear_cluster
            {
                void operator() (void *p)
                {
                    static_cast< M3DPoint<T> * >(p)->cluster = NULL;
                };
            } clear_cluster;
            
            for_each( fs->points.begin(), fs->points.end(), clear_cluster );
        }
        
#if WRITE_MODES
        const vector< vector<T> > &trajectory_endpoints() { return m_trajectory_endpoints; }
        const vector<size_t> &trajectory_lengths() { return m_trajectory_lengths; }
#endif
        
        void sanity_check( const FeatureSpace<T> *fs )
        {
            size_t point_count = 0;
            
            for ( size_t i=0; i < clusters.size(); i++ )
            {
                point_count += clusters[i]->points.size();
            }
            
            assert( point_count == fs->size() );
        }
    };
};
    
#endif