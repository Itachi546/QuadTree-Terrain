#include "resource_handle.h"

ResHandle::ResHandle(Resource& resource, char* buffer, uint32_t size, ResCache* pResCache) : m_resource(resource), m_buffer(buffer), m_size(size), m_pResCache(pResCache)
{

}

ResHandle::~ResHandle()
{
	delete[] m_buffer;
}
