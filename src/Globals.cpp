#include "tdp_maps_widget/Globals.h"

#include <QImage>

namespace tdp_maps_widget
{

//##################################################################################################
tp_maps::TextureData convertTexture(const QImage& image)
{
  tp_maps::TextureData textureData;

  QImage img = image.mirrored(false, true);
  img = img.convertToFormat(QImage::Format_ARGB32);

  textureData.w = img.width();
  textureData.h = img.height();

  textureData.data = new tp_maps::Pixel[size_t(textureData.w * textureData.h)];
  {
    const uchar* p = img.bits();
    const uchar* pMax = p + (img.bytesPerLine() * img.height());
    tp_maps::Pixel* dst = textureData.data;
    for(; p<pMax; p+=4, dst++)
    {
      dst->r = p[2];
      dst->g = p[1];
      dst->b = p[0];
      dst->a = p[3];
    }
  }

  return textureData;
}

}
