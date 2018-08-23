#ifndef tdp_maps_widget_QImageTexture_h
#define tdp_maps_widget_QImageTexture_h

#include "tp_maps/Texture.h"

class QImage;

namespace tdp_maps_widget
{

//##################################################################################################
class QImageTexture : public tp_maps::Texture
{
public:
  //################################################################################################
  QImageTexture(tp_maps::Map* map);

  //################################################################################################
  virtual ~QImageTexture();

  //################################################################################################
  void setImage(const QImage &image);

  //################################################################################################
  bool imageReady()override;

  //################################################################################################
  GLuint bindTexture()override;

  //################################################################################################
  //! Creates and binds a texure with the given image
  /*!
  \param img: The bitmap image for the texture
  \param target: The target type (normally GL_TEXTURE_2D)
  \param format: The format (normally GL_RGBA)
  \param magFilterOption: The texture magnification function to use
  \param minFilterOption: The texture minifying function to use
  \return the id for the new texture
  */
  GLuint bindTexture(const QImage& img,
                     GLenum target,
                     GLint format,
                     GLuint magFilterOption,
                     GLuint minFilterOption);

  //################################################################################################
  glm::vec2 textureDims()const override;

  //################################################################################################
  glm::vec2 imageDims()const override;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
