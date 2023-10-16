#include "Engine.hpp"


namespace rps
{
	Engine::Engine()
	{
		config = WindowConfig{};

		settings.antialiasingLevel = config.antialiasingLevel;
		window = new sf::RenderWindow(sf::VideoMode(config.width, config.height), config.name, sf::Style::Default, settings);
		winSize = window->getSize();
		isFullscreen = false;

		loadSettings();
		loadPresets();
		setFullscreen();
		for (uint8_t type = ROCK; type < types; ++type)
		{
			objects.push_back(std::vector<sf::RectangleShape*>());
			objects[type].reserve(count * types);
			for (size_t i = 0ull; i < count; ++i)
			{
				sf::RectangleShape* object = createObject(getRandomPos(), size, type);
				objects[type].push_back(object);
			}
		}

		playIntro();
	}

	Engine::~Engine()
	{
		delete window;

		for (uint8_t type = ROCK; type < types; ++type)
		{
			for (auto object : objects[type])
			{
				delete object;
			}
		}

		for (auto texture : textures)
		{
			delete texture;
		}
		delete errorTexture;

		for (auto typeSoundBuffer : soundBuffers)
		{
			for (auto soundBuffer : typeSoundBuffer)
			{
				delete soundBuffer;
			}
		}
		delete introSound;

		clearSoundPoll();
		clearSoundHeap();
	}

	void Engine::setFullscreen()
	{
		sf::Vector2u maxSize(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		sf::Vector2u size(maxSize);
		sf::Vector2i winPos(0, 0);
		uint16_t style = sf::Style::None;

		if (!isFullscreen)
		{
			size = sf::Vector2u(std::min(config.width, maxSize.x), std::min(config.height, maxSize.y));
			winPos = sf::Vector2i((maxSize.x - size.x) / 2, (maxSize.y - size.y) / 2);
			style = sf::Style::Default;
		}

		window->close();
		delete window;

		window = new sf::RenderWindow(sf::VideoMode(size.x, size.y), config.name, style, settings);
		window->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

		winSize = window->getSize();

		setIntro();
		setControlsTab();
		setDebugString();
	}

	void Engine::playIntro()
	{
		if (introSound != nullptr)
			introSound->play();

		window->clear();

		std::vector<int64_t> delays{ 500, 433, 700 };
		for (uint8_t type = ROCK; type < types; ++type)
		{
			if (type == SCISSORS)
			{
				introPreview.setTexture(textures[type]);
				window->draw(introPreview);
				window->display();
			}
			introPreview.setTexture(textures[type]);
			window->draw(introPreview);
			window->display();
			window->clear();
			sleep(delays[type]);
		}
	}

// --------------------------------Load Presets--------------------------------

	void Engine::loadSettings()
	{
		timeCounter = 0.0l;
		FPSLimit = gameSettings.FPSLimit;
		timer = std::chrono::high_resolution_clock();

		isF3Menu = false;
		isControlsTab = true;

		size = gameSettings.size;
		count = gameSettings.count;
		speed = gameSettings.speed;
		types = gameSettings.types;
		volume = gameSettings.volume;
	}

	void Engine::loadPresets()
	{
		loadIcon();

		loadFont();

		setFPSCounter();
		setF3Menu();
		setControlsTab();
		setDebugString();

		loadTextures();

		setIntro();

		loadSounds();
		soundPoll.reserve(140ull);
	}

	void Engine::loadFont()
	{
		fontName = "minecraft.ttf";
		std::cout << "Loading font..." << std::endl;
		if (!font.loadFromFile("./Font/" + fontName))
		{
			std::cout << "Failed to load " + fontName << std::endl;
			return;
		}
		std::cout << fontName << " was loaded successfully" << std::endl << std::endl;
	}

	void Engine::loadIcon()
	{
		std::cout << "Loading icon..." << std::endl;
		if (!icon.loadFromFile("./Icon/icon.png"))
		{
			std::cout << "Failed to load icon.png as Image" << std::endl;
			return;
		}
		std::cout << "icon.png was loaded successfully" << std::endl << std::endl;
		window->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	}

	void Engine::loadTextures()
	{
		createErrorTexture();
		textureNames = { "raw_iron.png", "paper.png", "shears.png" };
		std::cout << "Loading textures..." << std::endl;
		for (auto textureName : textureNames)
		{
			sf::Texture* texture = new sf::Texture();
			if (!texture->loadFromFile("./Textures/" + textureName))
			{
				std::cout << "Failed to load " << textureName << std::endl;
				textures.push_back(errorTexture);
				continue;
			}
			std::cout << textureName << " was loaded successfully" << std::endl;
			textures.push_back(texture);
		}
		std::cout << "Done." << std::endl << std::endl;
	}

	void Engine::createErrorTexture()
	{
		sf::Image errorImage;
		errorImage.create(2, 2);
		errorImage.setPixel(0, 0, sf::Color::Black);
		errorImage.setPixel(1, 0, sf::Color::Magenta);
		errorImage.setPixel(0, 1, sf::Color::Magenta);
		errorImage.setPixel(1, 1, sf::Color::Black);
		errorTexture = new sf::Texture();
		errorTexture->loadFromImage(errorImage);
	}

	void Engine::loadSounds()
	{
		soundNames = {
			{ "Stone_dig1.ogg", "Stone_dig2.ogg", "Stone_dig3.ogg", "Stone_dig4.ogg" },
			{ "Grass_hit1.ogg", "Grass_hit2.ogg", "Grass_hit3.ogg", "Grass_hit4.ogg", "Grass_hit5.ogg", "Grass_hit6.ogg"},
			{ "Shear.ogg" }
		};
		
		std::cout << "Loading sounds..." << std::endl;
		for (uint8_t type = ROCK; type < types; ++type)
		{
			std::cout << static_cast<int>(type) << ':' << std::endl;
			soundBuffers.push_back(std::vector<sf::SoundBuffer*>());
			for (auto soundName : soundNames[type])
			{
				sf::SoundBuffer* soundBuffer = new sf::SoundBuffer;
				if (!soundBuffer->loadFromFile("./Sounds/" + soundName))
				{
					std::cout << "\tFailed to load " << soundName << std::endl;
					continue;
				}
				std::cout << '\t' << soundName << " was loaded successfully" << std::endl;
				soundBuffers[type].push_back(soundBuffer);
			}
		}

		std::cout << "intro:" << std::endl;
		if (!introBuffer.loadFromFile("./Sounds/intro.ogg"))
		{
			std::cout << "\tFailed to load intro.ogg" << std::endl;
			return;
		}
		std::cout << "\tintro.ogg was loaded successfully" << std::endl;
		introSound = new sf::Sound;
		introSound->setBuffer(introBuffer);
		introSound->setVolume(100.0f);
		std::cout << "Done." << std::endl << std::endl;
	}

	void Engine::setIntro()
	{
		size_t minSize = std::min(winSize.x, winSize.y);
		size_t maxSize = std::max(winSize.x, winSize.y);
		sf::Vector2f previewSize(minSize, minSize);
		sf::Vector2f originPos(minSize / 2ull, minSize / 2ull);
		sf::Vector2f previewPos(winSize.x / 2ull, winSize.y / 2ull);
		introPreview.setSize(previewSize);
		introPreview.setOrigin(originPos);
		introPreview.setPosition(previewPos);
	}

	void Engine::setFPSCounter()
	{
		FPSCounter.setFont(font);
		FPSCounter.setString(std::to_string(FPSLimit));
		FPSCounter.setPosition(sf::Vector2f(8.0f, 8.0f));
		FPSCounter.setCharacterSize(16u);
		FPSCounter.setFillColor(sf::Color::Green);
		FPSCounter.setOutlineThickness(1.0f);
		FPSCounter.setOutlineColor(sf::Color::Black);
	}

	float Engine::getHeightOfBottomPanel(std::string& panel)
	{
		size_t lineCount = std::count(panel.begin(), panel.end(), '\n') + 1ull;
		float panelHeight = lineCount * CHAR_SIZE * LINE_SPACE;
		return static_cast<int>(winSize.y) - panelHeight - 8.0f;
	}

	void Engine::setF3Menu()
	{
		std::vector<std::string> panelContent = {
			"FPSLimit:\n"
			"deltaTime:\n"
			"\n"
			"Rocks:\n"
			"Papers:\n"
			"Scissors:\n"
			"Total:\n"
			"\n"
			"Speed:\n"
			"Size:\n"
			"Volume:\n"
			"Count:",

			"(There must be stats of first panel)"
		};

		std::vector<sf::Vector2f> panelPoses = {
			sf::Vector2f(8.0f, 32.0f),
			sf::Vector2f(128.0f, 32.0f)
		};

		controlsTab.clear();
		for (size_t i = 0ull; i < panelPoses.size(); ++i)
		{
			F3Menu.push_back(sf::Text());
			F3Menu[i].setFont(font);
			F3Menu[i].setString(panelContent[i]);
			F3Menu[i].setPosition(panelPoses[i]);
			F3Menu[i].setCharacterSize(CHAR_SIZE);
			F3Menu[i].setFillColor(sf::Color(234u, 234u, 234u));
			F3Menu[i].setOutlineThickness(1.0f);
			F3Menu[i].setOutlineColor(sf::Color::Black);
			F3Menu[i].setLineSpacing(LINE_SPACE);
		}
	}

	void Engine::setControlsTab()
	{
		std::vector<std::string> panelContent = {
			",--------Controls--------,\n"
			"|                                                                      |\n"
			"|                                                                      |\n"
			"|                                                                      |\n"
			"|                                                                      |\n"
			"|                                                                      |\n"
			"|                                                                      |\n"
			"|                                                                      |\n"
			"|                                                                      |\n"
			"|                                                                      |\n"
			"|                                                                      |\n"
			"|                                                                      |\n"
			"|                                                                      |\n"
			"|                                                                      |\n"
			"\\______________________/",

			"Esc\n"
			"F3\n"
			"F11\n"
			"R\n"
			"1/4\n"
			"2/5\n"
			"3/6\n"
			"A/Q\n"
			"S/W\n"
			"D/E\n"
			"Z/X\n"
			"C\n"
			"",

			"->    Exit\n"
			"->    F3 Menu\n"
			"->    Fullscreen\n"
			"->    Restart\n"
			"->    -/+ Rocks\n"
			"->    -/+ Papers\n"
			"->    -/+ Scissors\n"
			"->    -/+ Speed\n"
			"->    -/+ Size\n"
			"->    -/+ Volume\n"
			"->    -/+ Count\n"
			"->    Close Tab\n"
			""
		};

		std::vector<sf::Vector2f> panelPoses = {
			sf::Vector2f(8.0f, getHeightOfBottomPanel(panelContent[0]) - 20.0f),
			sf::Vector2f(48.0f, getHeightOfBottomPanel(panelContent[1]) - 20.0f),
			sf::Vector2f(100.0f, getHeightOfBottomPanel(panelContent[2]) - 20.0f)
		};

		for (size_t i = 0ull; i < panelPoses.size(); ++i)
		{
			controlsTab.push_back(sf::Text());
			controlsTab[i].setFont(font);
			controlsTab[i].setString(panelContent[i]);
			controlsTab[i].setPosition(panelPoses[i]);
			controlsTab[i].setCharacterSize(CHAR_SIZE);
			controlsTab[i].setFillColor(sf::Color::White);
			controlsTab[i].setOutlineThickness(1.0f);
			controlsTab[i].setOutlineColor(sf::Color::Black);
			controlsTab[i].setLineSpacing(LINE_SPACE);
		}
	}

	void Engine::setDebugString()
	{
		debugString.setFont(font);
		debugString.setPosition(sf::Vector2f(8.0f, winSize.y - 24.0f));
		debugString.setCharacterSize(CHAR_SIZE);
		debugString.setFillColor(sf::Color::Yellow);
	}

// --------------------------------Commands--------------------------------

	void Engine::addObject(uint8_t type)
	{
		sf::RectangleShape* object = createObject(getRandomPos(), size, type);
		objects[type].push_back(object);
	}

	void Engine::deleteObject(uint8_t type)
	{
		if (objects[type].size() == 0ull)
			return;
		size_t randomIndex = rand() % objects[type].size();
		delete objects[type][randomIndex];
		objects[type].erase(objects[type].begin() + randomIndex);
	}

	void Engine::changeSpeed(float change)
	{
		speed = std::fmaxf(-512.0f, std::fminf(speed + change, 512.0f));
		gameSettings.speed = speed;
	}

	void Engine::changeCount(int64_t change)
	{
		count = std::max(1ll, std::min(static_cast<int64_t>(count) + change, 512ll));
		gameSettings.count = count;
	}

	void Engine::changeSize(float change)
	{
		size = std::fmaxf(4.0f, std::fminf(size + change, 256.0f));
		for (uint8_t type = ROCK; type < types; ++type)
		{
			for (auto object : objects[type])
			{
				object->setSize(sf::Vector2f(size, size));
				object->setOrigin(sf::Vector2f(size / 2.0f, size / 2.0f));
			}
		}
		gameSettings.size = size;
	}

	void Engine::changeVolume(float change)
	{
		volume = std::fmaxf(0.0f, std::fminf(volume + change, 100.0f));
		for (auto sound : soundPoll)
		{
			sound->setVolume(volume);
		}
		gameSettings.volume = volume;
	}

// --------------------------------Helpful Functions--------------------------------

	inline sf::Vector2f Engine::getRandomPos()
	{
		return sf::Vector2f(rand() % winSize.x, rand() % winSize.y);
	}

	sf::RectangleShape* Engine::createObject(sf::Vector2f pos, float size, uint8_t type)
	{
		sf::RectangleShape* object = new sf::RectangleShape;

		object->setSize(sf::Vector2f(size, size));
		object->setOrigin(sf::Vector2f(size / 2.0f, size / 2.0f));
		object->setPosition(pos);
		object->setTexture(textures[type]);

		return object;
	}

	void Engine::playSound(sf::SoundBuffer* soundBuffer)
	{
		sf::Sound* sound = new sf::Sound;
		sound->setBuffer(*soundBuffer);
		if (soundPoll.size() < MAX_SIZE_SOUND_POLL)
		{
			sound->setVolume(volume);
			sound->play();
			soundPoll.push_back(sound);
		}
		else
		{
			soundHeap.push_back(sound);
		}
	}

	void Engine::setF3MenuStats()
	{
		F3Menu[1].setString(
			std::to_string(FPSLimit)  + '\n' +
			std::to_string(deltaTime) + '\n' +
			'\n' +
			std::to_string(objects[0].size()) + '\n' +
			std::to_string(objects[1].size()) + '\n' +
			std::to_string(objects[2].size()) + '\n' +
			std::to_string(objects[0].size() + objects[1].size() + objects[2].size()) + '\n' +
			'\n' +
			std::to_string(static_cast<int64_t>(speed))  + '\n' +
			std::to_string(static_cast<int64_t>(size))   + '\n' +
			std::to_string(static_cast<int64_t>(volume)) + '\n' +
			std::to_string(count)
		);
	}

	void Engine::cleanSoundPoll()
	{
		for (size_t i = 0ull; i < soundPoll.size(); ++i)
		{
			if (soundPoll[i]->getStatus() == sf::Sound::Stopped)
			{
				delete soundPoll[i];
				soundPoll.erase(soundPoll.begin() + i);
			}
		}
		for (size_t i = 0ull; i < std::min(soundHeap.size(), MAX_SIZE_SOUND_POLL - soundPoll.size()); ++i)
		{
			sf::Sound* temp = soundHeap.front(); 
			temp->setVolume(volume);
			temp->play();
			soundPoll.push_back(temp);
			soundHeap.erase(soundHeap.begin());
		}
	}

	void Engine::clearSoundPoll()
	{
		for (auto sound : soundPoll)
		{
			delete sound;
		}
		soundPoll.clear();
	}

	void Engine::clearSoundHeap()
	{
		for (auto sound : soundHeap)
		{
			delete sound;
		}
		soundHeap.clear();
	}

	inline void Engine::clearEventPoll()
	{
		while (window->pollEvent(event)) {};
	}

// --------------------------------Useful Functions--------------------------------

	sf::Vector2f getNormalized(const sf::Vector2f& v)
	{
		float mag = std::sqrtf(v.x * v.x + v.y * v.y);
		return sf::Vector2f(v.x / mag, v.y / mag);
	}

	inline sf::Vector2f getScaled(const sf::Vector2f& v, float scale)
	{
		return sf::Vector2f(v.x * scale, v.y * scale);
	}

	inline sf::Vector2f getDistance(const sf::Vector2f& a, const sf::Vector2f& b)
	{
		return sf::Vector2f(b.x - a.x, b.y - a.y);
	}

	inline float getMagnitude(const sf::Vector2f& v)
	{
		return std::sqrtf(v.x * v.x + v.y * v.y);
	}

	inline float getSqrMagnitude(const sf::Vector2f& v)
	{
		return v.x * v.x + v.y * v.y;
	}

	size_t Engine::getNearestObject(const sf::Vector2f& myPos, std::vector<sf::RectangleShape*>& victims)
	{
		size_t nearestVictim = 0ull;
		float nearestDistSqrMag = getSqrMagnitude(getDistance(myPos, victims[0]->getPosition()));
		for (size_t i = 0ull; i < victims.size(); ++i)
		{
			float distSqrMag = getSqrMagnitude(getDistance(myPos, victims[i]->getPosition()));
			if (distSqrMag < nearestDistSqrMag)
			{
				nearestVictim = i;
				nearestDistSqrMag = distSqrMag;
			}
		}
		return nearestVictim;
	}

	inline void Engine::moveTo(sf::RectangleShape* object, const sf::Vector2f& pos, float speed)
	{
		object->move(getScaled(getNormalized(getDistance(object->getPosition(), pos)), speed * deltaTime));
	}

	inline void Engine::sleep(int64_t milliseconds)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	}

	void Engine::restart()
	{
		for (uint8_t type = ROCK; type < types; ++type)
		{
			for (auto object : objects[type])
			{
				delete object;
			}
			objects[type].clear();

			for (size_t i = 0ull; i < count; ++i)
			{
				sf::RectangleShape* object = createObject(getRandomPos(), size, type);
				objects[type].push_back(object);
			}
		}
		clearEventPoll();
		clearSoundPoll();
		clearSoundHeap();
		playIntro();
	}

	void Engine::debugLog(size_t n, ...)
	{
		size_t* pointer = &n;

		std::string result = "Debug Log: " + std::to_string(*(pointer + 1ull));
		for (size_t i = 1ull; i < n; ++i)
		{
			result += ' ' + std::to_string(*(pointer + i + 1ull));
		}
		debugString.setString(result);
	}

// --------------------------------Main Loop--------------------------------

	void Engine::run()
	{
		while (window->isOpen())
		{
			startFrameTime = timer.now();

			while (window->pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window->close();
				else if (event.type == sf::Event::Resized)
				{
					isFullscreen = !isFullscreen;
					setFullscreen();
				}
				else if (event.type == sf::Event::KeyPressed)
				{
					switch (event.key.code)
					{
					case sf::Keyboard::Num1:
						addObject(ROCK); break;
					case sf::Keyboard::Num2:
						addObject(PAPER); break;
					case sf::Keyboard::Num3:
						addObject(SCISSORS); break;
					case sf::Keyboard::Num4:
						deleteObject(ROCK); break;
					case sf::Keyboard::Num5:
						deleteObject(PAPER); break;
					case sf::Keyboard::Num6:
						deleteObject(SCISSORS); break;
					case sf::Keyboard::Q:
						changeSpeed(4.0f); break;
					case sf::Keyboard::A:
						changeSpeed(-4.0f); break;
					case sf::Keyboard::W:
						changeSize(4.0f); break;
					case sf::Keyboard::S:
						changeSize(-4.0f); break;
					case sf::Keyboard::E:
						changeVolume(2.0f); break;
					case sf::Keyboard::D:
						changeVolume(-2.0f); break;
					case sf::Keyboard::X:
						changeCount(1ll); break;
					case sf::Keyboard::Z:
						changeCount(-1ll); break;
					}
				}
				else if (event.type == sf::Event::KeyReleased)
				{
					switch (event.key.code)
					{
					case sf::Keyboard::Escape:
						window->close(); break;
					case sf::Keyboard::F3:
						isF3Menu = !isF3Menu; break;
					case sf::Keyboard::R:
						restart(); break;
					case sf::Keyboard::C:
						isControlsTab = !isControlsTab; break;
					case sf::Keyboard::F11:
						isFullscreen = !isFullscreen;
						setFullscreen();
						break;
					}
				}
			}

			for (uint8_t type = 0u; type < types; ++type)
			{
				uint8_t victimType = (type + types - 1u) % types;
				uint8_t hunterType = (type + 1u) % types;

				for (size_t i = 0ull; i < objects[type].size(); ++i)
				{
					sf::RectangleShape* object = objects[type][i];
					if (objects[victimType].size() != 0ull)
					{
						size_t nearestVictimIndex = getNearestObject(object->getPosition(), objects[victimType]);
						sf::RectangleShape* nearestVictim = objects[victimType][nearestVictimIndex];
						moveTo(object, nearestVictim->getPosition(), speed);

						if (object->getGlobalBounds().contains(nearestVictim->getPosition()))
						{
							objects[victimType].erase(objects[victimType].cbegin() + nearestVictimIndex);
							objects[type].push_back(nearestVictim);
							//std::cout << i << '\t' << static_cast<int>(victimType) << '\t' << objects[victimType].size() << '\t' << static_cast<int>(type) << '\t' << objects[type].size() << '\t' << static_cast<int>(hunterType) << '\t' << objects[hunterType].size() << std::endl;
							objects[type].back()->setTexture(textures[type]);

							if (soundBuffers[type].size() != 0ull)
								playSound(soundBuffers[type][rand() % soundBuffers[type].size()]);

						}
					}
					if (objects[hunterType].size() != 0ull)
					{
						size_t nearestHunterIndex = getNearestObject(object->getPosition(), objects[hunterType]);
						sf::RectangleShape* nearestHunter = objects[hunterType][nearestHunterIndex];
						moveTo(object, nearestHunter->getPosition(), -speed * 0.5f);
					}
					sf::Vector2f myPos = object->getPosition();
					if (winSize.x - size < myPos.x || myPos.x < size || winSize.y - size < myPos.y || myPos.y < size)
					{
						float x = std::fmaxf(std::fminf(myPos.x, winSize.x - size), size);
						float y = std::fmaxf(std::fminf(myPos.y, winSize.y - size), size);
						object->setPosition(sf::Vector2f(x, y));
					}
				}
			}

			window->clear();

			for (uint8_t type = 0u; type < types; ++type)
			{
				for (auto object : objects[type])
				{
					window->draw(*object);
				}
			}

			if (isF3Menu)
			{
				setF3MenuStats();

				for (auto panel : F3Menu)
				{
					window->draw(panel);
				}
			}

			window->draw(debugString);

			if (isControlsTab)
			{
				for (auto panel : controlsTab)
				{
					window->draw(panel);
				}
			}

			if (timeCounter > 1.0l)
			{
				FPSCounter.setString(std::to_string(static_cast<int>(std::roundl(1.0 / deltaTime))));
				timeCounter = std::fmodl(timeCounter, 1.0l);
			}
			window->draw(FPSCounter);

			window->display();

			cleanSoundPoll();

			deltaTime = std::chrono::duration_cast<std::chrono::duration<long double>>(timer.now() - startFrameTime).count();
			if (1.0 / FPSLimit >= deltaTime)
			{
				deltaTime = 1.0 / FPSLimit;
				sleep(static_cast<int64_t>(deltaTime * 1000.0));
			}
			timeCounter += deltaTime;
		}
	}
}