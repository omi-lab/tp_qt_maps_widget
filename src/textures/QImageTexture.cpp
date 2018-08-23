#include "tdp_maps_widget/textures/QImageTexture.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"
#include "tp_utils/StackTrace.h"

#include <QImage>
#include <QPainter>

namespace tdp_maps_widget
{

//##################################################################################################
struct QImageTexture::Private
{
  bool imageReady{false};
  QImage image;
  float fw{1.0f};
  float fh{1.0f};
};

//##################################################################################################
QImageTexture::QImageTexture(tp_maps::Map* map):
  Texture(map),
  d(new Private())
{

}

//##################################################################################################
QImageTexture::~QImageTexture()
{
  delete d;
}

//##################################################################################################
void QImageTexture::setImage(const QImage& image)
{
  d->imageReady = (image.width()>0 && image.height()>0);

  if(!d->imageReady)
    d->image = QImage();
  else
  {
    auto po2 = [](uint32_t v)
    {
      v--;
      v |= v >> 1;
      v |= v >> 2;
      v |= v >> 4;
      v |= v >> 8;
      v |= v >> 16;
      v++;
      return v;
    };

    int w = image.width();
    int h = image.height();

    int wpo2 = po2(w);
    int hpo2 = po2(h);

    {
      d->image = QImage(wpo2, hpo2, QImage::Format_ARGB32);
      QPainter painter(&d->image);
      painter.drawImage(0, 0, image.mirrored(false, true));
      painter.end();
    }

    {
      uchar* p = d->image.bits();
      uchar* pMax = p + (d->image.bytesPerLine() * d->image.height());
      for(; p<pMax; p+=4)
      {
        auto t = p[0];
        p[0] = p[2];
        p[2] = t;
      }
    }

    d->fw = float(w) / float(wpo2);
    d->fh = float(h) / float(hpo2);
  }

  imageChanged();
}

//##################################################################################################
bool QImageTexture::imageReady()
{
  return d->imageReady;
}

//##################################################################################################
GLuint QImageTexture::bindTexture()
{
  if(!d->imageReady)
    return 0;

  GLuint texture = bindTexture(d->image,
                               GL_TEXTURE_2D,
                               GL_RGBA,
                               magFilterOption(),
                               minFilterOption());
  return texture;
}

//##################################################################################################
GLuint QImageTexture::bindTexture(const QImage& img,
                                  GLenum target,
                                  GLint format,
                                  GLuint magFilterOption,
                                  GLuint minFilterOption)
{
  if(!map()->initialized())
  {
    tpWarning() << "Error! Trying to generate a texture on a map that is not initialized.";
    tp_utils::printStackTrace();
    return 0;
  }

  if(img.width()<1 || img.height()<1)
  {
    tpWarning() << "QImageTexture::bindTexture() called with null image";
    tp_utils::printStackTrace();
    return 0;
  }

  GLuint txId=0;
  glGenTextures(1, &txId);
  glBindTexture(target, txId);

  glTexImage2D(target, 0, format, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());

  glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilterOption);
  glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilterOption);
  glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  if((minFilterOption == GL_NEAREST_MIPMAP_NEAREST) || (minFilterOption == GL_LINEAR_MIPMAP_LINEAR))
    glGenerateMipmap(target);

  return txId;
}

//##################################################################################################
glm::vec2 QImageTexture::textureDims()const
{
  return {d->fw, d->fh};
}

//##################################################################################################
glm::vec2 QImageTexture::imageDims()const
{
  return glm::vec2(d->image.width(), d->image.height());
}

}
