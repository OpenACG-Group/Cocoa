#include "Ciallo/Drawable.h"
CIALLO_BEGIN_NS

void DrawableListener::onRender()
{
}

bool DrawableListener::onClose()
{
    return true;
}

void DrawableListener::onDestroy()
{
}

Drawable::Drawable(Backend backend)
    : fBackend(backend),
      fListener(std::make_shared<DrawableListener>())
{
}

void Drawable::setListener(const std::shared_ptr<DrawableListener>& listener)
{
    fListener = listener;
}

DrawableListener *Drawable::listener()
{
    return fListener.get();
}

CIALLO_END_NS
