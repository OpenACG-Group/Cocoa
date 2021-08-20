#ifndef COCOA_AUEFFECTORSDAG_H
#define COCOA_AUEFFECTORSDAG_H

#include <string>

#include "Vanilla/Base.h"

struct AVFilterGraph;

VANILLA_NS_BEGIN

class AuEffector;
class AuEffectorsDAG
{
public:
    AuEffectorsDAG(int nInput, int nOutput);
    ~AuEffectorsDAG();

    void setInput(int idx, const std::string& name);


private:
    ::AVFilterGraph       *fGraph;
};

VANILLA_NS_END
#endif //COCOA_AUEFFECTORSDAG_H
