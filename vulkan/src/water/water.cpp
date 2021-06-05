#include "water.h"
#include "spectrum_texture.h"
#include "butterfly_texture.h"

#include "renderer/device.h"
#include "renderer/shaderbinding.h"


Water::Water(Context* context)
{
	m_properties = CreateRef<WaterProperties>();
	m_properties->dimension = 256;
	m_properties->horizontalDimension = 1024;
	m_properties->windDirection = glm::vec2(-1.0f, 0.0f);
	m_properties->windSpeed = 26.0f;
	m_properties->philipAmplitude = 1.0f;

	m_spectrumTexture = CreateRef<SpectrumTexture>(context, m_properties);
	m_spectrumTexture->create_spectrum_texture(context, m_properties);
	m_spectrumTexture->destroy_intermediate_data();

	m_butterflyTexture = CreateRef<ButterflyTexture>(context, m_properties->dimension);

	debugBindings = Device::create_shader_bindings();
	//debugBindings->set_texture_sampler(m_butterflyTexture->get_butterfly_texture(), 0);
	debugBindings->set_texture_sampler(m_butterflyTexture->get_butterfly_texture(), 0);
}

void Water::render(Context* context)
{
	m_butterflyTexture->create_butterfly_texture(context);
}

void Water::destroy()
{
	m_butterflyTexture->destroy();
	m_spectrumTexture->destroy();
}

