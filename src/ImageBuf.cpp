#include "ImageBuf.h"

#include <QImage>

ImageBuf::ImageBuf(const string& name) {
	QImage img(name.c_str(), "PNG");
	img.bits()
}