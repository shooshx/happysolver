#pragma once
#ifndef NOISEGENERATOR_H_INCLUDED
#define NOISEGENERATOR_H_INCLUDED

class GlTexture;
class QWidget;

class NoiseGenerator
{
public:
	//NoiseGenerator(QWidget* parent) : m_inWidget(parent) {}

	// size in pixels, start amplitude, factor amplitude
	static GlTexture* make3Dnoise(int size = 128, float ampStart = 0.5, float ampFactor = 0.5, int startFrequency = 4);

private:
	//QWidget *m_inWidget;
};


#endif // NOISEGENERATOR_H_INCLUDED
