#pragma once

#include "core/base.h"
#include <list>
#include <unordered_map>

class IResourceLoader;
class ResHandle;
class Resource;
class IResourceFile;

typedef std::list<Ref<ResHandle>> ResHandleList;
typedef std::unordered_map<std::string, Ref<ResHandle>> ResHandleMap;
typedef std::list<Ref<IResourceLoader>> ResourceLoaders;


class ResCache
{
public:
	ResCache(const uint32_t sizeInMb, IResourceFile* resourceFile);
	~ResCache();

	bool init();
	void RegisterLoader(Ref<IResourceLoader> loader);
	Ref<ResHandle> get_handle(Resource* r);
	void flush();

private:
	ResHandleList m_lru;
	ResHandleMap m_resources;
	ResourceLoaders m_resourceLoaders;

	IResourceFile* m_file;
	uint32_t m_cacheSize;
	uint32_t m_allocated;

	Ref<ResHandle> find(Resource* r);
	const void* update(Ref<ResHandle> handle);
	Ref<ResHandle> load(Resource* r);
	void free(Ref<ResHandle> handle);
	void free_one_resource();

	char* allocate(uint32_t size);
	void memory_has_been_freed(uint32_t size);
};