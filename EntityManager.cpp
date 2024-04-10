#include "EntityManager.h"

EntityManager::EntityManager() {}

void EntityManager::addEntities()
{	
	for (auto entity : m_entitiesToAdd)
	{
		m_entities.push_back(entity);
		auto tag = entity->m_tag;
		
		//std::cout << "Creating " << tag << " with ID " << entity->m_id << std::endl;
		
		if (m_entityMap.find(tag) == m_entityMap.end())
		{
			EntityVec entityVec;
			entityVec.push_back(entity);
			m_entityMap[tag] = entityVec;
		} 
		else
		{
			m_entityMap[tag].push_back(entity);
		}
	}
	
	m_entitiesToAdd.clear(); 
}

void EntityManager::removeDeadEntities()
{
	clearVec(m_entities);
	
	for (auto& [tag, entityVec] : m_entityMap)
	{
		clearVec(entityVec);
	}
}

void EntityManager::clearVec(EntityVec& vec)
{
	EntityVec to_delete;
	
	for (auto& e: vec)
	{
		if (!e->isActive())
		{
			//std::cout << "Deleting entity " << e->id() << " Tag " << e->tag() << std::endl;
			to_delete.push_back(e);
		}
	}
	
	for (auto& e : to_delete)
	{
		vec.erase(std::find(vec.begin(), vec.end(), e));
	}
}

std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag)
{
	auto entity = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));
	
	m_entitiesToAdd.push_back(entity);
	
	return entity;
}

const EntityVec& EntityManager::getEntities()
{
	return m_entities;
}

const EntityVec& EntityManager::getEntities(const std::string& tag)
{
	return m_entityMap[tag];
}

const EntityMap& EntityManager::getEntityMap()
{
	return m_entityMap;
}
