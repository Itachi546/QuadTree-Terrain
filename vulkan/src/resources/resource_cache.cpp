#include "resource_cache.h"

ResCache::ResCache(const uint32_t sizeInMb, IResourceFile* resourceFile)
{
	m_cacheSize = sizeInMb * 1024 * 1024;
	m_allocated = 0;

	m_file = resourceFile;
}

ResCache::~ResCache()
{
	while (!m_lru.empty())
		free_one_resource();

	delete m_file;
}

bool ResCache::init()
{
}
