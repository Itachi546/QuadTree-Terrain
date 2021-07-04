#pragma once

class Texture;
class Context;
class ShaderBindings;

class Skybox
{
public:
	Skybox(Context* context, const char* hdriFile);
	void set_cubemap(Texture* cubemap) { m_cubemap = cubemap; }
	void destroy();
private:
	Texture* m_cubemap;
	ShaderBindings* m_bindings;

	void create_cubemap_texture(Context* context);
	Texture* load_hdri(Context* context, const char* filename);
	void convert_hdri_to_cubemap(Context* context, Texture* hdri);
	const int CUBEMAP_SIZE = 512;
};