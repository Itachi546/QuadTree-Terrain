#include "water.h"
#include "spectrum_texture.h"
#include "twiddle_factors.h"
#include "inversion.h"
#include "butterfly.h"
#include "normal_map_generator.h"
#include "water_renderer.h"

#include "renderer/context.h"
#include "renderer/device.h"
#include "renderer/shaderbinding.h"

Water::Water(Context* context)
{
	m_properties = CreateRef<WaterProperties>();
	// @TODO support other dimension by updating the 
	// generation of bit reversed indices

	m_properties->dimension = 256;
	m_properties->horizontalDimension = 512;
	m_properties->windDirection = glm::vec2(1.0f, 1.0f);
	m_properties->windSpeed = 80.0f;
	m_properties->philipAmplitude = 10.0f;

	m_spectrumTexture = CreateRef<SpectrumTexture>(context, m_properties);
	m_spectrumTexture->create_spectrum_texture(context, m_properties);

	m_butterflyTexture = CreateRef<TwiddleFactors>(context, m_properties->dimension);
	m_butterflyTexture->create_twiddle_texture(context);

	{
		m_fft = CreateRef<ButterflyOperation>(context, m_properties->dimension);

		m_fftBindings = Device::create_shader_bindings();
		m_fftBindings->set_storage_image(m_butterflyTexture->get_twiddle_texture(), 0);
		m_fftBindings->set_storage_image(m_spectrumTexture->get_pingpoing_texture0(), 1);
		m_fftBindings->set_storage_image(m_spectrumTexture->get_pingpoing_texture1(), 2);
	}

	m_inversion = CreateRef<Inversion>(context, m_properties->dimension, m_spectrumTexture->get_pingpoing_texture0(), m_spectrumTexture->get_pingpoing_texture1());
	m_normaMapGenerator = CreateRef<NormalMapGenerator>(context, m_inversion->get_height_texture());

	m_renderer = CreateRef<WaterRenderer>(context, m_inversion->get_height_texture(), m_normaMapGenerator->get_normal_texture());

	debugBindings = Device::create_shader_bindings();
	//debugBindings->set_texture_sampler(m_normaMapGenerator->get_normal_texture(), 0);
	debugBindings->set_texture_sampler(m_spectrumTexture->get_pingpoing_texture0(), 0);
}

void Water::update(Context* context, float dt)
{
	m_timeElapsed += dt;
	context->begin_compute();
	m_spectrumTexture->create_hdt_texture(context, m_timeElapsed, m_properties->horizontalDimension);
	m_fft->update(context, m_fftBindings, m_properties->dimension);
	m_inversion->update(context, m_properties->dimension, m_fft->get_texture_index());

	Texture* textures[] = {
			m_spectrumTexture->get_pingpoing_texture0(),
			m_spectrumTexture->get_pingpoing_texture1(),
			m_inversion->get_height_texture(),
	};
	context->transition_layout_for_shader_read(textures, ARRAYSIZE(textures));
	m_normaMapGenerator->generate(context);

	Texture* normalMap = m_normaMapGenerator->get_normal_texture();
	context->transition_layout_for_shader_read(&normalMap, 1);
	context->end_compute();
}

void Water::render(Context* context, ShaderBindings** uniformBindings, uint32_t count)
{
	m_renderer->render(context, uniformBindings, m_translate, count);
}

void Water::destroy()
{
	m_butterflyTexture->destroy();
	m_spectrumTexture->destroy();
	m_fft->destroy();
	m_inversion->destroy();
	m_renderer->destroy();
	m_normaMapGenerator->destroy();

	Device::destroy_shader_bindings(m_fftBindings);
	Device::destroy_shader_bindings(debugBindings);
}

