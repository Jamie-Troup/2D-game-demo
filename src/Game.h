#pragma once

#include "imgui.h"
#include "imgui-SFML.h"
#include <math.h>
#include <bitset>

#include "EntityManager.h"

# define M_PI 3.14159265358979323846

struct PlayerConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S; };

struct EnemyConfig { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L; float SMIN, SMAX, SI; };

struct BulletConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };

struct BulletEvent 
{ 
	std::shared_ptr<Entity> source; 
	Vec2 target; 
	std::string type; 
};

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
	std::vector<BulletEvent>	m_bullets;
	bool						m_enemySpawnOn = true;
	bool						m_movementOn = true;
	bool						m_collisionOn = true;
	bool						m_lifespanOn = true;
	bool						m_guiOn = true;
	bool						m_renderOn = true;						
	bool						m_running = true;
	bool						m_paused = false;
	int 			 			m_score = 0;
	int 			 			m_currentFrame = 1;
	int							m_lastEnemySpawnTime = 0;
	
public:
	Game();
	
	Game(const std::string& config);
	
	void init(const std::string& path);
	void run();
	void spawnPlayer();
	void restartGame();
	 
	void spawnEnemy(bool small_enemy = false, const std::string& type = "bad", std::shared_ptr<Entity> parent = nullptr);
	void buildEnemy(std::shared_ptr<Entity> enemy, float coll_rad, Vec2& pos, Vec2& vel, float angle, float radius, int num_sides, sf::Color& col, sf::Color& out_col, float out_thick);
	void infect(std::shared_ptr<Entity>);
	
	bool spawnCollision(Vec2& pos_a, float coll_rad_a);
	bool intersects(Vec2 pos_a, Vec2 pos_b, float coll_rad_a, float coll_rad_b);
	
	Vec2 playerVel();
	
	void sSpawnBullets();
	void sMovement();
	void sLifespan();
	void sCollision();
	void sEnemySpawner();
	void sGui();
	void sRender();
	void sUserInput();
	void sInfected();
};
