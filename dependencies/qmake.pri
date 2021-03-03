
greaterThan(QT_MAJOR_VERSION, 5) {
 QT += core gui widgets opengl openglwidgets
}
else {
  QT += core gui widgets opengl
}

