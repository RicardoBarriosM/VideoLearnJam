#pragma once
#include "Renderer.h"

class VideoTexture
{
private:
	unsigned int m_RendererID;
	int m_Width, m_Height;
public:
	VideoTexture(const int witdh, const int height, const unsigned char* data);
	~VideoTexture();

	void Update(const unsigned char* newData);
	void Bind(unsigned int slot = 0) const;
	void UnBind() const;

	inline int GetWitdh() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }
};
