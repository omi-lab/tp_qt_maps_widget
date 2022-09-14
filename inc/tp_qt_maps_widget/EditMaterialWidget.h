#ifndef tp_qt_maps_widget_EditMaterialWidget_h
#define tp_qt_maps_widget_EditMaterialWidget_h

#include "tp_qt_maps_widget/Globals.h"

#include <QWidget>

namespace tp_qt_maps_widget
{

//##################################################################################################
class TP_QT_MAPS_WIDGET_SHARED_EXPORT EditMaterialWidget : public QWidget
{
  Q_OBJECT
public:
  //################################################################################################
  //! Use to set whether the widgets to set material textures should be available or not.
  enum class TextureSupported
  {
    Yes, //!< Textures are supported for the material.
    No   //!< Textures are not supported for the material. Don't display the widgets related to texture handling.
  };

  //################################################################################################
  EditMaterialWidget(TextureSupported TextureSupported = TextureSupported::Yes,
                     const std::function<void(QLayout*)>& addButtons=std::function<void(QLayout*)>(),
                     QWidget* parent = nullptr);

  //################################################################################################
  ~EditMaterialWidget() override;

  //################################################################################################
  void setMaterial(const tp_math_utils::Material& material);

  //################################################################################################
  tp_math_utils::Material material() const;

  //################################################################################################
  void setGetExistingTextures(const std::function<std::vector<tp_utils::StringID>()>& getExistingTextures);

  //################################################################################################
  void setLoadTexture(const std::function<tp_utils::StringID(const std::string&)>& loadTexture);

  //################################################################################################
  //! Shows a dialog to edit the material and returns true if accepted.
  static bool editMaterialDialog(QWidget* parent,
                                 tp_math_utils::Material& material,
                                 TextureSupported textureSupported = TextureSupported::Yes,
                                 const std::function<std::vector<tp_utils::StringID>()>& getExistingTextures = std::function<std::vector<tp_utils::StringID>()>(),
                                 const std::function<tp_utils::StringID(const std::string&)>& loadTexture = std::function<std::string(const std::string&)>());

Q_SIGNALS:
  //################################################################################################
  //! Emitted when a material is edited by the user.
  void materialEdited();

protected:
  //################################################################################################
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  struct Private;
  Private* d;
  friend struct Private;
};
}
#endif
