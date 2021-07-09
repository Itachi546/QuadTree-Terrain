#pragma once

#include "core/base.h"
#include "resource.h"

class ResCache;
class IResourceExtraData;

class ResHandle
{
public:
	ResHandle(Resource& resource, char* buffer, uint32_t size, ResCache* pResCache);

	char* get_buffer() const { return m_buffer; }
	char* get_buffer_writable() { return m_buffer; }

	Ref<IResourceExtraData> get_extra_data() { return m_extra; }
	void set_extra_data(Ref<IResourceExtraData> extra) { m_extra = extra; }
	
	virtual ~ResHandle();
private:
	Resource m_resource;
	char* m_buffer;
	uint32_t m_size;

	Ref<IResourceExtraData> m_extra;
	ResCache* m_pResCache;
};