#pragma once

#include <Jnrlib.h>

class PipelineManager : public Jnrlib::ISingletone<PipelineManager>
{
    MAKE_SINGLETONE_CAPABLE(PipelineManager);

public:
    PipelineManager();
    ~PipelineManager();

};

