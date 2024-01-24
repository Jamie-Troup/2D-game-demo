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
				m_window.create(sf::VideoMode(width, height), "Assignment 2", sf::Style::Fullscreen);
			} else
			{
				m_window.create(sf::VideoMode(width, height), "Assignment 2");
			}
			
			m_window.setFramerateLimit(framerate);
			ImGui::SFML::Init(m_window);
			m_window.setKeyRepeatEnabled(false);
		}
		
		if (keyword == "Font")
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
		
		if (keyword == "Player")
		{
			fin >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
		}
		
		if (keyword == "Enemy")
		{
			fin >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
		}
		
		if (keyword == "Bullet")
		{
			fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
		}
	}
	
	spawnPlayer();
}

void Game::run()
{
	while(m_running)
	{
		m_entities.update();
		
		if (m_player.use_count() == 1)
		{
			m_player.reset();
		}
		
		ImGui::SFML::Update(m_window, m_deltaClock.restart());
		
		if (m_sMovementOn && !m_paused)
		{
			sMovement();		
		}
		
		if (m_sEnemySpawnOn && !m_paused)
		{
			sEnemySpawner();
		}
		
		sUserInput();
		
		if (m_sCollisionOn)
		{
			sCollision();		
		}

		sCountdown();
		
		if (m_sLifespanOn)
		{
			sLifespan();		
		}
		
		if (m_sGuiOn)
		{
			sGui();
		}
		
		if (m_sRenderOn)
		{
			sRender();
		}
		
		std::cout << "Frame " << m_currentFrame << std::endl;
		m_currentFrame++;
	}
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

void Game::spawnEnemy()
{
	auto win_size = m_window.getSize();
	
	auto num_sides = rand() % m_enemyConfig.VMAX + m_enemyConfig.VMIN;
	auto speed = ((float) rand() / RAND_MAX) * m_enemyConfig.SMAX + m_enemyConfig.SMIN;
	auto found_pos = false;
	auto pos_x = rand() % (win_size.x - 2 * m_enemyConfig.SR) + m_enemyConfig.SR;
	auto pos_y = rand() % (win_size.y - 2 * m_enemyConfig.SR) + m_enemyConfig.SR;
	if (m_player)
	{
		auto player_pos = m_player->cTransform->pos;
		while (!found_pos)
		{
			pos_x = rand() % (win_size.x - 2 * m_enemyConfig.SR) + m_enemyConfig.SR;
			pos_y = rand() % (win_size.y - 2 * m_enemyConfig.SR) + m_enemyConfig.SR;
			auto x_good = (pos_x > (player_pos.x + speed + m_enemyConfig.CR)) || (pos_x < (player_pos.x - speed - m_enemyConfig.CR));
			auto y_good = (pos_y > (player_pos.y + speed + m_enemyConfig.CR)) || (pos_y < (player_pos.y - speed - m_enemyConfig.CR));
			
			if (x_good && y_good)
			{
				found_pos = true;
			}
		}
	} 
	auto angle = rand() % 360;
	auto entity = m_entities.addEntity("enemy");
	
	entity->cTransform = std::make_shared<CTransform>(Vec2(pos_x, pos_y), Vec2(speed*cos(angle), speed*sin(angle)), 0);
	entity->cShape = std::make_shared<CShape>(m_enemyConfig.SR, num_sides, sf::Color(rand()%255, rand()%255, rand()%255), sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), m_enemyConfig.OT);
	entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);
	entity->cScore = std::make_shared<CScore>(num_sides * 100);
}

void Game::sEnemySpawner()
{
	if ((m_currentFrame - m_lastEnemySpawnTime) > m_enemyConfig.SI)
	{
		Game::spawnEnemy();
		m_lastEnemySpawnTime = m_currentFrame;
	}
}

void Game::spawnSmallEnemy(std::shared_ptr<Entity>& entity, std::string type = "bad")
{
	auto num_sides = entity->cShape->circle.getPointCount();
	auto radius = entity->cShape->circle.getRadius() / 4;
	auto angle = (2 * M_PI) / num_sides;
	auto speed = 2.5;
	auto pos = entity->cTransform->pos;
	
	std::shared_ptr<Entity> enemy;
	sf::Color color, outline_col;
	int outline_thick;
		
	for (int i = 0; i < num_sides; i++)
	{
		auto velocity = Vec2(speed*cos(i*angle), speed*sin(i*angle));
		if (type == "bad")
		{
			enemy = m_entities.addEntity("small_enemy");
			color = entity->cShape->circle.getFillColor();
			outline_col = sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB);
			outline_thick = m_enemyConfig.OT;
		} else
		{
			enemy = m_entities.addEntity("small_good_enemy");
			color = sf::Color(255, 255, 255);
			outline_col = sf::Color(57, 255, 20);
			outline_thick = m_enemyConfig.OT + 5;
		}
		
		auto coll_rad = radius + outline_thick;
		enemy->cCollision = std::make_shared<CCollision>(coll_rad);
		enemy->cTransform = std::make_shared<CTransform>(pos, velocity, 0);
		enemy->cShape = std::make_shared<CShape>(radius, num_sides, color, outline_col, outline_thick);
		enemy->cScore = std::make_shared<CScore>(num_sides * 2 * 100);
		enemy->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
	}
}

void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2& target)
{
	auto dx = target.x - entity->cTransform->pos.x;
	auto dy = target.y - entity->cTransform->pos.y;
	
	auto angle = atan2f(dy, dx);
	
	float bullet_speed = m_bulletConfig.S;
	auto bullet_vx = bullet_speed * cos(angle);
	auto bullet_vy = bullet_speed * sin(angle);
	auto bullet_vel = Vec2(bullet_vx, bullet_vy);
	
	auto e = m_entities.addEntity("bullet");
	
	e->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, bullet_vel, 0);
	e->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB), sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), m_bulletConfig.OT);
	e->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);
	e->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity, const Vec2& target)
{
	std::cout << "Spawning special bullet" << std::endl;
	auto dx = target.x - entity->cTransform->pos.x;
	auto dy = target.y - entity->cTransform->pos.y;
	
	auto angle = atan2f(dy, dx);
	
	float bullet_speed = 7.5;
	auto bullet_vx = bullet_speed * cos(angle);
	auto bullet_vy = bullet_speed * sin(angle);
	auto bullet_vel = Vec2(bullet_vx, bullet_vy);
	
	auto e = m_entities.addEntity("special_bullet");
	
	e->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, bullet_vel, 0);
	e->cShape = std::make_shared<CShape>(15, m_bulletConfig.V, sf::Color(255, 215, 0), sf::Color(255, 215, 0), 5);
	e->cLifespan = std::make_shared<CLifespan>(45);
	e->cCollision = std::make_shared<CCollision>(20);
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
				e->cLifespan->remaining -= 1;
			} else
			{
				e->destroy();
			}
		}
	}
}

void Game::sCountdown()
{
	for (auto e : m_entities.getEntities())
	{
		if (e->cCountdown)
		{
			if (e->cCountdown->remaining > 0)
			{
				e->cCountdown->remaining -= 1;
			} else
			{
				e->destroy();
				Game::spawnSmallEnemy(e, "good");
			}
		}
	}
}

void Game::sCollision()
{
	// BULLET HITS //
	for (auto& bullet : m_entities.getEntities("bullet"))
	{
		float radius = bullet->cCollision->radius;
		auto pos = bullet->cTransform->pos;
		
		for (auto enemy : m_entities.getEntities("enemy"))
		{
			if (pos.dist(enemy->cTransform->pos) < (radius + enemy->cCollision->radius))
			{
				enemy->destroy();
				m_score += enemy->cScore->score;
				spawnSmallEnemy(enemy);
				bullet->destroy();
				break;
			}
		}
		
		for (auto& enemy : m_entities.getEntities("small_enemy"))
		{
			if (pos.dist(enemy->cTransform->pos) < (radius + enemy->cCollision->radius))
			{
				enemy->destroy();
				m_score += enemy->cScore->score;
				bullet->destroy();
				break;
			}
		}
	}
	
	// SPECIAL BULLET HITS
	for (auto& bullet : m_entities.getEntities("special_bullet"))
	{
		float radius = bullet->cCollision->radius;
		auto pos = bullet->cTransform->pos;
		
		for (auto& enemy : m_entities.getEntities("enemy"))
		{
			if (pos.dist(enemy->cTransform->pos) < (radius + enemy->cCollision->radius))
			{
				enemy->cCountdown = std::make_shared<CCountdown>(120);
				enemy->cShape->circle.setFillColor(sf::Color(255, 255, 255));
				enemy->cShape->circle.setOutlineColor(sf::Color(57, 255, 20));
				enemy->cShape->circle.setOutlineThickness(5);
				bullet->destroy();
				m_score += enemy->cScore->score;
				break;
			}
		}
	}
	
	// ENEMY/SMALL ENEMY HITTING PLAYER CHECKS //
	if (m_player)
	{
		auto pos = m_player->cTransform->pos;
		for (auto& enemy : m_entities.getEntities("enemy"))
		{
			auto enemy_pos = enemy->cTransform->pos;
			auto enemy_rad = enemy->cCollision->radius;
			
			if (pos.dist(enemy_pos) < (enemy_rad + m_player->cCollision->radius))
			{
				enemy->destroy();
				m_player->destroy();
				m_score = 0;
				Game::spawnPlayer();
			}
		}
		
		for (auto& enemy : m_entities.getEntities("small_enemy"))
		{
			auto enemy_pos = enemy->cTransform->pos;
			auto enemy_rad = enemy->cCollision->radius;
			
			if (pos.dist(enemy_pos) < (enemy_rad + m_player->cCollision->radius))
			{
				enemy->destroy();
				m_player->destroy();
				m_score = 0;
				Game::spawnPlayer();
			}
		}
	}
	
	// ENEMIES/SMALL ENEMIES/PLAYERS HITTING WALL CHECKS
	for (auto& entity : m_entities.getEntities())
	{
		auto tag = entity->tag();
		auto pos = entity->cTransform->pos;
		auto radius = entity->cCollision->radius;
		auto size = m_window.getSize();
		if (tag == "enemy" || tag == "small_enemy" || tag == "small_good_enemy")
		{
			if (pos.x <= radius)
			{
				if (entity->cTransform->velocity.x < 0)
				{
					entity->cTransform->velocity.x *= -1;
				}
			} else if (pos.x >= (size.x - radius))
			{
				if (entity->cTransform->velocity.x > 0)
				{
					entity->cTransform->velocity.x *= -1;
				}
			}
			
			if (pos.y <= radius)
			{
				if (entity->cTransform->velocity.y < 0)
				{
					entity->cTransform->velocity.y *= -1;
				}
			} else if (pos.y >= (size.y - radius))
			{
				if (entity->cTransform->velocity.y > 0)
				{
					entity->cTransform->velocity.y *= -1;
				}
			}
		} else if (tag == "player")
		{
			if (pos.x <= radius)
			{
				if (entity->cInput->left)
				{
					entity->cTransform->pos.x += entity->cTransform->velocity.x;
				}
			} else if (pos.x >= (size.x - radius))
			{
				if (entity->cInput->right)
				{
					entity->cTransform->pos.x -= entity->cTransform->velocity.x;
				}
			}
			
			if (pos.y <= radius)
			{
				if (entity->cInput->up)
				{
					entity->cTransform->pos.y += entity->cTransform->velocity.y;
				}
			} else if (pos.y >= (size.y - radius))
			{
				if (entity->cInput->down)
				{
					entity->cTransform->pos.y -= entity->cTransform->velocity.y;
				}
			}
		}
	}
	
	// Small good enemies hitting enemies
	for (auto& good_enemy : m_entities.getEntities("small_good_enemy"))
	{
		auto good_enemy_pos = good_enemy->cTransform->pos;
		auto good_enemy_coll = good_enemy->cCollision->radius;
		
		for (auto& enemy : m_entities.getEntities())
		{
			if (enemy->tag() == "enemy" || enemy->tag() == "small_enemy")
			{
				if (good_enemy_pos.dist(enemy->cTransform->pos) < (good_enemy_coll + enemy->cCollision->radius))
				{
					enemy->destroy();
					m_score += enemy->cScore->score;
					good_enemy->destroy();
					break;
				}
			}
		}
	}
}

void Game::sGui()
{
	ImGui::Begin("Geometry Wars");
	
    if (ImGui::BeginTabBar("MyTabBar"))
    {
        if (ImGui::BeginTabItem("Systems"))
        {
			ImGui::Checkbox("Enemy spawning", &m_sEnemySpawnOn);
			ImGui::SliderFloat("Spawn interval", &m_enemyConfig.SI, 0.0f, 120.0f);
			if (ImGui::Button("Manual spawn"))
			{
				Game::spawnEnemy();
			}
			if (ImGui::Button("Pause"))
			{
				m_paused = !m_paused;
			}
			ImGui::Checkbox("Movement", &m_sMovementOn);
			ImGui::Checkbox("Collisions", &m_sCollisionOn);
			ImGui::Checkbox("Lifespans", &m_sLifespanOn);
			ImGui::Checkbox("GUI", &m_sGuiOn);
			ImGui::Checkbox("Rendering", &m_sRenderOn);
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
		
		if (tag != "bullet")
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
	if (m_player)
	{
		float player_angle;
		if (m_player->cInput->keys_pressed == 2)
		{
			std::cout << "2 keys pressed" << std::endl;
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
			} else
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
				m_player->cTransform->pos.y += m_player->cTransform->velocity.y*sin(player_angle) * -1;
				m_player->cTransform->pos.x += m_player->cTransform->velocity.x*cos(player_angle);
			}
		} else
		{
			if (m_player->cInput->up)
			{
				std::cout << "UP" << std::endl;
				m_player->cTransform->pos.y -= m_player->cTransform->velocity.y;
			}
			if (m_player->cInput->left)
			{
				std::cout << "LEFT" << std::endl;
				m_player->cTransform->pos.x -= m_player->cTransform->velocity.x;
			}
			if (m_player->cInput->right)
			{
				std::cout << "RIGHT" << std::endl;
				m_player->cTransform->pos.x += m_player->cTransform->velocity.x;
			}
			if (m_player->cInput->down)
			{
				std::cout << "DOWN" << std::endl;
				m_player->cTransform->pos.y += m_player->cTransform->velocity.y;
			}
		}
	}
	
	// ALL OTHER ENTITIES //
	for (auto& entity : m_entities.getEntities())
	{
		auto tag = entity->tag();
		if (tag == "enemy" || tag == "small_enemy" || tag == "small_good_enemy")
		{
			if (entity->cTransform)
			{
				entity->cTransform->angle += 1.0f;
				entity->cShape->circle.setRotation(entity->cTransform->angle);
				
				auto radius = entity->cCollision->radius;
				auto pos = entity->cTransform->pos;
				auto size = m_window.getSize();
			
				entity->cTransform->pos.x += entity->cTransform->velocity.x;
				entity->cTransform->pos.y += entity->cTransform->velocity.y;
			}
		} else if (tag == "bullet" || tag == "special_bullet")
		{
			entity->cTransform->pos.x += entity->cTransform->velocity.x;
			entity->cTransform->pos.y += entity->cTransform->velocity.y;
		}
	}
}

void Game::sUserInput()
{
	// Make this so that it loops over all enemies with a cInput and applies same logic. That way, you could add the cInput component dynamically and assume control. Not essential.
	sf::Event event;
	while (m_window.pollEvent(event))
	{
		ImGui::SFML::ProcessEvent(m_window, event);
		
		if (event.type == sf::Event::Closed)
		{
			m_running = false;
		}
		
		if (m_player)
		{
			if (event.type == sf::Event::KeyPressed)
			{
				m_player->cInput->keys_pressed++;
				std::cout << "KEYS PRESSED " << m_player->cInput->keys_pressed << std::endl;
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
					default: break;
				}
			}
			
			if (event.type == sf::Event::KeyReleased)
			{
				m_player->cInput->keys_pressed--;
				std::cout << "(released) KEYS PRESSED " << m_player->cInput->keys_pressed << std::endl;
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
				if (ImGui::GetIO().WantCaptureMouse) { continue; }
				
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					Game::spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
				}
				
				if (event.mouseButton.button == sf::Mouse::Right)
				{
					Game::spawnSpecialWeapon(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
				}
			}
		}
	}
}
