/* 
 * File:   example_wf.h
 * Author: simon
 *
 * Created on November 6, 2014, 11:29 AM
 */

#ifndef M3D_EXAMPLE_WF_H
#define M3D_EXAMPLE_WF_H

#include <meanie3D/defines.h>
#include <meanie3D/namespaces.h>
#include <meanie3D/utils.h>
#include <meanie3D/weights/weight_function.h>
#include <vector>

namespace m3D {
    
    template <class T>
    class ExampleWF : public WeightFunction<T>
    {
    private:

        MultiArray<T>       *m_weight;

    public:

        ExampleWF(FeatureSpace<T> *fs, const vector<T> &center)
        {
            using namespace utils::vectors;
            
            m_weight = new MultiArrayBlitz<T>(fs->coordinate_system->get_dimension_sizes(),0.0);
            
            for (size_t i=0; i < fs->points.size(); i++)
            {
                Point<T> *p = fs->points[i];
                T distance = vector_norm(center - p->coordinate);
                m_weight->set(p->gridpoint, distance);
            }
        }

        ~ExampleWF()
        {
            if (m_weight!=NULL)
            {
                delete m_weight;
                m_weight = NULL;
            }
        }

        T operator()(const typename Point<T>::ptr p) const
        {
            return m_weight->get(p->gridpoint);
        }
    };
}


#endif	/* EXAMPLE_WF_H */
