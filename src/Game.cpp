#include "Game.h"

#include <iostream>
#include <fstream>

Game::Game(const std::string& config)
{
	init(config);
}

void Game::init(const std::string& path)
{
	srand(time(NULL));
	
	std::ifstream fin("config.txt");
	
	std::string keyword;
	
	while (fin >> keyword)
	{
		if (keyword == "Window")
		{
			int width, height, framerate, fullscreen;
			
			fin >> width >> height >> framerate >> fullscreen;
			
			if (fullscreen)
			{
				m_window.create(sf::VideoMode::getDesktopMode(), "Assignment 2", sf::Style::Fullscreen);
			} 
			else
			{
				m_window.create(sf::VideoMode(width, height), "Assignment 2");
			}
			
			m_window.setFramerateLimit(framerate);
			m_window.setKeyRepeatEnabled(false);
			
			ImGui::SFML::Init(m_window);
		}
		else if (keyword == "Font")
		{
			std::string font;
			
			int font_size, col1, col2, col3;
			
			fin >> font >> font_size >> col1 >> col2 >> col3;
			
			if (!m_font.loadFromFile(font))
			{
				std::cout << "Can't find font file." << std::endl;
			}
			
			m_text.setFont(m_font);
			m_text.setCharacterSize(font_size);
			m_text.setFillColor(sf::Color(col1, col2, col3));
		}
		else if (keyword == "Player")
		{
			fin >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
		}
		else if (keyword == "Enemy")
		{
			fin >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
		}
		else if (keyword == "Bullet")
		{
			fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
		}
		else
		{
			std::cout << "Keyword -> " << keyword << " is not recognised." << std::endl;
		}
	}
	
	spawnPlayer();
}

void Game::spawnPlayer()
{
	auto entity = m_entities.addEntity("player");
	auto size = m_window.getSize();
	
	entity->cTransform = std::make_shared<CTransform>(Vec2(size.x/2, size.y/2), Vec2(m_playerConfig.S, m_playerConfig.S), 0);
	
	entity->cInput = std::make_shared<CInput>();
	
	entity->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V, sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);
	
	entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);
	
	m_player = entity;
}

void Game::run()
{
	while(m_running)
	{
		ImGui::SFML::Update(m_window, m_deltaClock.restart());
		
		sUserInput();
		
		sSpawnBullets();
		
		if (m_movementOn && !m_paused)
		{
			sMovement();		
		}
		
		if (m_enemySpawnOn && !m_paused)
		{
			sEnemySpawner();
		}
		
		sInfected();
		
		if (m_lifespanOn)
		{
			sLifespan();		
		}
		
		m_entities.addEntities(); // adds any enemies spawned by enemy spawner
		
		if (m_collisionOn)
		{
			sCollision();		
		}
		
		m_entities.addEntities(); // adds any small enemies spawned from collisions
		
		m_entities.removeDeadEntities(); // removes all dead enemies from collisions
		
		if (m_guiOn)
		{
			sGui();
		}
		
		if (m_renderOn)
		{
			sRender();
		}
		
		m_currentFrame++;
	}
}

bool Game::spawnCollision(Vec2& pos_a, float coll_rad_a)
{
	for (auto& bullet : m_entities.getEntities("bullet"))
	{
		auto pos_b = bullet->cTransform->pos;
		auto coll_rad_b = bullet->cCollision->radius;
		
		if (intersects(pos_a, pos_b, coll_rad_a, coll_rad_b))
		{
			return true;
		}
	}
	
	for (auto& small_good : m_entities.getEntities("small_inf"))
	{
		auto pos_b = small_good->cTransform->pos;
		auto coll_rad_b = small_good->cCollision->radius;
		
		if (intersects(pos_a, pos_b, coll_rad_a, coll_rad_b))
		{
			return true;
		}
	}
	
	for (auto& spec_bull : m_entities.getEntities("special_bullet"))
	{
		auto pos_b = spec_bull->cTransform->pos;
		auto coll_rad_b = spec_bull->cCollision->radius;
		
		if (intersects(pos_a, pos_b, coll_rad_a, coll_rad_b))
		{
			return true;
		}
	}
	
	auto pos_b = m_player->cTransform->pos;
	auto coll_rad_b = m_player->cCollision->radius;
	
	if (intersects(pos_a, pos_b, coll_rad_a, coll_rad_b))
	{
		return true;
	}
	
	return false;
}

void Game::spawnEnemy(bool small_enemy, const std::string& type, std::shared_ptr<Entity> parent)
{
	// shared variables between enemy types
	float coll_rad, angle, radius, out_thick, speed;
	Vec2 pos, vel;
	sf::Color col, out_col;
	int num_sides;
	
	std::shared_ptr<Entity> enemy;
	
	if (small_enemy == false)
	{
		auto win_size = m_window.getSize();
		
		pos.x = rand() % (win_size.x - 2 * m_enemyConfig.CR) + m_enemyConfig.CR;
		pos.y = rand() % (win_size.y - 2 * m_enemyConfig.CR) + m_enemyConfig.CR;
		
		while (spawnCollision(pos, m_enemyConfig.CR))
		{
			pos.x = rand() % (win_size.x - 2 * m_enemyConfig.CR) + m_enemyConfig.CR;
			pos.y = rand() % (win_size.y - 2 * m_enemyConfig.CR) + m_enemyConfig.CR;
		}
		
		speed = ((float) rand() / RAND_MAX) * m_enemyConfig.SMAX + m_enemyConfig.SMIN;
		angle = rand() % 360;
		vel = Vec2(speed*cos(angle), speed*sin(angle));
		
		coll_rad = m_enemyConfig.CR;
		radius = m_enemyConfig.SR;
		out_thick = m_enemyConfig.OT;
		col = sf::Color(sf::Color(rand()%255, rand()%255, rand()%255));
		out_col = sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB);
		num_sides = rand() % m_enemyConfig.VMAX + m_enemyConfig.VMIN;
		
		auto enemy = m_entities.addEntity("enemy");
		
		buildEnemy(enemy, coll_rad, pos, vel, angle, radius, num_sides, col, out_col, out_thick);
	}
	else
	{
		num_sides = parent->cShape->circle.getPointCount();
		angle = (2 * M_PI) / num_sides;
		radius = parent->cShape->circle.getRadius() / 4;
		speed = 2.5;
		
		pos = parent->cTransform->pos;
			
		for (int i = 0; i < num_sides; i++)
		{
			vel = Vec2(speed*cos(i*angle), speed*sin(i*angle));
			
			if (type == "bad")
			{
				enemy = m_entities.addEntity("small_enemy");
				
				col = parent->cShape->circle.getFillColor();
				out_col = sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB);
				out_thick = m_enemyConfig.OT;
			} 
			else
			{
				enemy = m_entities.addEntity("small_inf");
				
				col = sf::Color(255, 255, 255);
				out_col = sf::Color(57, 255, 20);
				out_thick = m_enemyConfig.OT + 5;
			}
			
			coll_rad = radius + out_thick;
			
			buildEnemy(enemy, coll_rad, pos, vel, angle, radius, num_sides, col, out_col, out_thick);
			
			if (type == "good")
			{
				m_score += enemy->cScore->score; // You immediately get points for infected small enemies
			}
		}
	}
}

void Game::buildEnemy(std::shared_ptr<Entity> enemy, float coll_rad, Vec2& pos, Vec2& vel, float angle, float radius, int num_sides, sf::Color& col, sf::Color& out_col, float out_thick)
{
	enemy->cCollision = std::make_shared<CCollision>(coll_rad);
	enemy->cTransform = std::make_shared<CTransform>(pos, vel, angle);
	enemy->cShape = std::make_shared<CShape>(radius, num_sides, col, out_col, out_thick);
	
	auto tag = enemy->tag();
	
	if (tag == "enemy")
	{
		enemy->cScore = std::make_shared<CScore>(num_sides * 10);
	}
	else
	{
		enemy->cScore = std::make_shared<CScore>(num_sides * 20);
		enemy->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
	}
}

void Game::restartGame()
{
	std::cout << "YOUR SCORE WAS: " << m_score << std::endl << "Congratulations!" << std::endl;
	
	m_score = 0;
	
	for (auto& e : m_entities.getEntities())
	{
		if (e->tag() != "player")
		{
			e->destroy();
		}
	}

	auto size = m_window.getSize();
	m_player->cTransform->pos = Vec2(size.x/2, size.y/2);
}

void Game::sEnemySpawner()
{
	if ((m_currentFrame - m_lastEnemySpawnTime) > m_enemyConfig.SI)
	{
		Game::spawnEnemy();
		m_lastEnemySpawnTime = m_currentFrame;
	}
}

Vec2 Game::playerVel()
{
	auto player_vel = m_player->cTransform->velocity;
	
	float player_angle;
	
	Vec2 final_vel;

	if (m_player->cInput->keys_pressed == 2)
	{
		//std::cout << "2 keys pressed" << std::endl;
		
		if (m_player->cInput->up)
		{
			if (!m_player->cInput->down)
			{
				if (m_player->cInput->right)
				{
					player_angle = (45.0f/360)*2*M_PI;
				}
				
				if (m_player->cInput->left)
				{
					player_angle = (135.0f/360)*2*M_PI;
				}
			}
		} 
		else
		{
			if (m_player->cInput->down)
			{
				if (m_player->cInput->left)
				{
					player_angle = (-135.0f/360)*2*M_PI;
				} else
				{
					player_angle = (-45.0f/360)*2*M_PI;
				}
			}
		}
		
		if (player_angle)
		{
			final_vel = player_vel;
			final_vel.x *= cos(player_angle);
			final_vel.y *= sin(player_angle)*-1;
		}
	}
	else
	{
		if (m_player->cInput->up)
		{
			//std::cout << "UP" << std::endl;
			final_vel.y -= player_vel.y;
		}
		
		if (m_player->cInput->left)
		{
			//std::cout << "LEFT" << std::endl;
			final_vel.x -= player_vel.x;
		}
		
		if (m_player->cInput->right)
		{
			//std::cout << "RIGHT" << std::endl;
			final_vel.x += player_vel.x;
		}
		
		if (m_player->cInput->down)
		{
			//std::cout << "DOWN" << std::endl;
			final_vel.y += player_vel.y;
		}
	}
	
	return final_vel;
}

void Game::sSpawnBullets()
{
	for (auto& bull_event : m_bullets)
	{
		auto target = bull_event.target;
		auto source = bull_event.source;
		auto type = bull_event.type;
		
		// shared variables
		float radius, out_thick, lifespan, coll_rad, speed;
		sf::Color col, out_col;
		
		if (type == "bullet")
		{
			speed = m_bulletConfig.S; 
			radius = m_bulletConfig.SR;
			out_thick = m_bulletConfig.OT;
			lifespan = m_bulletConfig.L;
			coll_rad = m_bulletConfig.CR;
			col = sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB);
			out_col = sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB);
		}
		else
		{
			speed = 7.5; // add on to this value the
			radius = 15;
			out_thick = 5;
			lifespan = 45;
			coll_rad = radius + out_thick;
			col = sf::Color(255, 215, 0);
			out_col = sf::Color(255, 215, 0);
		}
		
		auto dx = target.x - source->cTransform->pos.x;
		auto dy = target.y - source->cTransform->pos.y;
		
		auto angle = atan2f(dy, dx);
		auto vel = Vec2(speed*cos(angle), speed*sin(angle));
		auto player_vel = playerVel();
		vel += player_vel;
		
		auto bullet = m_entities.addEntity(type);
		
		bullet->cTransform = std::make_shared<CTransform>(source->cTransform->pos, vel, 0);
		bullet->cShape = std::make_shared<CShape>(radius, m_bulletConfig.V, col, out_col, out_thick);
		bullet->cLifespan = std::make_shared<CLifespan>(lifespan);
		bullet->cCollision = std::make_shared<CCollision>(coll_rad);
	}
	
	m_bullets.clear();
}

void Game::sLifespan()
{
	for (auto& e : m_entities.getEntities())
	{
		if (e->cLifespan)
		{
			if (e->cLifespan->remaining > 0)
			{
				auto opacity = (float) e->cLifespan->remaining / e->cLifespan->total;
				int opacity_int = round(opacity*255);
				
				auto fill_col = e->cShape->circle.getFillColor();
				fill_col.a = opacity_int;
				auto outline_col = e->cShape->circle.getOutlineColor();
				outline_col.a = opacity_int;
				
				e->cShape->circle.setFillColor(fill_col);
				e->cShape->circle.setOutlineColor(outline_col);
				e->cLifespan->remaining--;
			} else
			{
				e->destroy();
			}
		}
	}
}

// TODO add a countdown indicator on infected normal enemies that counts down to when they explode. Also, add player-infected small collisions
void Game::sInfected()
{
	for (auto e : m_entities.getEntities())
	{
		if (e->cInfected)
		{
			if (e->cInfected->remaining > 0)
			{
				e->cInfected->remaining--;
			} 
			else
			{
				e->destroy();
				Game::spawnEnemy(true, "good", e);
			}
		}
	}
}

bool Game::intersects(Vec2 pos_a, Vec2 pos_b, float coll_rad_a, float coll_rad_b)
{
	auto dist = pos_a.dist(pos_b);
	
	if (dist < (coll_rad_a + coll_rad_b))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Game::sCollision()
{
	// Wall on player/enemy collisions
	for (auto& entity : m_entities.getEntities())
	{
		auto tag = entity->tag();
		auto& pos = entity->cTransform->pos;
		auto& vel = entity->cTransform->velocity;
		auto coll_rad = entity->cCollision->radius;
		auto win_size = m_window.getSize();
		
		if (tag == "enemy" || tag == "small_enemy" || tag == "small_inf")
		{
			if (pos.x <= coll_rad)
			{
				if (vel.x < 0)
				{
					vel.x *= -1;
				}
			} 
			else if (pos.x >= (win_size.x - coll_rad))
			{
				if (vel.x > 0)
				{
					vel.x *= -1;
				}
			}
			
			if (pos.y <= coll_rad)
			{
				if (vel.y < 0)
				{
					vel.y *= -1;
				}
			}
			else if (pos.y >= (win_size.y - coll_rad))
			{
				if (vel.y > 0)
				{
					vel.y *= -1;
				}
			}
		} 
		else if (tag == "player")
		{
			if (pos.x <= coll_rad)
			{
				if (entity->cInput->left)
				{
					pos.x += vel.x;
				}
			} 
			else if (pos.x >= (win_size.x - coll_rad))
			{
				if (entity->cInput->right)
				{
					pos.x -= vel.x;
				}
			}
			
			if (pos.y <= coll_rad)
			{
				if (entity->cInput->up)
				{
					pos.y += vel.y;
				}
			}
			else if (pos.y >= (win_size.y - coll_rad))
			{
				if (entity->cInput->down)
				{
					pos.y -= vel.y;
				}
			}
		}
	}
	
	// Merge enemy and small enemy entity vectors into one (enemyVec)
	auto vec1 = m_entities.getEntities("enemy");
	auto vec2 = m_entities.getEntities("small_enemy");
	std::vector<std::shared_ptr<Entity>> enemyVec(vec1.size() + vec2.size());
	
	if (vec1.size() > vec2.size())
	{
		vec1.insert(vec1.end(), vec2.begin(), vec2.end());
		enemyVec = vec1;
	}
	else
	{
		vec2.insert(vec2.end(), vec1.begin(), vec1.end());
		enemyVec = vec2;
	}
	
	// Perform remaining collision checks (enemy on player, bullets on enemy etc)
	for (auto& entity : enemyVec)
	{
		auto pos_a = entity->cTransform->pos;
		auto coll_rad_a = entity->cCollision->radius;
		
		if (intersects(pos_a, m_player->cTransform->pos, coll_rad_a, m_player->cCollision->radius))
		{
			restartGame();
			break;
		}
		else
		{
			for (auto& bullet : m_entities.getEntities("bullet"))
			{
				auto pos_b = bullet->cTransform->pos;
				auto coll_rad_b = bullet->cCollision->radius;
			
				if (intersects(pos_a, pos_b, coll_rad_a, coll_rad_b))
				{
					if (entity->tag() == "enemy")
					{
						spawnEnemy(true, "bad", entity);				
					}
					
					entity->destroy();
					bullet->destroy();
					m_score += entity->cScore->score;
					break;
				}
			}
			
			if (entity->isActive())
			{
				for (auto& small_good : m_entities.getEntities("small_inf"))
				{
					if (!entity->cInfected)
					{
						auto pos_b = small_good->cTransform->pos;
						auto coll_rad_b = small_good->cCollision->radius;
					
						if (intersects(pos_a, pos_b, coll_rad_a, coll_rad_b))
						{
							if (entity->tag() == "enemy")
							{
								infectTarget(entity);
								small_good->destroy();						
							}
							else
							{
								entity->destroy();
								m_score += entity->cScore->score;
								small_good->destroy();
							}
							
							break;
						}
					}				
					
				}
			}
			
			if (entity->isActive() && entity->tag() == "enemy" && !entity->cInfected)
			{
				for (auto& special_bull : m_entities.getEntities("special_bullet"))
				{
					auto pos_b = special_bull->cTransform->pos;
					auto coll_rad_b = special_bull->cCollision->radius;
				
					if (intersects(pos_a, pos_b, coll_rad_a, coll_rad_b))
					{
						infectTarget(entity);
						special_bull->destroy();
						break;
					}
				}
			}
		}
	}
}

void Game::infectTarget(std::shared_ptr<Entity> target)
{
	target->cInfected = std::make_shared<CInfected>(120);
	target->cShape->circle.setFillColor(sf::Color(255, 255, 255));
	target->cShape->circle.setOutlineColor(sf::Color(57, 255, 20));
	target->cShape->circle.setOutlineThickness(5);
	
	m_score += target->cScore->score; // infecting "big" enemies gives you their points immediately
}

void Game::sGui()
{
	// TODO tidy this up
	ImGui::Begin("Geometry Wars");
	
    if (ImGui::BeginTabBar("MyTabBar"))
    {
        if (ImGui::BeginTabItem("Systems"))
        {
			ImGui::Checkbox("Enemy spawning", &m_enemySpawnOn);
			ImGui::SliderFloat("Spawn interval", &m_enemyConfig.SI, 0.0f, 120.0f);
			
			if (ImGui::Button("Manual spawn"))
			{
				Game::spawnEnemy();
			}
			
			if (ImGui::Button("Pause"))
			{
				m_paused = !m_paused;
			}
			
			ImGui::Checkbox("Movement", &m_movementOn);
			ImGui::Checkbox("Collisions", &m_collisionOn);
			ImGui::Checkbox("Lifespans", &m_lifespanOn);
			ImGui::Checkbox("GUI", &m_guiOn);
			ImGui::Checkbox("Rendering", &m_renderOn);
			
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Entities"))
        {	
        	auto destroyID = 0;
        	
        	if (ImGui::CollapsingHeader("All entities"))
     	    {
    			if (ImGui::BeginTable("table1", 4))
				{
		        	for (auto entity : m_entities.getEntities())
					{
						destroyID++;
						ImGui::TableNextRow();
						
						for (int column = 0; column < 4; column++)
						{
							ImGui::TableSetColumnIndex(column);
							
							switch (column)
							{
								case 0:
									ImGui::PushID(destroyID);
									
									if (ImGui::Button("Destroy"))
									{
										entity->destroy();
									}
									
									ImGui::PopID();
									break;
								case 1:
									ImGui::Text("%d", entity->id());
									break;
								case 2:
									ImGui::Text("%s", entity->tag().c_str());
									break;
								case 3:
									auto pos = entity->cTransform->pos;
									ImGui::Text("(%f, %f)", pos.x, pos.y);
									break;	
							}
						}
					}
					ImGui::EndTable();
				}
       	    }
       	    
       	    if (ImGui::CollapsingHeader("Entities by tag"))
     	    {     	    	
				for (auto [tag, entityVec] : m_entities.getEntityMap())
				{
					if (ImGui::CollapsingHeader(tag.c_str()))
					{
            			if (ImGui::BeginTable("table2", 4))
            			{
							for (auto entity : entityVec)
							{
								destroyID++;
								ImGui::TableNextRow();
								
								for (int column = 0; column < 4; column++)
								{
									ImGui::TableSetColumnIndex(column);
									
									switch (column)
									{
										case 0:
											ImGui::PushID(destroyID);
											if (ImGui::Button("Destroy"))
											{
												entity->destroy();
											}
											ImGui::PopID();
											break;
										case 1:
											ImGui::Text("%d", entity->id());
											break;
										case 2:
											ImGui::Text("%s", tag.c_str());
											break;
										case 3:
											auto pos = entity->cTransform->pos;
											ImGui::Text("(%f, %f)", pos.x, pos.y);
											break;
									}
								}
							}
							ImGui::EndTable();
						}
					}
				}
			}
            ImGui::EndTabItem();
		}
        ImGui::EndTabBar();
	}
	
	ImGui::End();
	ImGui::EndFrame();
}

void Game::sRender()
{
	m_window.clear();

	for (auto& entity : m_entities.getEntities())
	{
		entity->cShape->circle.setPosition(entity->cTransform->pos.x, entity->cTransform->pos.y);
		
		m_window.draw(entity->cShape->circle);
		
		auto tag = entity->tag();
		if (tag != "bullet" && tag != "special_bullet")
		{
			entity->cTransform->angle += 1.0f;
			entity->cShape->circle.setRotation(entity->cTransform->angle);
		}
	}
	
	m_text.setString(std::to_string(m_score));
	m_window.draw(m_text);
	
	ImGui::SFML::Render(m_window);
	
	m_window.display();
}

void Game::sMovement()
{
	// PLAYER //
	auto player_vel = playerVel();
	
	m_player->cTransform->pos += player_vel;
	
	// ALL OTHER ENTITIES //
	for (auto& entity : m_entities.getEntities())
	{
		auto tag = entity->tag();
		
		if (tag != "player")
		{
			if (tag == "enemy" || tag == "small_enemy" || tag == "small_inf")
			{
				entity->cTransform->angle += 1.0f;
				entity->cShape->circle.setRotation(entity->cTransform->angle);
			} 
		
			entity->cTransform->pos.x += entity->cTransform->velocity.x;
			entity->cTransform->pos.y += entity->cTransform->velocity.y;
		}
	}
}

void Game::sUserInput()
{
	sf::Event event;
	
	while (m_window.pollEvent(event))
	{
		ImGui::SFML::ProcessEvent(m_window, event);
		
		if (event.type == sf::Event::Closed)
		{
			m_running = false;
		}
		
		if (event.type == sf::Event::KeyPressed)
		{
			m_player->cInput->keys_pressed++;
			
			//std::cout << "KEYS PRESSED " << m_player->cInput->keys_pressed << std::endl;
			
			switch (event.key.code)
			{
				case sf::Keyboard::W:
					m_player->cInput->up = true;
					break;
				case sf::Keyboard::A:
					m_player->cInput->left = true;
					break;
				case sf::Keyboard::S:
					m_player->cInput->down = true;
					break;
				case sf::Keyboard::D:
					m_player->cInput->right = true;
					break;
				case sf::Keyboard::P:
					m_paused = !m_paused;
					break;
				case sf::Keyboard::Q:
					m_running = false;
					break;
				default: break;
			}
		}
		
		if (event.type == sf::Event::KeyReleased)
		{
			m_player->cInput->keys_pressed--;
			
			//std::cout << "KEYS RELEASED " << m_player->cInput->keys_pressed << std::endl;
			
			switch (event.key.code)
			{
				case sf::Keyboard::W:
					m_player->cInput->up = false;
					break;
				case sf::Keyboard::A:
					m_player->cInput->left = false;
					break;
				case sf::Keyboard::S:
					m_player->cInput->down = false;
					break;
				case sf::Keyboard::D:
					m_player->cInput->right = false;
				default: break;
			}
		}
		
		if (event.type == sf::Event::MouseButtonPressed)
		{
			// ignore mouse events if ImGui is clicked
			if (ImGui::GetIO().WantCaptureMouse)
			{ 
				continue; 
			}
			
			if (event.mouseButton.button == sf::Mouse::Left || event.mouseButton.button == sf::Mouse::Right)
			{
				struct BulletEvent bull_event;
				bull_event.source = m_player;
				bull_event.target =  Vec2(event.mouseButton.x, event.mouseButton.y);
				
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					bull_event.type = "bullet";
				}
				
				if (event.mouseButton.button == sf::Mouse::Right)
				{
					// TODO for special bullets, add a 10 second cooldown
					bull_event.type = "special_bullet";
				}
				
				m_bullets.push_back(bull_event);
			}
		}
	}
}
