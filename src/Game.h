#pragma once

#include "imgui.h"
#include "imgui-SFML.h"
#include <math.h>
#include <bitset>

#include "EntityManager.h"

# define M_PI 3.14159265358979323846

struct PlayerConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S; };
// shape radius, collision radius, fill colours, outline colours, outline thickness, vertices, float s is speed
struct EnemyConfig { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L; float SMIN, SMAX, SI; };
// vel min and max, lifespan of small, spawn interval, speed min and max
struct BulletConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };
// same and contains lifespan. s is speed.

class Game
{
	sf::RenderWindow 			m_window;
	sf::Clock 		 			m_deltaClock;
	sf::Font 					m_font;
	sf::Text 					m_text;
	std::shared_ptr<Entity>		m_player;
	EntityManager 			 	m_entities;
	PlayerConfig 			 	m_playerConfig;
	EnemyConfig 			 	m_enemyConfig;
	BulletConfig 	 			m_bulletConfig;
	bool						m_sEnemySpawnOn = true;
	bool						m_sMovementOn = true;
	bool						m_sCollisionOn = true;
	bool						m_sLifespanOn = true;
	bool						m_sGuiOn = true;
	bool						m_sRenderOn = true;						
	bool						m_running = true;
	bool						m_paused = false;
	int 			 			m_score = 0;
	int 			 			m_currentFrame = 0;
	int							m_lastEnemySpawnTime = 0;
	
public:
	Game();
	
	Game(const std::string& config);
	
	void init(const std::string& path);
	void run();
	void spawnPlayer();
	void spawnEnemy();
	void spawnSmallEnemy(std::shared_ptr<Entity>& entity, std::string type);
	void spawnBullet(std::shared_ptr<Entity> entity, const Vec2& target);
	void spawnSpecialWeapon(std::shared_ptr<Entity> entity, const Vec2& target);
	
	void sMovement();
	void sLifespan();
	void sCollision();
	void sEnemySpawner();
	void sGui();
	void sRender();
	void sUserInput();
	void sCountdown();
};
