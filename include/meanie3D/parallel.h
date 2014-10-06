#ifndef M3D_PARALL_H
#define	M3D_PARALL_H

#if WITH_OPENMP
    #include <omp.h>
#endif

#if WITH_TBB
    #include <tbb/tbb.h>
    #define PROVIDE_MUTEX 1
    #define PROVIDE_THREADSAFETY 1
#endif

#endif	

