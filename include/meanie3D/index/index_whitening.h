/* The MIT License (MIT)
 * 
 * (c) Jürgen Simon 2014 (juergen.simon@uni-bonn.de)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef M3D_WHITENINGINDEX_H
#define M3D_WHITENINGINDEX_H

#include <meanie3D/defines.h>
#include <meanie3D/namespaces.h>
#include <meanie3D/utils/time_utils.h>
#include <meanie3D/utils/visit.h>
#include <meanie3D/index.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>

namespace m3D {

    /** Abstract base class for a feature space index, which
     * requires pre-whitening of data.
     */
    template <typename T>
    class WhiteningIndex : public PointIndex<T>
    {
    public:

        /** This is the constant radius used for the whitening
         * transformation
         */
        static const T white_radius;

    protected:

        typedef boost::numeric::ublas::matrix<T> matrix_t;
        typedef boost::numeric::ublas::vector<T> vector_t;

#pragma mark 
#pragma mark Member variables

        matrix_t point_matrix; // Featurespace as matrix (size x dimensions)

        matrix_t white_point_matrix; // Whitened featurespace matrix (size x dimensions)

        matrix_t omega; // Transformation matrix

        vector<T> white_range; // Last transformation parameters

#pragma mark 
#pragma mark Protected Constructor/Destructor

        inline
        WhiteningIndex(typename Point<T>::list *points, size_t dimension) : PointIndex<T>(points, dimension)
        {
        };

        inline
        WhiteningIndex(typename Point<T>::list *points, const vector<size_t> &indexes) : PointIndex<T>(points, indexes)
        {
        };

        inline
        WhiteningIndex(FeatureSpace<T> *fs) : PointIndex<T>(fs)
        {
        };

        inline
        WhiteningIndex(FeatureSpace<T> *fs, const vector<netCDF::NcVar> &index_variables) : PointIndex<T>(fs, index_variables)
        {
        };

        inline
        WhiteningIndex(const WhiteningIndex<T> &o) : PointIndex<T>(o), point_matrix(o.point_matrix)
        {
        };

#pragma mark -
#pragma mark Abstract Protected Methods

        virtual
        void
        build_index(const vector<T> &bandwidths) = 0;

    public:

#pragma mark -
#pragma mark Abstract Public Methods

        virtual
        typename Point<T>::list *
        search(const vector<T> &x, const SearchParameters *params, vector<T> *distances = NULL) = 0;

        virtual
        void
        add_point(typename Point<T>::ptr p) = 0;

#pragma mark -
#pragma mark Destructor

        ~WhiteningIndex()
        {
        };

    protected:

#pragma mark -
#pragma mark Protected Specific Methods

        //        void test_lapack() 
        //        {
        //            matrix_t A(3,2), B(2,2), C(3,2);
        //            A(0,0) = 2.0;
        //            A(0,1) = 1.0;
        //            A(1,0) = -1.0;
        //            A(1,1) = 2.0;
        //            A(2,0) = -1.0;
        //            A(2,1) = 2.0;
        //            
        //            cout << A << endl;
        //            
        //            //B = boost::numeric::ublas::identity_matrix<T>(2,2);
        //            B(0,0) = 0.5;
        //            B(0,1) = 0.0;
        //            B(1,0) = 0.0;
        //            B(1,1) = 0.5;
        //            
        //            cout << B << endl;
        //
        //            C = boost::numeric::ublas::prod( A, B );
        //            
        //            cout << C << endl;
        //        }

        // calculate transformation omega from F to F' 
        // and calculate F'

        void transform_featurespace(vector<T> ranges)
        {
            using namespace std;
            using namespace ::m3D::utils;

            // test_lapack();

            // Lazy transform

            if (point_matrix.size1() == 0) {
                // turn the feature-space into matrix form with #points rows
                // and #dimensions columns.

                point_matrix = matrix_t(this->size(), this->dimension());

                typename Point<T>::list::iterator it;

                for (size_t row_index = 0; row_index < this->size(); row_index++) {
                    typename Point<T>::ptr p = this->m_points->at(row_index);

                    for (size_t col_index = 0; col_index < this->dimension(); col_index++) {
                        size_t actual_index = this->m_index_variable_indexes[col_index];

                        point_matrix(row_index, col_index) = p->values[ actual_index ];
                    }
                }
            }

            white_range = ranges;

            // construct a diagonal matrix with the whitening factors
            // from the bandwidth vector

            omega = boost::numeric::ublas::matrix<T>(this->dimension(), this->dimension());

            for (size_t i = 0; i < this->dimension(); i++) {
                for (size_t j = 0; j < this->dimension(); j++) {
                    omega(i, j) = (i == j) ? (white_radius / white_range[i]) : 0.0;
                }
            }

            // multiply the original featurespace matrix with
            // this transformation matrix
            white_point_matrix = boost::numeric::ublas::prod(point_matrix, omega);
        };

        /** Transform the given coordinate using the current omega
         * transformation.
         * @param vector
         * @return vector
         */
        vector<T> transform_vector(const vector<T> &x)
        {
            vector<T> r(x.size());

            for (size_t i = 0; i < x.size(); i++) {
                r[i] = omega(i, i) * x[i];
            }

            return r;
        };
    };

    template <typename T>
    const T WhiteningIndex<T>::white_radius = 1.0;
}

#endif
