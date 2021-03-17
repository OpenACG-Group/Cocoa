#ifndef COCOA_BASEEVENTPROCESSOR_H
#define COCOA_BASEEVENTPROCESSOR_H

namespace cocoa {

class BaseEventProcessor
{
public:
    virtual ~BaseEventProcessor() = default;
    virtual void processEvent() = 0;
};

} // namespace cocoa

#endif //COCOA_BASEEVENTPROCESSOR_H
